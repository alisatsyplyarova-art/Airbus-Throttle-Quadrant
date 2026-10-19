# Крепление датчиков AS5600 и магнитов / AS5600 Sensor and Magnet Mounting

**Статус:** предварительная механическая концепция; размеры деталей ещё не утверждены.  
**Проект:** Airbus Throttle Quadrant  
**Материал прототипа:** PETG или ABS  
**Контроллер датчиков:** AS5600 × 2 через TCA9548A

## 1. Цель

Разработать компактное и обслуживаемое крепление двух датчиков угла AS5600 для рычагов тяги L/R. Каждый магнит вращается вместе со своей осью, а плата AS5600 остаётся неподвижной относительно корпуса. Конструкция не должна мешать полному ходу рычага или существующим механическим фиксаторам.

## 2. Предлагаемая конструкция

Узел каждого рычага состоит из двух основных деталей:

1. **Держатель магнита на оси.** Печатная втулка или торцевой держатель закрепляется на вращающейся оси/ступице. В держателе предусмотрено посадочное место под фактически имеющийся магнит. Способ фиксации (посадка с натягом, винт, зажим или клей) выбирается после измерения оси и магнита.
2. **Неподвижный регулируемый кронштейн датчика.** Плата AS5600 крепится винтами через её штатные отверстия, если они есть. Кронштейн фиксируется к корпусу и по возможности имеет пазы для регулировки расстояния и центрирования магнита над чувствительным элементом.

Предпочтительно расположить магнит на торце оси и датчик напротив торца, если это позволяет свободное пространство. Если торец недоступен, можно рассмотреть боковое расположение, но его пригодность нужно подтвердить геометрией и требованиями датчика.

## 3. Предварительные параметры для CAD

Это стартовые параметры прототипа, не окончательный чертёж:

- толщина основания кронштейна: около **3 мм**;
- стенки держателя магнита: ориентировочно **3–4 мм**;
- крепёж кронштейна: M3, если размеры корпуса позволяют;
- продолговатые монтажные пазы — для настройки зазора и положения;
- целевой зазор между магнитом и датчиком: начать с **1,5 мм**, затем проверить фактическое значение и показания; ориентир для настройки — диапазон **0,5–3 мм**, если он допустим для конкретного магнита и геометрии;
- магнит должен быть диаметрально намагниченным (полюса на противоположных сторонах цилиндра), подходящим для AS5600;
- ось магнита должна совпадать с центром чувствительной области AS5600. Стремиться к минимальному смещению; допуск держателя будет определён после измерения компонентов и теста показаний.

Окончательный зазор, диаметр и высота посадочного места под магнит, отверстия платы и крепление к корпусу нельзя задавать до получения реальных размеров. Проверять тип магнита следует по маркировке/описанию или подходящим тестом, не полагаться только на внешний вид.

## 4. Что измерить до проектирования

Для каждого отличающегося механического узла:

1. Диаметр оси и её форма: круглая, с плоской гранью, со шлицами, ступицей или иная.
2. Длина доступного выступающего конца оси.
3. Расстояние от торца оси до ближайшей неподвижной стенки/детали.
4. Свободное пространство вокруг оси: радиально и по оси.
5. Полный угол хода рычага и наличие люфта/осевого смещения.
6. Габариты платы AS5600 (ширина × длина × толщина), расположение и диаметр крепёжных отверстий, положение микросхемы относительно краёв платы.
7. Размер магнита (диаметр × высота) и тип намагничивания.
8. Доступные точки крепления кронштейна к корпусу и возможные размеры винтов.
9. Не пересекается ли держатель с соседним рычагом, проводами или механическими ограничителями во всём диапазоне движения.

Для начала достаточно измерить диаметр оси и расстояние от торца оси до ближайшей неподвижной стенки, а также снять размеры платы и магнита, когда они доступны.

## 5. Процесс прототипирования

1. Измерить компоненты и заполнить размеры выше.
2. Выбрать торцевое или боковое расположение магнита по доступному месту.
3. Создать CAD-модель держателя и кронштейна с регулировкой.
4. Напечатать только механические прототипы, без декоративного корпуса.
5. Установить магнит и датчик, вручную проверить весь ход рычага на отсутствие столкновений.
6. Подключить датчик и проверить плавность показаний по всему диапазону; убедиться, что магнит не выпадает и зазор не меняется из-за люфта.
7. Только после проверки зафиксировать окончательные размеры и печатать окончательные детали.

## 6. Материал и печать

- PETG — рекомендуемый материал для первого прототипа.
- ABS также подходит, если печать обеспечивает стабильную геометрию.
- Не закладывать окончательный натяг посадки магнита до пробной печати и измерения фактического магнита.
- Не допускать, чтобы крепёж или металлические детали создавали механическое препятствие. Не считать любую металлическую деталь рядом с магнитом автоматически безопасной для магнитного поля — проверить показания датчика в собранном виде.
- Магнит надёжно удерживать механически; клей можно использовать как дополнительную фиксацию, если он совместим с материалами.

## 7. Что пока НЕ утверждено

- тип, полярность/намагничивание и размеры магнитов;
- диаметр и геометрия осей;
- торцевое или боковое расположение магнитов;
- окончательный зазор;
- размеры отверстий, пазов и крепления к корпусу;
- окончательные STL/STEP-модели.

**Не выдавать предварительные параметры из раздела 3 за готовый чертёж.** Финальная геометрия определяется только после измерений и проверки реального узла.

## 8. Ссылки

- [AS5600 datasheet (ams OSRAM, PDF)](https://look.ams-osram.com/m/7059eac7531a86fd/original/AS5600-DS000365.pdf)
- [README проекта](../README.md)
- [Схема подключения электроники](../Electronics/WIRING.md)

---

# English

**Status:** preliminary mechanical concept; dimensions are not finalized.  
**Prototype material:** PETG or ABS.

## Concept

Each throttle lever gets a rotating magnet holder attached to its shaft/hub and a stationary AS5600 PCB bracket attached to the chassis. Prefer an end-of-shaft arrangement if space allows; consider a side-mounted arrangement only after checking the actual geometry. The sensor board should remain stationary, and the magnet must rotate with the lever.

A modular assembly is recommended:
- a printed magnet holder, sized only after measuring the real shaft and magnet;
- a stationary sensor bracket, using the PCB's actual mounting holes;
- slotted mounting features where practical, allowing adjustment of sensor-to-magnet spacing and alignment.

## Preliminary CAD starting points (not final dimensions)

- sensor bracket base: approximately 3 mm thick;
- magnet-holder walls: approximately 3–4 mm;
- M3 mounting hardware if suitable for the chassis;
- start testing with about 1.5 mm sensor-to-magnet spacing, then verify the actual operating range for the selected magnet and geometry;
- use a diametrically magnetized magnet compatible with AS5600;
- align the magnet center with the sensor's sensitive center as accurately as practical.

Do not finalize the magnet pocket, shaft bore, mounting-hole locations, or air gap until the actual parts have been measured.

## Measurements required

Measure shaft diameter and shape, exposed shaft length, distance to the nearest stationary surface, radial/axial free space, lever travel and play, PCB dimensions and hole pattern, magnet diameter/height and magnetization type, and available chassis mounting points. Check clearance over the full lever travel.

## Prototype workflow

Measure parts → choose end or side mounting → model holder and adjustable bracket → print mechanical prototypes → check full movement and collisions → test sensor readings throughout travel → finalize CAD only after successful checks.

PETG is recommended for the first prototype; ABS is also suitable. Secure the magnet mechanically and verify the assembled sensor readings. The final design and STL/STEP files remain pending real measurements and physical validation.

See the [AS5600 datasheet](https://look.ams-osram.com/m/7059eac7531a86fd/original/AS5600-DS000365.pdf).
