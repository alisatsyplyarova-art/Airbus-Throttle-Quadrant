# Дорожная карта / Roadmap — Airbus Throttle Quadrant

**Цель проекта:** самодельный USB-квадрант тяги для Microsoft Flight Simulator (MSFS), вдохновлённый Airbus A320.  
**Зафиксирована архитектура:** 2026-10-19  
**Текущая разрабатываемая версия прошивки:** 1.0.02 — критерии проверки ещё не подтверждены на целевом оборудовании.

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

### 1.0.02 — Надёжные датчики и USB HID
**Состояние:** исходный код обновлён; компиляция и аппаратная проверка ещё не подтверждены.

- Скомпилировать проект с точным описанием целевой платы Pro Micro и выбранной библиотекой Joystick.
- Проверить оба канала AS5600 через TCA9548A.
- Проверить, что ошибки чтения явно отображаются в Serial, а удержание последнего HID-значения не маскируется под новое измерение.
- Проверить ошибочную калибровку с одинаковыми конечными значениями.
- Проверить переход угла через границу 4095 → 0.
- Проверить диапазоны и направление HID-осей, стабильность обновления и повторное подключение USB.
- Записать результаты проверок аппаратуры и ПО.

**Критерии завершения:** устойчивые повторяемые показания и стабильные оси L/R на реальном оборудовании; ошибки видны в диагностике.

### 1.0.03 — Калибровка и хранение конфигурации
- Определить версионируемую структуру конфигурации.
- Реализовать пользовательскую калибровку каждого рычага без редактирования исходного кода. При проектировании алгоритма оценить подход с опорным углом и подписанными смещениями, используемый в OpenNauticalController, и сопоставить его с текущим круговым диапазоном от начала до конца.
- Проверять диапазоны и корректно обрабатывать переход через ноль.
- Сохранять параметры в EEPROM с сигнатурой/версией и проверкой целостности.
- Добавить команды Serial для чтения состояния и конфигурации, изменения параметров, сохранения и сброса настроек.

**Критерии завершения:** параметры сохраняются после выключения питания; неверная конфигурация безопасно отклоняется.

### 1.1.00 — Фиксированные положения тяги и REVERSE
- Реализовать настраиваемые пороги и гистерезис.
- Поддержать состояния IDLE, CL, FLX/MCT и TOGA.
- Добавить настраиваемый источник REVERSE: ANGLE_ZONE, LIFT_SWITCH, BUTTON или DISABLED.
- Проверить назначение в выбранном самолёте MSFS и описать особенности конкретного самолёта.

**Критерии завершения:** повторяемое распознавание положений и документированное поведение в выбранном самолёте симулятора.

### 1.2.00 — Конфигуратор Windows
- Создать отдельное приложение Python + PySide6.
- Обеспечить работу через USB Serial без блокировки USB HID.
- Показывать версию прошивки, состояние датчиков и текущие значения L/R.
- Добавить калибровку осей, направление, фильтрацию и настройку фиксированных положений.
- Поддержать импорт/экспорт профилей и понятные сообщения об ошибках команд.

**Критерии завершения:** приложение умеет читать, проверять, менять, сохранять и восстанавливать настройки реального контроллера.

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

## Правила работы

- Не считать этап завершённым до проверки его критериев.
- Делать небольшие изменения прошивки и обновлять CHANGELOG.md.
- Не заявлять проверку на оборудовании без фактических результатов.
- Не добавлять SPEED BRAKE и FLAPS в первую стабильную версию прошивки.

---

# Roadmap — English

**Project target:** DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.  
**Architecture baseline:** 2026-10-19  
**Current development firmware:** 1.0.02 — target-hardware validation criteria have not yet been confirmed.

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

### 1.0.02 — Reliable sensors and USB HID
**Status:** source updated; compilation and hardware validation are still pending.

- Compile with the exact target Pro Micro board definition and selected Joystick library.
- Verify both AS5600 channels through TCA9548A.
- Confirm that read faults are explicit in Serial and that a held HID value is not presented as a new measurement.
- Test rejection of equal calibration endpoints.
- Test angle wrap-around across 4095 → 0.
- Verify HID axis ranges, direction, update stability and USB reconnection.
- Record hardware and software test results.

**Exit criteria:** repeatable readings and stable L/R axes on actual hardware; faults are visible in diagnostics.

### 1.0.03 — Calibration and configuration storage
- Define a versioned configuration structure.
- Implement user calibration for each lever without editing source code. Evaluate the reference-angle plus signed-offset method used by OpenNauticalController against the current circular start-to-end range representation.
- Validate ranges and handle wrap-around correctly.
- Save settings to EEPROM with a signature/version and integrity check.
- Add Serial commands to read status/configuration, change values, save and restore defaults.

**Exit criteria:** settings survive power cycling; invalid configuration is rejected safely.

### 1.1.00 — Throttle detents and REVERSE
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

## Working rules

- Do not mark a milestone complete until its exit criteria are tested.
- Keep firmware changes small and update CHANGELOG.md.
- Never claim hardware validation without actual results.
- Keep SPEED BRAKE and FLAPS out of the first stable firmware milestone.
