# История изменений / Changelog

## Не выпущено — двуязычная документация (2026-10-19)

- Установлен приоритет русского языка: русская версия документа идёт первой, английская — следом.
- Обновлён README.md; дорожная карта и архитектура прошивки подготовлены к двуязычному ведению.
- Сохранены зафиксированные решения по MSFS, Arduino Pro Micro ATmega32U4, рычагам L/R, датчикам AS5600 через TCA9548A и USB HID (L → X, R → Y).
- Подтверждён выбор Python + PySide6 для планируемого конфигуратора Windows через USB Serial.
- Механизм REVERSE намеренно оставлен открытым: угловая зона, отдельный датчик/выключатель подъёма, кнопка/переключатель или отключённый режим.
- SPEED BRAKE, FLAPS и дополнительные кнопки/переключатели отложены до стабилизации основных рычагов.

**Это обновление документации не означает, что появились новые функции прошивки или что оборудование прошло проверку.**

## Unreleased — bilingual documentation (2026-10-19)

- Set Russian as the primary language: Russian documentation comes first, followed by English.
- Updated README.md and prepared the roadmap and firmware architecture for bilingual maintenance.
- Preserved the fixed decisions for MSFS, Arduino Pro Micro ATmega32U4, L/R levers, AS5600 sensors through TCA9548A, and USB HID mapping (L → X, R → Y).
- Confirmed Python + PySide6 for the planned Windows configurator over USB Serial.
- Kept the REVERSE mechanism intentionally open: angle zone, separate lift sensor/switch, button/switch, or disabled mode.
- Deferred SPEED BRAKE, FLAPS and additional buttons/switches until the main levers are stable.

**This documentation update does not claim new firmware features or hardware validation.**

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
