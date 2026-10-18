# Airbus Throttle Quadrant

Самодельный USB-квадрант управления тягой для Microsoft Flight Simulator (MSFS), вдохновлённый органами управления Airbus A320.

> **Статус:** разработка, прошивка **1.0.02**. Работа на целевом оборудовании пока не подтверждена. Изменения версии 1.0.02 улучшают диагностику ошибок датчиков и отклоняют некорректный диапазон калибровки; это не означает завершение проверки этапа.

## Зафиксированные решения

- Контроллер: Arduino Pro Micro (ATmega32U4).
- Два основных рычага тяги: L (левый) и R (правый), каждый с магнитным датчиком угла AS5600.
- Мультиплексор I²C TCA9548A: L — канал 0, R — канал 1.
- USB HID: L → ось X, R → ось Y.
- USB Serial: диагностика и отдельный конфигуратор для Windows.
- Конфигуратор: Python + PySide6.
- Механическая цель: характерные положения Airbus A320 — IDLE, CL, FLX/MCT и TOGA.
- Механизм REVERSE пока не выбран. Архитектура должна поддерживать угловую зону, отдельный датчик/выключатель подъёма рычага или отдельную кнопку/переключатель.
- SPEED BRAKE, FLAPS и дополнительные кнопки/переключатели добавим после стабилизации двух основных рычагов.

## Текущая прошивка

Файл: `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.

- Чтение двух AS5600 через TCA9548A.
- USB HID оси X/Y.
- Настраиваемые в исходном коде крайние углы, инверсия и фильтрация.
- Круговая арифметика диапазона поддерживает пересечение 4095 → 0.
- Одинаковые крайние углы отклоняются как ошибочная калибровка, а не трактуются как полный оборот.
- При ошибке чтения датчика прошивка удерживает последнее значение HID-оси и явно показывает `ERR_READ` и `HID HOLD` в Serial. При некорректном диапазоне выводится `ERR_CAL`.
- Диагностический вывод Serial — 115200 бод.

Пока не реализовано или не проверено:

- пользовательская калибровка и сохранение конфигурации в EEPROM;
- проверка на реальном оборудовании и компиляция с окончательно выбранным окружением;
- автомат состояний фиксированных положений рычага;
- абстракция источника сигнала REVERSE;
- конфигуратор для Windows;
- SPEED BRAKE, FLAPS и дополнительные кнопки.

**Политика ошибки датчика:** HID удерживает последнее корректное положение оси, чтобы не создавать резкий скачок. Это удержанное значение не считается новым измерением; ошибка обозначается в Serial. Такое поведение требует проверки на реальном оборудовании и в MSFS.

## Оборудование

- Arduino Pro Micro, ATmega32U4 (проверьте конкретную версию: 5 В/16 МГц или 3,3 В).
- 2 × модуля AS5600 и подходящие магниты.
- 1 × мультиплексор I²C TCA9548A.
- Механический квадрант и USB-кабель.

Перед подключением проверьте допустимое напряжение модулей и уровни логики I²C. Не считайте автоматически, что любая плата датчика допускает питание 5 В.

Схема подключения: [Electronics/WIRING.md](Electronics/WIRING.md).

## Сборка и загрузка

1. Установите Arduino IDE и подходящее описание платы Pro Micro/Leonardo.
2. Установите библиотеку **Joystick by Matthew Heironimus**.
3. Откройте скетч `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.
4. Проверьте выбор платы, выводы SDA/SCL, каналы датчиков и электрические уровни.
5. Загрузите прошивку и откройте Serial Monitor на скорости 115200 бод.

Скетч рассчитан на адрес TCA9548A `0x70`, левый рычаг L на канале 0 и правый R на канале 1. Компиляция и проверка на фактическом оборудовании пока не подтверждены.

## Документы проекта

- [Дорожная карта](Documentation/ROADMAP.md)
- [Архитектура прошивки](Documentation/FIRMWARE_ARCHITECTURE.md)
- [Схема подключения](Electronics/WIRING.md)
- [История изменений](CHANGELOG.md)

## Версии и проверка

Текущая разрабатываемая версия — 1.0.02. Этап нельзя считать завершённым, пока не выполнены критерии проверки на целевом оборудовании. Подробности — в CHANGELOG.md и ROADMAP.md.

Это любительский контроллер, а не сертифицированное авиационное оборудование. Физические фиксированные положения создаются механикой; AS5600 только измеряет угол.

---

# Airbus Throttle Quadrant — English

DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.

> **Status:** firmware **1.0.02** is under development. Target hardware has not been validated. This version improves sensor-fault diagnostics and rejects invalid calibration spans; it does not mean the milestone has passed validation.

## Fixed project decisions

- Controller: Arduino Pro Micro (ATmega32U4).
- Two main throttle levers: L (left) and R (right), each with an AS5600 magnetic angle sensor.
- TCA9548A I²C multiplexer: L on channel 0, R on channel 1.
- USB HID: L → X axis, R → Y axis.
- USB Serial: diagnostics and a separate Windows configurator.
- Configurator: Python + PySide6.
- Mechanical goal: Airbus A320-style IDLE, CL, FLX/MCT and TOGA detents.
- The REVERSE mechanism remains undecided. The architecture should support an angle zone, a separate lift switch/sensor, or a separate button/switch.
- SPEED BRAKE, FLAPS and additional buttons/switches are deferred until the two main levers are stable.

## Current firmware

File: `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.

- Reads two AS5600 sensors through TCA9548A.
- USB HID X/Y axes.
- Source-code endpoint settings, inversion and filtering.
- Circular arithmetic supports travel crossing 4095 → 0.
- Equal endpoints are rejected as invalid calibration instead of being treated as a full revolution.
- On sensor read failure, the last HID axis value is held and Serial explicitly reports `ERR_READ` and `HID HOLD`. Invalid calibration is reported as `ERR_CAL`.
- Serial diagnostics run at 115200 baud.

Not yet implemented or verified:

- user calibration and EEPROM configuration storage;
- compilation in the final selected environment and real-hardware validation;
- detent state machine;
- REVERSE input abstraction;
- Windows configurator;
- SPEED BRAKE, FLAPS and extra buttons.

**Sensor fault policy:** hold the last valid HID axis position to avoid an abrupt jump. The held value is not considered a new measurement; Serial reports the fault. This behaviour still requires testing on real hardware and in MSFS.

## Hardware

- Arduino Pro Micro, ATmega32U4 (verify the exact 5 V/16 MHz or 3.3 V variant).
- 2 × AS5600 modules and matching magnets.
- 1 × TCA9548A I²C multiplexer.
- Mechanical quadrant and USB cable.

Verify module voltage and I²C logic-level requirements before wiring. Do not assume all breakout boards tolerate 5 V.

Wiring: [Electronics/WIRING.md](Electronics/WIRING.md).

## Build

1. Install Arduino IDE and the matching Pro Micro/Leonardo board definition.
2. Install **Joystick by Matthew Heironimus**.
3. Open `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`.
4. Verify board selection, SDA/SCL pins, sensor channels and electrical levels.
5. Upload and open Serial Monitor at 115200 baud.

The sketch expects TCA9548A address `0x70`, L on channel 0 and R on channel 1. Compilation and hardware validation are not yet confirmed.

## Project documents

- [Roadmap](Documentation/ROADMAP.md)
- [Firmware architecture](Documentation/FIRMWARE_ARCHITECTURE.md)
- [Wiring](Electronics/WIRING.md)
- [Changelog](CHANGELOG.md)

## Versioning and validation

The current development version is 1.0.02. The milestone is not complete until its target-hardware exit criteria pass. See CHANGELOG.md and ROADMAP.md.

This is a hobby controller, not certified aviation equipment. Physical detents are created by the mechanism; AS5600 only measures angle.
