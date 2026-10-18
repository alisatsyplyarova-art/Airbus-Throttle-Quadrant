# Airbus Throttle Quadrant

Самодельный USB-квадрант управления тягой для Microsoft Flight Simulator (MSFS), вдохновлённый органами управления Airbus A320.

> **Статус:** начальная версия проекта; прошивка **1.0.01**. Работа на целевом оборудовании пока не подтверждена. Архитектура и дорожная карта описаны в документации; запланированные функции не обязательно реализованы.

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

## Текущая базовая версия прошивки

Реализовано в `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`:
- базовое чтение AS5600 через TCA9548A;
- базовые оси USB HID X/Y;
- калибровочные константы в исходном коде, инверсия и фильтрация;
- диагностический вывод Serial.

Пока не реализовано или не проверено:
- надёжная обработка ошибок датчиков на реальном оборудовании;
- калибровка пользователем и сохранение конфигурации в EEPROM;
- автомат состояний фиксированных положений рычага;
- абстракция источника сигнала REVERSE;
- конфигуратор для Windows;
- SPEED BRAKE, FLAPS и дополнительные кнопки.

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

Скетч рассчитан на адрес TCA9548A `0x70`, левый рычаг L на канале 0 и правый R на канале 1. Требуется компиляция и проверка на фактическом оборудовании.

## Документы проекта

- [Дорожная карта](Documentation/ROADMAP.md)
- [Архитектура прошивки](Documentation/FIRMWARE_ARCHITECTURE.md)
- [Схема подключения](Electronics/WIRING.md)
- [История изменений](CHANGELOG.md)

## Версии и проверка

Текущая базовая версия прошивки — 1.0.01. Плановые этапы описаны в дорожной карте; этап нельзя считать завершённым, пока не проверены критерии его готовности. Подробности — в CHANGELOG.md.

Это любительский контроллер, а не сертифицированное авиационное оборудование. Физические фиксированные положения создаются механикой; AS5600 только измеряет угол.

---

# Airbus Throttle Quadrant — English

DIY USB throttle quadrant for Microsoft Flight Simulator (MSFS), inspired by Airbus A320 controls.

> **Status:** initial development baseline, firmware **1.0.01**. Operation on target hardware has not yet been validated. The architecture and roadmap are documented; planned features are not necessarily implemented.

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

## Current firmware baseline

Implemented in `Firmware/AirbusThrottleQuadrant/AirbusThrottleQuadrant.ino`:
- basic AS5600 reads through TCA9548A;
- basic USB HID X/Y axes;
- source-code calibration constants, inversion and filtering;
- Serial debug output.

Not yet implemented or verified:
- reliable sensor error handling on actual hardware;
- user-facing calibration and EEPROM configuration;
- detent state machine;
- REVERSE input abstraction;
- Windows configurator;
- SPEED BRAKE, FLAPS and extra buttons.

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

The sketch expects TCA9548A address `0x70`, L on channel 0 and R on channel 1. Compilation and hardware testing against the actual installation are still required.

## Project documents

- [Roadmap](Documentation/ROADMAP.md)
- [Firmware architecture](Documentation/FIRMWARE_ARCHITECTURE.md)
- [Wiring](Electronics/WIRING.md)
- [Changelog](CHANGELOG.md)

## Versioning and validation

The current firmware baseline is 1.0.01. Planned milestones are described in the roadmap; a milestone must not be called complete until its exit criteria have been tested. See CHANGELOG.md.

This is a hobby controller, not certified aviation equipment. Physical detents are created by the mechanism; AS5600 only measures angle.
