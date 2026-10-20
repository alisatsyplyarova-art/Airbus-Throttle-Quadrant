# Дорожная карта / Roadmap — Airbus Throttle Quadrant

**Цель проекта:** самодельный USB-квадрант тяги для Microsoft Flight Simulator (MSFS), вдохновлённый Airbus A320.  
**Зафиксирована архитектура:** 2026-10-19  
**Текущая разрабатываемая версия прошивки:** 1.0.05 — реализуется конфигурация/EEPROM/Serial-протокол; CI и аппаратная проверка ожидают результата.

## Зафиксированные решения

- Основной контроллер: Arduino Pro Micro, ATmega32U4.
- Основные рычаги: L (левый) и R (правый), каждый измеряется магнитным датчиком угла AS5600.
- Оба AS5600 обычно имеют адрес I²C 0x36, поэтому используется мультиплексор TCA9548A: L — канал 0, R — канал 1.
- Интерфейс симулятора — USB HID. Текущая привязка: L → ось X, R → ось Y.
- USB Serial предназначен для диагностики и отдельного конфигуратора Windows.
- Конфигуратор: Python + PySide6.
- Механическая цель: фиксированные положения IDLE, CL, FLX/MCT и TOGA, характерные для Airbus A320. Геометрию и калибровочные значения нужно измерить на готовом механизме.
- Способ реализации REVERSE намеренно не выбран. Архитектура должна поддерживать угловую зону, отдельный выключатель/датчик подъёма рычага или отдельную кнопку/переключатель.
- SPEED BRAKE, FLAPS и дополнительные кнопки/переключатели откладываются до стабилизации двух основных рычагов.
- Указанные ниже версии — плановые этапы, а не утверждение о наличии этих функций.

## Этапы

### 1.0.04 — Надёжность датчиков, I²C и тесты
**Состояние:** исправления внесены; host-тесты проходят в GitHub Actions. Проверка компиляции Pro Micro и испытания на аппаратуре ожидают подтверждения.

- Устранить скачок при шуме за границей кругового диапазона: ограничивать значение ближайшим краем.
- Не публиковать произвольное стартовое значение 50%; обновлять ось после первого валидного чтения.
- Использовать I²C 400 кГц и тайм-аут 25 мс, отключать каналы TCA9548A после чтения.
- Читать STATUS AS5600: MD/ML/MH; сообщать о потере датчика после 10 подряд ошибок.
- Заменить блокирующий `delay(5)` планировщиком на `millis()`.
- Прогонять host-тесты круговой математики и автоматическую компиляцию через GitHub Actions.
- Скомпилировать проект с точным описанием целевой платы Pro Micro и выбранной библиотекой Joystick.
- Проверить оба канала AS5600 через TCA9548A.
- Проверить, что ошибки чтения явно отображаются в Serial, а удержание последнего HID-значения не маскируется под новое измерение.
- Проверить ошибочную калибровку с одинаковыми конечными значениями.
- Проверить переход угла через границу 4095 → 0.
- Проверить диапазоны и направление HID-осей, стабильность обновления и повторное подключение USB.
- Записать результаты проверок аппаратуры и ПО.

**Критерии завершения:** устойчивые повторяемые показания и стабильные оси L/R на реальном оборудовании; ошибки видны в диагностике.

### 1.0.05 — Калибровка и хранение конфигурации
**Состояние:** первая реализация добавлена в feature-ветку; прохождение CI, компиляция и аппаратная проверка ещё не подтверждены.

- Добавлены `ConfigCore.h` (схема 1, проверка полей, CRC-16/CCITT-FALSE) и `ConfigStorage.h` (EEPROM с чтением обратно после записи).
- Реализованы команды Serial v1: `GET_STATUS`, `GET_CONFIG`, `CAL_START L|R`, `CAL_MARK_MIN L|R`, `CAL_MARK_MAX L|R`, `SET_INVERT L|R 0|1`, `SET_FILTER 1..100`, `SAVE_CONFIG`, `LOAD_DEFAULTS`.
- Калибровка фиксирует RAW для IDLE и TOGA; одинаковые концы отвергаются. `LOAD_DEFAULTS` меняет RAM, но не записывает EEPROM.
- Парсер команд неблокирующий, строка ограничена 63 символами, неизвестные команды и некорректные диапазоны возвращают `ERR`.
- Проверять диапазоны и корректно обрабатывать переход через ноль.

**Критерии завершения:** параметры сохраняются после выключения питания; неверная конфигурация безопасно отклоняется.

### 1.1.00 — Детенты, REVERSE и интеграция с самолётами
- Реализовать настраиваемые пороги и гистерезис.
- Поддержать состояния IDLE, CL, FLX/MCT и TOGA.
- Добавить настраиваемый источник REVERSE: ANGLE_ZONE, LIFT_SWITCH, BUTTON или DISABLED.
- Проверить назначение в выбранном самолёте MSFS и описать особенности конкретного самолёта.

**Критерии завершения:** повторяемое распознавание положений и документированное поведение в выбранном самолёте симулятора.

### 1.2.00 — Конфигуратор Windows и общий интерфейс конфигурации
- Создать отдельное приложение Python + PySide6.
- Реализовать общий версионируемый Serial-протокол для конфигуратора и будущего OLED-меню; интерфейсы не должны дублировать логику калибровки в прошивке.
- Показывать версию прошивки, состояние датчиков и текущие значения L/R.
- Добавить калибровку осей, направление, фильтрацию и настройку фиксированных положений.
- Показывать живые шкалы L/R, RAW/HID, график сырых углов и состояние магнитного поля AS5600.
- Поддержать импорт/экспорт профилей и понятные сообщения об ошибках команд.
- Предусмотреть сохранение настроек в EEPROM только по явной команде и восстановление заводских настроек.

**Критерии завершения:** приложение умеет читать, проверять, менять, сохранять и восстанавливать настройки реального контроллера; отключение конфигуратора не влияет на USB HID.

### 1.2.01 — Автономный OLED-интерфейс
- Зафиксирован выбранный модуль: OLED 0,96″ 128×64 на контроллере SSD1315, I²C, питание 3,3 В, четыре независимых интерфейсных контакта кнопок K1–K4; размер модуля по описанию продавца 44,53×27,66 мм.
- Подключить OLED к общей I²C-шине с TCA9548A и AS5600; подтвердить на аппаратуре адрес (предварительно 0x3C), совместимость библиотеки/инициализации SSD1315 и устойчивость общей шины.
- Использовать четыре кнопки: UP, DOWN, OK и BACK. Назначение GPIO для K1–K4 определить после проверки схемы Pro Micro и фактической распиновки модуля; номера выводов пока не фиксируются.
- Реализовать меню: Calibrate L, Calibrate R, Invert, Filter, Save, Reset; UP/DOWN перемещают выбор, OK подтверждает, BACK возвращает/отменяет.
- Реализовать пошаговую калибровку IDLE → TOGA для каждого рычага, отмену и защиту от случайного сохранения.
- Показывать живые шкалы L/R, RAW/HID, качество магнитного поля MD/ML/MH и ошибки ERR_READ/ERR_CAL/ERR_EEPROM.
- Использовать тот же слой конфигурации, проверки и EEPROM, что и Windows-конфигуратор.

**Критерии завершения:** калибровка и настройка выполняются без ПК; экран и конфигуратор согласованно показывают и меняют одни и те же настройки.

### 1.3.00 — Дополнительные органы управления
- Добавить SPEED BRAKE и FLAPS после проверки основных рычагов.
- Выбрать датчики и переключатели после определения механики.
- Добавить кнопки/тумблеры и подавление дребезга.
- Расширить HID-отчёты и конфигуратор, не нарушая работу L/R.

**Критерии завершения:** новые органы управления проверяются в Windows и назначаются в MSFS.

### 2.0.00 — Механическая интеграция и подготовка релиза
- Опубликовать механические чертежи/STL после проверки.
- Подготовить инструкции по подключению, сборке и калибровке.
- Документировать версии прошивки, конфигуратора и схемы настроек.
- Выполнить полный регрессионный тест и описать известные ограничения.

## Новые требования и будущие функции

### Калибровка без ПК
- Долгое нажатие назначенной кнопки запускает калибровку.
- Для каждого рычага пользователь выставляет IDLE, затем TOGA; прошивка фиксирует сырые конечные значения и направление.
- Проверить диапазон, одинаковые концы и круговой переход; сохранить версионируемые параметры в EEPROM с сигнатурой и CRC.
- Предусмотреть отмену и безопасный выход из режима калибровки.

### Детенты и REVERSE
- Программные детенты IDLE/CL/FLX-MCT/TOGA как HID-кнопки рядом с аналоговой осью.
- Отдельный сигнал/ось для зоны REVERSE и профиль FlyByWire A32NX с учётом его детентов.
- Профильные пороги и гистерезис, не зашитые под один самолёт.
- Механические детенты: шарик с пружиной и печатная профилированная кулачковая пластина.

### Дополнительные органы управления
- Кнопки A/THR DISCONNECT и TOGA на рычагах.
- FLAPS — 5 положений; SPEED BRAKE — ось или энкодер после уточнения механики.
- Колесо триммера на AS5600 или энкодере.
- Парковочный тормоз и CRANK/NORM/START как отдельные кнопки или логическое HID-событие.
- Индикаторы ENG MASTER FAULT/FIRE и другие лампы через исследуемую интеграцию MobiFlight или SimConnect.

### Конфигуратор и профили
- Профили самолётов: FlyByWire A320, Fenix и штатный A320neo.
- Диагностика: график L/R в реальном времени и шкала статуса магнитного поля MD/ML/MH.
- Сохранять host-тесты математики и CI-компиляцию как обязательную проверку.

## Правила работы

- Не считать этап завершённым до проверки его критериев.
- Делать небольшие изменения прошивки и обновлять CHANGELOG.md.
- Не заявлять проверку на оборудовании без фактических результатов.
- Не добавлять SPEED BRAKE и FLAPS в первую стабильную версию прошивки.

---

# Roadmap — English

**Project target:** DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.  
**Architecture baseline:** 2026-10-19  
**Current development firmware:** 1.0.05 — configuration/EEPROM/Serial implementation is in progress; CI and hardware validation are pending.

## Fixed decisions

- Main controller: Arduino Pro Micro, ATmega32U4.
- Main levers: L (left) and R (right), each measured by an AS5600 magnetic angle sensor.
- Both AS5600 sensors normally use I²C address 0x36, so a TCA9548A multiplexer is used: L on channel 0, R on channel 1.
- Simulator interface: USB HID. Current mapping: L → X axis, R → Y axis.
- USB Serial is reserved for diagnostics and a separate Windows configurator.
- Configurator: Python + PySide6.
- Mechanical goal: Airbus A320-style IDLE, CL, FLX/MCT and TOGA detents. Measure final geometry and calibration values on the completed mechanism.
- REVERSE implementation is intentionally undecided. The architecture must support an angle zone, a separate lift switch/sensor, or a separate button/switch.
- SPEED BRAKE, FLAPS and extra buttons/switches are deferred until the two main levers are stable.
- The versions below are planned milestones, not claims that all features already exist.

## Milestones

### 1.0.04 — Sensor reliability, I²C and tests
**Status:** fixes are committed; host tests pass in GitHub Actions. Pro Micro compilation and hardware tests still need confirmation.

- Clamp out-of-range circular samples to the nearest endpoint to prevent jumps near IDLE.
- Do not publish an arbitrary 50% startup axis value; update each axis after its first valid sample.
- Use 400 kHz I²C and a 25 ms timeout; disable TCA9548A channels after reads.
- Read AS5600 STATUS MD/ML/MH and report sensor loss after 10 consecutive failures.
- Replace blocking `delay(5)` with a `millis()` scheduler.
- Run host tests for circular math and automatic Pro Micro compilation in GitHub Actions.
- Compile with the exact target Pro Micro board definition and selected Joystick library.
- Verify both AS5600 channels through TCA9548A.
- Confirm that read faults are explicit in Serial and that a held HID value is not presented as a new measurement.
- Test rejection of equal calibration endpoints.
- Test angle wrap-around across 4095 → 0.
- Verify HID axis ranges, direction, update stability and USB reconnection.
- Record hardware and software test results.

**Exit criteria:** repeatable readings and stable L/R axes on actual hardware; faults are visible in diagnostics.

### 1.0.05 — Calibration and configuration storage
- Define a versioned configuration structure.
- Implement user calibration for each lever without editing source code. Evaluate the reference-angle plus signed-offset method used by OpenNauticalController against the current circular start-to-end range representation.
- Validate ranges and handle wrap-around correctly.
- Save settings to EEPROM with a signature/version and integrity check.
- Add Serial commands to read status/configuration, change values, save and restore defaults.

**Exit criteria:** settings survive power cycling; invalid configuration is rejected safely.

### 1.1.00 — Detents, REVERSE and aircraft integration
- Implement configurable thresholds and hysteresis.
- Support IDLE, CL, FLX/MCT and TOGA.
- Add a configurable REVERSE source: ANGLE_ZONE, LIFT_SWITCH, BUTTON or DISABLED.
- Test assignments with the selected MSFS aircraft and document aircraft-specific behaviour.

**Exit criteria:** repeatable detent recognition and documented behaviour in the chosen simulator aircraft.

### 1.2.00 — Windows configurator
- Build a separate Python + PySide6 application.
- Communicate over USB Serial without blocking USB HID.
- Display firmware version, sensor health and live L/R values.
- Provide axis calibration, direction, filtering and detent settings.
- Support profile import/export and clear command-error messages.

**Exit criteria:** the application can read, validate, change, save and restore settings on the actual controller.

### 1.3.00 — Additional controls
- Add SPEED BRAKE and FLAPS after validating the main levers.
- Select sensors and switches after the mechanical design is known.
- Add buttons/toggles and debounce handling.
- Extend HID reports and the configurator without destabilising L/R axes.

**Exit criteria:** new controls are testable in Windows and assignable in MSFS.

### 2.0.00 — Mechanical integration and release preparation
- Publish validated mechanical drawings/STL files.
- Publish wiring, assembly and calibration instructions.
- Document firmware, configurator and configuration-schema versions.
- Run a full regression checklist and document known limitations.

## New requirements and future features

### Stand-alone calibration
- A long press of a designated button enters calibration mode.
- For each lever, guide the user through IDLE then TOGA; capture raw endpoints and direction.
- Validate the span, equal endpoints and circular wrap-around; store versioned settings in EEPROM with signature and CRC.
- Provide cancellation and a safe exit from calibration mode.

### Detents and REVERSE
- Expose software IDLE/CL/FLX-MCT/TOGA detents as HID buttons alongside the analog axis.
- Consider a separate REVERSE signal/axis and a FlyByWire A32NX profile respecting its detent expectations.
- Keep thresholds and hysteresis aircraft-profile-specific.
- Mechanical detents: spring-loaded ball and a 3D-printed profiled cam plate.

### Additional controls
- Lever-mounted A/THR DISCONNECT and TOGA buttons.
- Five-position FLAPS; SPEED BRAKE as an axis or encoder after the mechanics are defined.
- Trim wheel using AS5600 or an encoder.
- Parking brake and CRANK/NORM/START as separate buttons or a logical HID event.
- ENG MASTER FAULT/FIRE and other annunciators through a researched MobiFlight or SimConnect integration.

### Configurator and profiles
- Aircraft profiles for FlyByWire A320, Fenix and the default A320neo.
- Diagnostics with live L/R graphs and AS5600 magnetic-field status indicators MD/ML/MH.
- Keep host math tests and CI compilation mandatory.

## Working rules

- Do not mark a milestone complete until its exit criteria are tested.
- Keep firmware changes small and update CHANGELOG.md.
- Never claim hardware validation without actual results.
- Keep SPEED BRAKE and FLAPS out of the first stable firmware milestone.

## Анализ FreeJoy — функции для возможного переноса

**Источник для изучения:** [FreeJoy firmware](https://github.com/FreeJoy-Team/FreeJoy), [настройка осей](https://github.com/FreeJoy-Team/FreeJoyWiki/blob/master/eng/Axis-configuration.md), [FreeJoy Configurator Qt](https://github.com/FreeJoy-Team/FreeJoyConfiguratorQt). FreeJoy распространяется под GPL-3.0. Наш проект теперь использует GPL-3.0-only; при фактическом переносе кода нужно сохранять исходные copyright/license notices и проверять лицензию конкретного файла и зависимостей.

**Ограничения совместимости:** FreeJoy ориентирован на STM32F103, а наш контроллер — ATmega32U4; у FreeJoy AS5600 заявлен как SPI-источник, у нас AS5600 подключены по I²C через TCA9548A. Нельзя без проверки переносить драйверы датчиков, HAL, USB/HID и код, завязанный на STM32. В первую очередь изучать алгоритмы и переносить только подходящие части, адаптируя их под текущую архитектуру.

### Приоритет A — диагностика и настройка осей
- [ ] В конфигураторе показывать отдельно RAW (физическое значение AS5600) и OUT/HID (значение после калибровки и обработки) для L/R.
- [ ] Добавить живые шкалы обеих осей и ограниченный по размеру график истории; показать MD/ML/MH, валидность чтения и ошибки датчиков.
- [ ] Добавить проверку и визуализацию калибровки: минимальное/максимальное RAW, направление, эффективный круговой диапазон и состояние за пределами калибровки.
- [ ] Сохранить поведение удержания последнего корректного HID-значения при ошибке датчика, но явно отображать ошибку и не выдавать удержанное значение за свежее измерение.

### Приоритет B — управляемая фильтрация и точность
- [ ] Сравнить текущий EMA-фильтр с несколькими понятными режимами сглаживания; показывать компромисс между дрожанием и задержкой.
- [ ] Исследовать динамический deadband для подавления мелких колебаний неподвижного рычага. Настройка по умолчанию — выключена; значение и единицы должны быть понятны, без скачков у границ и потери полного хода.
- [ ] Добавить регулируемую кривую отклика: Linear по умолчанию, затем Exponential/Inverse Exponential; произвольную кривую вводить только если интерфейс и память позволяют.
- [ ] Рассмотреть настройку выходного разрешения/масштабирования только после проверки реального HID-диапазона Windows. Не менять существующий диапазон осей незаметно для пользователя.
- [ ] Добавить host-тесты для каждой функции: монотонность, точные конечные значения, инверсия, круговой переход 4095→0, границы deadband, экстремальные параметры и отсутствие переполнений.

### Приоритет C — детенты и логические кнопки
- [ ] Использовать идею «ось → кнопки» для распознавания IDLE, CL, FLX/MCT и TOGA, если она подходит к модели управления; для каждой зоны задать гистерезис и проверку дребезга состояния.
- [ ] Не смешивать аналоговый выход оси с состоянием детента: HID-ось продолжает работать, а детенты передаются отдельно только после утверждения HID-контракта.
- [ ] Для REVERSE оставить отдельный выбираемый источник ANGLE_ZONE/LIFT_SWITCH/BUTTON/DISABLED; не привязывать его необратимо к общей кривой оси.
- [ ] Для будущих тумблеров изучить явные режимы Toggle ON/OFF и обработку переходов состояний, но не менять текущую распиновку D4–D7 без отдельного решения.

### Приоритет D — конфигуратор и профили
- [ ] Развить PySide6-конфигуратор: живые RAW/OUT, график отклика выбранной кривой, проверка параметров до отправки и понятные ошибки протокола.
- [ ] Добавить импорт/экспорт пользовательских профилей в версионируемый JSON; включить схему профиля, версию прошивки/конфигурации и проверку значений.
- [ ] Поддержать профили самолётов только для порогов детентов и назначений, которые действительно различаются; базовую калибровку конкретного устройства хранить отдельно от профиля самолёта.
- [ ] Сохранять в EEPROM только по явной команде пользователя; не выполнять запись при каждом движении слайдера. Предусмотреть проверку после записи и безопасную миграцию схемы конфигурации.

### Порядок внедрения
1. Сначала RAW/OUT и диагностика в конфигураторе.
2. Затем тесты и сравнение режимов фильтрации; deadband — отдельная опция, выключенная по умолчанию.
3. После этого — кривые отклика и их график.
4. Далее — детенты с гистерезисом и отдельные события HID.
5. В последнюю очередь — профили JSON и расширенные типы логики переключателей.

**Критерии приёмки:** никаких изменений поведения по умолчанию без необходимости; существующая калибровка и Serial protocol остаются совместимыми либо версия протокола явно повышается; все математические функции покрыты host-тестами; прошивка компилируется для Pro Micro; аппаратные заявления делаются только после реальных испытаний.

## FreeJoy review — candidate roadmap items

**References:** [FreeJoy firmware](https://github.com/FreeJoy-Team/FreeJoy), [axis configuration guide](https://github.com/FreeJoy-Team/FreeJoyWiki/blob/master/eng/Axis-configuration.md), [FreeJoy Configurator Qt](https://github.com/FreeJoy-Team/FreeJoyConfiguratorQt). FreeJoy is GPL-3.0. This project now uses GPL-3.0-only; when copying code, preserve the original copyright/license notices and review the license of each file and dependency.

**Compatibility limits:** FreeJoy targets STM32F103, while this project uses ATmega32U4. FreeJoy lists AS5600 as an SPI source; our AS5600 sensors use I²C through TCA9548A. Do not transplant STM32 drivers, HAL, USB/HID or hardware-specific code without review. Prioritise reusable algorithms and adapt them to the current architecture.

### Priority A — axis diagnostics
- [ ] Show RAW (physical AS5600 value) separately from OUT/HID (calibrated and processed output) for both axes.
- [ ] Add live L/R bars and a bounded history graph; display MD/ML/MH, read validity and sensor errors.
- [ ] Visualise calibration endpoints, direction, effective circular span and out-of-range samples.
- [ ] Keep last-valid HID output on sensor failure, but expose the fault and never label the held value as a fresh measurement.

### Priority B — filtering and response
- [ ] Compare the current EMA filter with a small set of understandable smoothing modes; show the jitter-versus-latency trade-off.
- [ ] Evaluate dynamic deadband for stationary-axis jitter. Default it to off; document units and avoid edge jumps or loss of full travel.
- [ ] Add response curves: Linear by default, then Exponential/Inverse Exponential. Add user-defined curves only if UI and memory budgets allow.
- [ ] Consider output resolution/scaling only after verifying the actual Windows HID range; never silently change the existing axis range.
- [ ] Add host tests for monotonicity, exact endpoints, inversion, 4095→0 wrap-around, deadband boundaries, extreme settings and overflow safety.

### Priority C — detents and logical buttons
- [ ] Evaluate axis-to-button logic for IDLE, CL, FLX/MCT and TOGA; use hysteresis and state debounce for each zone.
- [ ] Keep analog axis output independent from detent state; expose detents as separate HID events only after the HID contract is approved.
- [ ] Keep REVERSE as a selectable ANGLE_ZONE/LIFT_SWITCH/BUTTON/DISABLED source, independent from the general axis curve.
- [ ] Study explicit Toggle ON/OFF semantics for future switches; do not change existing D4–D7 pin mapping without a separate decision.

### Priority D — configurator and profiles
- [ ] Extend the PySide6 configurator with live RAW/OUT, response-curve preview, pre-send validation and clear protocol errors.
- [ ] Add import/export of versioned JSON profiles with schema, firmware/config version and value validation.
- [ ] Add aircraft profiles only for detent thresholds and assignments that genuinely differ; keep per-device calibration separate from aircraft profiles.
- [ ] Write EEPROM only after explicit user action, never on every slider movement. Verify writes and plan safe configuration-schema migration.

### Implementation order
1. RAW/OUT and diagnostics in the configurator.
2. Tests and comparison of filtering modes; deadband as a separate opt-in feature.
3. Response curves and curve preview.
4. Detents with hysteresis and separate HID events.
5. JSON profiles and advanced switch logic.

**Acceptance criteria:** preserve current default behaviour; keep the existing calibration and Serial protocol compatible or explicitly version protocol changes; cover math with host tests; compile for Pro Micro; claim hardware validation only after real tests.

