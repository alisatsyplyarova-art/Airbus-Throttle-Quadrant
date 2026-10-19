# История изменений / Changelog

## Не выпущено — выбран OLED-модуль с четырьмя кнопками (2026-10-19) / Unreleased — four-button OLED module selected (2026-10-19)

- Зафиксирован модуль OLED 0,96″ 128×64 с контроллером SSD1315, интерфейсом I²C, питанием 3,3 В и четырьмя независимыми контактами кнопок K1–K4; размеры по описанию продавца — 44,53×27,66 мм.
- Запланированы функции кнопок UP, DOWN, OK и BACK для автономного меню настройки и калибровки.
- Адрес I²C (предположительно 0x3C), GPIO для K1–K4, совместимость библиотеки/инициализации SSD1315 и работа на общей шине с TCA9548A/AS5600 остаются предметом аппаратной проверки.
- Это архитектурное решение; модуль ещё не подключён и не проверен на реальном устройстве.

- Selected a 0.96-inch 128×64 OLED module using SSD1315, I²C, 3.3 V, and four independent K1–K4 button interfaces; seller-listed dimensions are 44.53×27.66 mm.
- Planned button roles: UP, DOWN, OK and BACK for the stand-alone configuration/calibration menu.
- The I²C address (expected 0x3C), GPIO mapping for K1–K4, SSD1315 library/initialization compatibility, and operation on the shared TCA9548A/AS5600 bus remain subject to hardware verification.
- This records an architectural choice; the module has not yet been connected or tested on hardware.


## 1.0.04 — Надёжность датчиков и тесты / Sensor reliability and tests

- Исправлена круговая калибровка: значение за пределами диапазона ограничивается ближайшим краем по круговому расстоянию.
- Убрано произвольное стартовое значение осей 50%; оси обновляются после первого валидного измерения.
- Добавлены тайм-аут I²C 25 мс, скорость 400 кГц и попытка отключить все каналы TCA9548A после чтения.
- Добавлено чтение STATUS AS5600 (0x0B): MD — магнит обнаружен, ML/MH — предупреждения о слабом/сильном поле.
- Добавлена диагностика потери датчика после 10 подряд неудачных чтений и сообщение о восстановлении.
- Убран delay(5); опрос выполняется по millis() с интервалом 10 мс.
- Математика вынесена в AxisMath.h; добавлены host-тесты и GitHub Actions.

**Проверка:** изменения сохранены в репозитории, но компиляция CI и аппаратная проверка должны быть подтверждены отдельно. Wire.setWireTimeout() требует совместимого AVR Wire core.

- Fixed circular calibration so out-of-range readings clamp to the nearest endpoint by circular distance.
- Removed the arbitrary 50% startup axis value; axes update after the first valid sample.
- Added a 25 ms I²C timeout, 400 kHz bus speed, and an attempt to disable all TCA9548A channels after reads.
- Read AS5600 STATUS (0x0B): MD indicates magnet detected; ML/MH warn about weak/strong field.
- Added sensor-loss reporting after 10 consecutive failed reads and recovery notification.
- Replaced delay(5) with a millis()-scheduled 10 ms poll.
- Isolated math in AxisMath.h; added host tests and GitHub Actions.

**Validation:** CI compilation and hardware validation still need confirmation. Wire.setWireTimeout() requires a compatible AVR Wire core.


## 1.0.04 — Надёжность датчиков и тесты / Sensor reliability and tests

- Исправлена обработка значения вне кругового калибровочного диапазона: оно ограничивается ближайшим краем по круговому расстоянию, а не всегда концом диапазона.
- Убрано произвольное стартовое значение осей 50%; оси обновляются только после первого валидного измерения.
- Добавлен тайм-аут I²C 25 мс, скорость 400 кГц и попытка отключения всех каналов TCA9548A после чтения.
- Добавлено чтение регистра STATUS AS5600 (`0x0B`): MD сообщает об обнаружении магнита, ML/MH дают предупреждения о слабом/сильном поле.
- Добавлена диагностика потери датчика после 10 подряд неудачных чтений/отсутствия магнита и сообщение о восстановлении.
- Убран `delay(5)`: опрос датчиков запускается по `millis()` с интервалом 10 мс.
- Математика круговой калибровки вынесена в `AxisMath.h`; добавлены host-тесты и GitHub Actions для тестов и компиляции Pro Micro.

**Проверка:** код внесён в репозиторий. До результата GitHub Actions и проверки на реальном устройстве прошивку не считать полностью проверенной. `Wire.setWireTimeout()` требует совместимого AVR Wire core.

- Fixed out-of-range circular calibration so samples clamp to the nearest endpoint by circular distance.
- Removed arbitrary 50% startup axis values; axes update after the first valid sample.
- Added a 25 ms I²C timeout, 400 kHz bus speed, and an attempt to disable all TCA9548A channels after reads.
- Read AS5600 STATUS register (`0x0B`): MD detects the magnet, ML/MH warn about weak/strong field.
- Added sensor-loss reporting after 10 consecutive failed reads/missing-magnet samples and recovery notification.
- Replaced `delay(5)` with a `millis()`-scheduled 10 ms sensor poll.
- Moved circular calibration math into `AxisMath.h`; added host tests and GitHub Actions for tests and Pro Micro compilation.

**Validation:** changes are committed, but the firmware is not considered fully validated until CI and real-hardware checks pass. `Wire.setWireTimeout()` requires a compatible AVR Wire core.

## Не выпущено — механическая концепция AS5600 (2026-10-19) / Unreleased — AS5600 mounting concept

- Добавлен документ `Mechanical/AS5600_MOUNTING.md` с предварительной конструкцией держателя магнита и регулируемого кронштейна AS5600.
- Зафиксированы стартовые параметры для CAD-прототипа, перечень необходимых измерений и последовательность механической проверки.
- Размеры магнитов, осей, зазор и окончательная геометрия оставлены неподтверждёнными до фактических измерений; STL/STEP-модели ещё не созданы.

**Статус:** документация фиксирует концепцию, а не готовый чертёж или проверенную механическую конструкцию.

- Added `Mechanical/AS5600_MOUNTING.md` describing a preliminary magnet holder and adjustable AS5600 sensor bracket.
- Recorded initial CAD parameters, required measurements, and the mechanical validation workflow.
- Magnet/shaft dimensions, air gap, and final geometry remain unconfirmed pending measurements; no STL/STEP models have been created yet.

**Status:** this records a concept, not a finalized drawing or validated mechanical design.

## 1.0.03 — Переключатели двигателя и селектор / Engine switches and selector

- Добавлены входы ENGINE MASTER 1 и ENGINE MASTER 2 на D4 и D5.
- Добавлены два входа трёхпозиционного поворотного селектора на D6 и D7.
- HID настроен на четыре кнопки: 1/2 — ENGINE MASTER 1/2, 3/4 — контакты A/B селектора.
- Входы используют `INPUT_PULLUP`, замыкание на GND означает ON; добавлено программное подавление дребезга 25 мс.
- В Serial добавлена диагностика состояний переключателей.
- Обновлена схема подключения.

**Проверка:** код и документация внесены в репозиторий. Компиляция, соответствие конкретного селектора схеме 2NO и испытание на реальном оборудовании ещё не подтверждены.

- Added ENGINE MASTER 1 and ENGINE MASTER 2 inputs on D4 and D5.
- Added two inputs for a 3-position rotary selector on D6 and D7.
- Configured four HID buttons: 1/2 for the engine master switches, 3/4 for selector contacts A/B.
- Inputs use `INPUT_PULLUP`; contact closure to GND means ON. Added 25 ms software debounce.
- Added switch-state diagnostics to Serial.
- Updated the wiring guide.

**Validation:** source and documentation have been committed. Compilation, the actual selector's 2NO contact behaviour, and real-hardware testing remain unconfirmed.

## 1.0.02 — Обработка ошибок датчиков и проверка диапазона / Sensor faults and calibration span validation

- Обновлена диагностика: при ошибке чтения `ERR_READ` явно указывает, что значение HID удерживается (`HID HOLD`), а не обновляется новым измерением.
- При некорректном диапазоне калибровки выводится `ERR_CAL`; одинаковые конечные значения больше не превращаются в диапазон почти на полный оборот.
- Сохранена круговая арифметика для диапазона, пересекающего 4095 → 0.
- Выбран предварительный резервный режим при ошибке датчика: удерживать последнее корректное значение HID, явно сообщая об ошибке через Serial.

**Проверка:** изменения в исходном коде внесены, но компиляция и проверка на реальном оборудовании ещё не подтверждены. Этап 1.0.02 не следует считать завершённым до прохождения критериев из ROADMAP.md.

- Diagnostics explicitly report `ERR_READ` and `HID HOLD` when a sensor read fails.
- Invalid calibration ranges report `ERR_CAL`; equal endpoints are not treated as an almost-full-turn range.
- Circular arithmetic supports travel crossing 4095 → 0.
- Provisional sensor-fault fallback holds the last valid HID value and reports the fault over Serial.

**Validation:** compilation and real-hardware testing remain unconfirmed.

## Не выпущено — двуязычная документация (2026-10-19) / Unreleased — bilingual documentation (2026-10-19)

- Установлен приоритет русского языка: русская версия документа идёт первой, английская — следом.
- Обновлён README.md; дорожная карта и архитектура прошивки подготовлены к двуязычному ведению.
- Сохранены зафиксированные решения по MSFS, Arduino Pro Micro ATmega32U4, рычагам L/R, датчикам AS5600 через TCA9548A и USB HID (L → X, R → Y).
- Подтверждён выбор Python + PySide6 для планируемого конфигуратора Windows через USB Serial.
- Механизм REVERSE намеренно оставлен открытым: угловая зона, отдельный датчик/выключатель подъёма, кнопка/переключатель или отключённый режим.
- SPEED BRAKE, FLAPS и дополнительные органы управления отложены до стабилизации основных рычагов.

**Это обновление документации не означает, что оборудование прошло проверку.**

- Set Russian as the primary language, followed by English.
- Updated README.md and prepared the roadmap and firmware architecture for bilingual maintenance.
- Preserved decisions for MSFS, Arduino Pro Micro ATmega32U4, L/R levers, AS5600 through TCA9548A, and USB HID mapping.
- Confirmed Python + PySide6 for the planned Windows configurator over USB Serial.
- Kept the REVERSE mechanism open.
- Deferred SPEED BRAKE, FLAPS and additional controls until the main levers are stable.

**This documentation update does not claim hardware validation.**

## 1.0.01 — Начальная версия разработки / Initial development baseline

- Создана начальная структура репозитория и документация.
- В документации определены два канала управления тягой L и R.
- Предусмотрено чтение датчиков AS5600 через мультиплексор I²C TCA9548A.
- Предусмотрены оси USB HID X/Y, калибровка в исходном коде и инверсия.
- Добавлена базовая диагностика через Serial и фильтрация входных данных.

**Проверка:** работа на реальном оборудовании не подтверждена. Перед использованием необходимо скомпилировать проект с точным пакетом платы Arduino Pro Micro и библиотекой Joystick и провести испытания.

- Initial repository structure and documentation.
- Two throttle axes, L and R, documented.
- AS5600 sensors read through a TCA9548A I²C multiplexer.
- USB HID joystick X/Y axes, source-code calibration and inversion.
- Basic Serial diagnostics and input filtering.

**Validation:** not yet hardware-validated. Compile with the exact Arduino Pro Micro board package and Joystick library, then test on the actual hardware.
