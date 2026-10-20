# Airbus Throttle Quadrant

Самодельный USB-квадрант управления тягой для Microsoft Flight Simulator (MSFS), вдохновлённый органами управления Airbus A320.

> **Статус:** прошивка 1.0.05 в разработке в feature-ветке. Добавляются калибровка через Serial, версионируемая конфигурация и EEPROM; CI и аппаратная проверка ещё не подтверждены.

## Лицензия / License

Проект распространяется по лицензии **GNU GPL v3.0 only**. Разрешены использование, изучение, изменение и распространение, в том числе коммерческое, при соблюдении условий GPL-3.0. При распространении проекта или производных работ необходимо соблюдать требования лицензии, включая предоставление соответствующего исходного кода в предусмотренных GPL случаях и сохранение уведомлений об авторских правах.

Код, заимствованный из других проектов, сохраняет применимые уведомления об авторстве и лицензионные условия. В частности, при включении кода FreeJoy необходимо сохранить его авторские уведомления и выполнить требования GPL-3.0.

The project is licensed under **GNU GPL v3.0 only**. Use, study, modification, and redistribution—including commercial redistribution—are permitted subject to GPL-3.0. When distributing the project or derivative works, comply with the license requirements, including providing corresponding source code where required and preserving copyright notices.

Code taken from other projects retains applicable copyright notices and license terms. In particular, if FreeJoy code is incorporated, preserve its notices and comply with GPL-3.0.


## Зафиксированные решения

- Контроллер: Arduino Pro Micro (ATmega32U4).
- Два основных рычага тяги: L и R, каждый с магнитным датчиком угла AS5600.
- Мультиплексор I²C TCA9548A: левый датчик — канал 0, правый — канал 1.
- USB HID: левый рычаг → ось X; правый рычаг → ось Y.
- ENGINE MASTER 1/2 и трёхпозиционный поворотный селектор передаются как HID-кнопки.
- Диагностика через USB Serial; планируемый конфигуратор Windows — Python + PySide6.
- Механическая цель: положения Airbus A320 — IDLE, CL, FLX/MCT и TOGA. Фиксированные положения создаются механикой, AS5600 измеряет угол.

## Текущая прошивка

Файл: `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.

- Чтение двух AS5600 через TCA9548A.
- USB HID: оси X/Y и четыре кнопки.
- Калибровочные концы, инверсия и фильтрация хранятся в runtime-конфигурации; параметры можно менять через Serial protocol v1.
- Поддерживается круговой диапазон, пересекающий 4095 → 0; одинаковые крайние углы отклоняются как ошибочная калибровка.
- При ошибке чтения датчика HID удерживает последнее корректное положение оси и сообщает `ERR_READ` / `HID HOLD`; некорректный диапазон сообщает `ERR_CAL`.
- Входы переключателей используют `INPUT_PULLUP` и подавление дребезга 25 мс.
- Диагностика и настройка Serial — 115200 бод. Команды: `GET_STATUS`, `GET_CONFIG`, `CAL_START L|R`, `CAL_MARK_MIN L|R`, `CAL_MARK_MAX L|R`, `SET_INVERT L|R 0|1`, `SET_FILTER 1..100`, `SAVE_CONFIG`, `LOAD_DEFAULTS`.
- Конфигурация EEPROM имеет сигнатуру, версию схемы и CRC; запись выполняется только командой `SAVE_CONFIG`. `LOAD_DEFAULTS` меняет настройки в RAM без записи.

### Распиновка переключателей

| Элемент | Вывод Pro Micro | HID |
|---|---:|---|
| ENGINE MASTER 1 | D4 | Кнопка 1 |
| ENGINE MASTER 2 | D5 | Кнопка 2 |
| Контакт A селектора | D6 | Кнопка 3 |
| Контакт B селектора | D7 | Кнопка 4 |

Каждый контакт подключается между цифровым выводом и GND. Для селектора с двумя нормально разомкнутыми контактами (2NO) предполагается: CRANK — контакт A замкнут, NORM — оба разомкнуты, IGN/START — контакт B замкнут. Перед подключением необходимо проверить реальную контактную схему мультиметром.

## Что ещё не завершено или не проверено

- OLED-меню калибровки без ПК (модуль выбран, GPIO и I²C-адрес ещё не проверены);
- программные детенты и отдельный сигнал REVERSE;
- GitHub Actions: фактический результат первого запуска CI нужно проверить после push.

- компиляция с выбранной версией платы и библиотеки Joystick;
- проверка прошивки на реальном оборудовании и в MSFS;
- логика фиксированных положений рычагов и REVERSE;
- конфигуратор Windows;
- SPEED BRAKE и FLAPS.

## Оборудование и сборка

- Arduino Pro Micro ATmega32U4; уточните версию платы (5 В/16 МГц или 3,3 В).
- 2 × AS5600, 1 × TCA9548A, механический квадрант и USB-кабель.
- Библиотека Arduino: **Joystick by Matthew Heironimus**.
- Адрес TCA9548A: `0x70`; AS5600: `0x36`; I²C: 400 кГц.

Откройте `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`, выберите подходящую плату Pro Micro/Leonardo, загрузите скетч и откройте Serial Monitor на 115200 бод. Перед подключением проверьте напряжение модулей и уровни логики I²C.

Схема подключения: [Electronics/WIRING.md](Electronics/WIRING.md). Механическая концепция крепления AS5600 и магнитов: [Mechanical/AS5600_MOUNTING.md](Mechanical/AS5600_MOUNTING.md). История изменений: [CHANGELOG.md](CHANGELOG.md). Дорожная карта: [Documentation/ROADMAP.md](Documentation/ROADMAP.md).

Это любительский контроллер, а не сертифицированное авиационное оборудование.

---

# Airbus Throttle Quadrant — English

DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.

> **Status:** firmware 1.0.05 is under development on a feature branch. Serial calibration, a versioned configuration and EEPROM storage are being added; CI and hardware validation are still pending.

## Fixed decisions

- Controller: Arduino Pro Micro (ATmega32U4).
- Two throttle levers L/R, each with an AS5600 magnetic angle sensor.
- TCA9548A I²C multiplexer: left sensor on channel 0, right sensor on channel 1.
- USB HID: left lever → X axis; right lever → Y axis.
- ENGINE MASTER 1/2 and a 3-position rotary selector are exposed as HID buttons.
- USB Serial diagnostics; planned Windows configurator uses Python + PySide6.
- Mechanical goal: Airbus A320-style IDLE, CL, FLX/MCT and TOGA detents. Mechanical detents are physical; AS5600 measures angle.

## Current firmware

File: `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.

- Reads two AS5600 sensors through TCA9548A.
- USB HID exposes X/Y axes and four buttons.
- Calibration endpoints, inversion and filtering are runtime configuration parameters exposed through Serial protocol v1.
- Supports circular angle ranges crossing 4095 → 0; equal endpoints are rejected.
- Holds the last valid HID axis value on sensor faults and reports `ERR_READ` / `HID HOLD`; invalid calibration reports `ERR_CAL`.
- Switch inputs use `INPUT_PULLUP` and 25 ms debounce.
- Serial diagnostics/configuration: 115200 baud. Commands: `GET_STATUS`, `GET_CONFIG`, `CAL_START L|R`, `CAL_MARK_MIN L|R`, `CAL_MARK_MAX L|R`, `SET_INVERT L|R 0|1`, `SET_FILTER 1..100`, `SAVE_CONFIG`, `LOAD_DEFAULTS`.
- EEPROM config uses a signature, schema version and CRC; it is written only by `SAVE_CONFIG`. `LOAD_DEFAULTS` changes RAM settings without writing EEPROM.

| Control | Pro Micro pin | HID |
|---|---:|---|
| ENGINE MASTER 1 | D4 | Button 1 |
| ENGINE MASTER 2 | D5 | Button 2 |
| Selector contact A | D6 | Button 3 |
| Selector contact B | D7 | Button 4 |

Connect each switch contact between its pin and GND. The assumed selector behaviour is for two normally-open contacts (2NO): CRANK closes A, NORM leaves both open, IGN/START closes B. Verify the actual switch contact diagram with a multimeter before wiring.

## Not yet complete or validated

- stand-alone OLED calibration menu (module selected; GPIO and I²C address are not yet verified);
- software detents and a separate REVERSE signal;
- verify the result of the first GitHub Actions run after the workflow starts.

- Compilation with the final board package and Joystick library.
- Real-hardware and MSFS tests.
- Throttle detent and REVERSE logic.
- Windows configurator.
- SPEED BRAKE and FLAPS.

## Build

Install the board definition for Pro Micro/Leonardo and **Joystick by Matthew Heironimus**. Open `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`, select the correct board, upload, and open Serial Monitor at 115200 baud.

Defaults: TCA9548A `0x70`, AS5600 `0x36`, I²C 400 kHz. Verify supply voltage and I²C logic levels for the exact breakout boards before wiring.

Wiring: [Electronics/WIRING.md](Electronics/WIRING.md). Preliminary AS5600 mounting concept: [Mechanical/AS5600_MOUNTING.md](Mechanical/AS5600_MOUNTING.md). Changelog: [CHANGELOG.md](CHANGELOG.md). Roadmap: [Documentation/ROADMAP.md](Documentation/ROADMAP.md).

This is a hobby controller, not certified aviation equipment.
