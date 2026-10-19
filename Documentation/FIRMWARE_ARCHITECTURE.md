# Архитектура прошивки / Firmware Architecture

**Целевой микроконтроллер:** Arduino Pro Micro, ATmega32U4  
**Симулятор:** Microsoft Flight Simulator  
**Управление симулятором:** USB HID  
**Настройка и диагностика:** USB Serial  
**Текущая разрабатываемая прошивка:** 1.0.05. Конфигурация/EEPROM/Serial-протокол реализуются в feature-ветке; CI и аппаратная проверка ещё не подтверждены.

## 1. Поток данных

1. Входы: AS5600 для рычагов L/R через TCA9548A; в будущем — отдельные выключатели и датчики.
2. Слой датчиков выбирает канал мультиплексора, считывает угол и сообщает о достоверности данных и ошибках связи.
3. Обработка осей разворачивает круговой диапазон, калибрует ход механизма, применяет инверсию и ограниченную фильтрацию, затем формирует нормализованные значения.
4. Интерпретация управления определяет фиксированные положения с гистерезисом и обрабатывает выбранный источник сигнала REVERSE.
5. Выходной слой передаёт оси/кнопки через USB HID, а состояние и настройки — через Serial.
6. Хранилище конфигурации проверяет и сохраняет настройки в EEPROM.

Достоверность датчика должна храниться отдельно от последнего измеренного значения. Текущая резервная стратегия версии 1.0.04 — удерживать последнее корректное значение HID, чтобы ошибка не вызывала резкий скачок оси, и явно сообщать через Serial об ошибке чтения (ERR_READ; HID HOLD) или ошибке диапазона (ERR_CAL). Удержанное значение не считается новым измерением. Это поведение ещё нужно проверить на целевом оборудовании и в MSFS.

## 2. Предлагаемые программные модули

Математика круговой калибровки вынесена в `Firmware/AirbusThrottleQuadrant/AxisMath.h` и покрывается host-тестами. Чтение датчиков использует тайм-аут Wire, статус AS5600 и попытку отключить каналы мультиплексора после транзакции. Эти изменения пока не считаются проверенными на аппаратуре.


| Модуль | Назначение |
|---|---|
| SensorManager | Выбор канала TCA9548A, чтение AS5600, достоверность данных и счётчики ошибок |
| AxisProcessor | Калибровка, обработка кругового диапазона, инверсия, фильтрация, нормализация |
| DetentManager | Состояния IDLE, CL, FLX/MCT и TOGA; пороги и гистерезис |
| ReverseInput | Выбор источника: ANGLE_ZONE, LIFT_SWITCH, BUTTON, DISABLED |
| ButtonManager | Будущие кнопки/переключатели, подавление дребезга и логические состояния |
| HidManager | Передача осей и кнопок через USB HID |
| ConfigStorage | Версионируемые настройки, проверка и сохранение в EEPROM |
| SerialProtocol | Команды, ответы, диагностика и связь с конфигуратором |

Сначала это логические границы. Разделять скетч на отдельные файлы будем постепенно — после аудита существующего кода и проверки компиляции.

## 3. Модель основных осей

Для каждого рычага задаются независимо:
- минимальное и максимальное сырое значение;
- инверсия направления;
- параметры фильтра;
- необязательные пороги фиксированных положений.

Выход AS5600 имеет разрядность 12 бит: 0–4095. Поскольку это круговое измерение, калибровка должна корректно работать, если механический диапазон пересекает границу 4095 → 0. Одинаковые конечные значения нужно считать ошибочной конфигурацией, а не трактовать молча как полный оборот.

Текущая привязка USB HID:
- рычаг L → ось X;
- рычаг R → ось Y.

Итоговый диапазон HID и поведение осей необходимо подтвердить в Windows и MSFS.

## 4. Модель фиксированных положений

Фиксированные положения — это логические состояния, вычисленные из откалиброванной позиции. Они не создают физические упоры: упоры обеспечивает механика.

Для каждого положения следует задавать порог и гистерезис — отдельные границы входа/выхода или эквивалентное правило с памятью состояния. Это предотвращает дрожание состояния около границы. Точные углы нельзя фиксировать до измерения готового механизма.

Начальные состояния: IDLE, CL, FLX_MCT и TOGA. REVERSE остаётся отдельным настраиваемым источником сигнала и не должен быть необратимо связан с основной обработкой угла.

## 5. Протокол конфигурации

Реализация Serial protocol v1 находится в разработке. Команды ASCII, разделитель — пробелы, конец команды — LF или CRLF; ответы — одна строка. Парсер обрабатывает входные байты в основном цикле без ожидания завершения строки, максимальная команда — 63 символа.

| Команда | Назначение |
|---|---|
| `GET_STATUS` | FW/protocol, RAW/HID L/R, valid и MD/ML/MH |
| `GET_CONFIG` | Прочитать активную конфигурацию |
| `CAL_START L|R` | Начать калибровку выбранного рычага |
| `CAL_MARK_MIN L|R` | Запомнить текущий RAW как IDLE/min |
| `CAL_MARK_MAX L|R` | Запомнить текущий RAW как TOGA/max и принять диапазон |
| `SET_INVERT L|R 0|1` | Установить инверсию |
| `SET_FILTER 1..100` | Установить EMA alpha в процентах |
| `SAVE_CONFIG` | Проверить и сохранить настройки в EEPROM |
| `LOAD_DEFAULTS` | Загрузить значения по умолчанию только в RAM |

Успешные команды отвечают `OK ...`, ошибки — `ERR <code>`. EEPROM не записывается автоматически. Команды пока не включают детенты и управление профилями.

Примечание: отладочные сообщения текущего скетча остаются в Serial, поэтому клиент должен распознавать строки по префиксам `STATUS`, `CONFIG`, `OK` и `ERR`, игнорируя строки диагностики.

## 6. Хранение настроек в EEPROM

`ConfigCore.h` определяет упакованную запись schema 1: сигнатура, версия, калибровочные концы и инверсия L/R, процент фильтра и CRC-16/CCITT-FALSE. Каждое поле проверяется независимо от CRC. `ConfigStorage.h` использует EEPROM, записывает только по явному `SAVE_CONFIG` и перечитывает запись для проверки.

При запуске:
1. прочитать сохранённые данные;
2. проверить сигнатуру, версию, допустимые диапазоны и целостность;
3. применить корректные настройки или безопасные значения по умолчанию;
4. сообщить через Serial об отсутствующей или некорректной конфигурации.

Избегать лишних записей и предусмотреть миграцию схемы при добавлении новых полей.

## 7. Конфигуратор Windows

Планируемый стек: Python + PySide6 и библиотека последовательного порта, например pyserial.

Начальные экраны:
- подключение, версия прошивки и состояние;
- текущие значения L/R и состояние датчиков;
- калибровка осей и инверсия;
- настройка фиксированных положений;
- профили конфигурации: импорт/экспорт;
- диагностика и ошибки команд.

Конфигуратор работает только через документированный Serial-протокол. Закрытие приложения или потеря Serial-соединения не должны останавливать USB HID.

## 8. Будущие входы

SPEED BRAKE, FLAPS и дополнительные кнопки намеренно отложены до стабилизации и калибровки L/R. Их интерфейсы добавим после определения механического хода, типа датчиков и электрических уровней. Архитектура должна позволять расширять число осей и кнопок без переписывания базового кода датчиков L/R.

## 9. Требования к проверке

- Скомпилировать с документированным пакетом платы и точной версией библиотеки Joystick.
- Проверить оба канала датчиков и намеренное отключение датчика.
- Проверить переход угла через ноль и крайние точки.
- Проверить направление и диапазон осей в свойствах игровых контроллеров Windows.
- Проверить назначения и фиксированные положения в целевом самолёте MSFS.
- Перезапустить питание и проверить сохранение EEPROM.
- Проверить некорректные Serial-команды и отключение конфигуратора.
- Записать реальные результаты; не описывать непроверенную прошивку как проверенную на оборудовании.

---

# Firmware Architecture — English

**Target MCU:** Arduino Pro Micro, ATmega32U4  
**Simulator:** Microsoft Flight Simulator  
**Control link:** USB HID  
**Configuration and diagnostics:** USB Serial  
**Current development firmware:** 1.0.04. Code and hardware behaviour have not yet completed final validation.

## 1. Data flow

1. Inputs: AS5600 sensors for levers L/R through TCA9548A; future discrete switches and sensors.
2. The sensor layer selects a multiplexer channel, reads the angle, and reports data validity and communication errors.
3. Axis processing unwraps circular travel, calibrates the mechanism, applies inversion and bounded filtering, then produces normalized values.
4. Control interpretation detects detents with hysteresis and handles the configured REVERSE input source.
5. The output layer sends axes/buttons via USB HID and status/configuration responses via Serial.
6. Configuration storage validates and persists settings in EEPROM.

Sensor validity must be stored separately from the last measured value. The current firmware 1.0.04 fallback policy is to hold the last valid HID axis value to avoid an abrupt jump and explicitly report a read fault (ERR_READ; HID HOLD) or invalid range (ERR_CAL) over Serial. A held value is not a new measurement. This policy still requires validation on the target hardware and in MSFS.

## 2. Suggested software modules

Circular calibration math is isolated in `Firmware/AirbusThrottleQuadrant/AxisMath.h` and covered by host tests. Sensor reads use Wire timeout support, AS5600 status diagnostics and an attempt to disable the multiplexer after each transaction. These changes are not yet hardware-validated.


| Module | Responsibility |
|---|---|
| SensorManager | TCA9548A channel selection, AS5600 reads, validity and error counters |
| AxisProcessor | Calibration, circular travel handling, inversion, filtering and normalization |
| DetentManager | IDLE, CL, FLX/MCT and TOGA states; thresholds and hysteresis |
| ReverseInput | Selectable source: ANGLE_ZONE, LIFT_SWITCH, BUTTON, DISABLED |
| ButtonManager | Future switches/buttons, debounce and logical states |
| HidManager | USB HID axis and button reports |
| ConfigStorage | Versioned settings, validation and EEPROM persistence |
| SerialProtocol | Commands, responses, diagnostics and configurator communication |

These are logical boundaries first. Split the sketch into separate files gradually, after auditing the existing code and verifying compilation.

## 3. Main axis model

Each lever has independent settings:
- raw minimum and maximum;
- direction inversion;
- filter parameters;
- optional detent thresholds.

AS5600 output is 12-bit (0–4095). Because the reading is circular, calibration must handle a mechanical range that crosses 4095 → 0. Equal endpoints must be treated as invalid configuration rather than silently interpreted as a full revolution.

Current USB HID mapping:
- lever L → X axis;
- lever R → Y axis.

The final HID range and axis behaviour must be confirmed in Windows and MSFS.

## 4. Detent model

Detents are logical states derived from calibrated position; they do not physically create the stops. Physical detents belong to the mechanical design.

Each detent should have a configurable threshold and hysteresis—separate entry/exit boundaries or an equivalent stateful rule—to prevent flicker near a boundary. Exact angles must not be fixed until the completed mechanism is measured.

Initial states: IDLE, CL, FLX_MCT and TOGA. REVERSE remains a separate configurable input source and must not be irreversibly coupled to the main angle processing.

## 5. Configuration protocol

Initially use newline-delimited text commands over USB Serial. The protocol is provisional and must be versioned before the PC application depends on it.

Planned command groups:
- status/version: `GET_STATUS`, `GET_CONFIG`;
- axis settings: read/change minimum, maximum, inversion and filter;
- detent settings: read/change thresholds;
- persistence: `SAVE_CONFIG`, `LOAD_DEFAULTS`.

Every command needs a clear success/error response. Validate bounds, safely reject unknown commands, and keep the sensor/HID loop non-blocking. Only explicit save commands should write EEPROM. Specify exact grammar, response format and maximum line length during implementation.

## 6. EEPROM configuration

Use a structure with a signature, schema version and integrity check. On startup:
1. read stored data;
2. verify signature, version, ranges and integrity;
3. apply valid configuration or safe defaults;
4. report missing or invalid configuration over Serial.

Avoid unnecessary writes and plan for schema migration as fields are added.

## 7. Windows configurator

Planned stack: Python + PySide6, with a serial-port library such as pyserial.

Initial screens:
- connection, firmware version and status;
- live L/R values and sensor health;
- axis calibration and inversion;
- detent configuration;
- configuration profiles (import/export);
- diagnostics and command errors.

The configurator communicates only through the documented Serial protocol. Closing the app or losing the Serial connection must not stop USB HID control.

## 8. Future inputs

SPEED BRAKE, FLAPS and extra buttons are deliberately deferred until L/R stability and calibration are complete. Add their interfaces after mechanical travel, sensor types and electrical levels are known. The architecture should allow additional axes and buttons without rewriting the core L/R sensor code.

## 9. Validation requirements

- Compile with the documented board package and exact Joystick library version.
- Test both sensor channels and deliberate sensor disconnection.
- Test angle wrap-around and endpoints.
- Confirm axis direction/range in Windows game-controller properties.
- Confirm assignments and detent behaviour in the target MSFS aircraft.
- Power-cycle and verify EEPROM configuration.
- Test malformed Serial commands and configurator disconnects.
- Record actual results; never describe untested firmware as hardware-validated.
