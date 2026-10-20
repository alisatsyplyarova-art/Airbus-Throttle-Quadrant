# Windows configurator

A first working GUI for firmware 1.0.05. It uses the firmware's newline-delimited Serial protocol; it does not flash firmware.

## Requirements

- Windows 10/11, 64-bit
- Python 3.10 or newer
- Airbus Throttle Quadrant with firmware 1.0.05
- USB data cable

## Run from source

Open PowerShell in the repository folder:

```powershell
py -3 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r Configurator\requirements.txt
python Configurator\main.py
```

If PowerShell blocks virtual-environment activation, run the program using the environment's interpreter directly:

```powershell
.\.venv\Scripts\python.exe -m pip install -r Configurator\requirements.txt
.\.venv\Scripts\python.exe Configurator\main.py
```

## Features

- Lists available COM ports and connects at 115200 baud.
- Displays live AS5600 raw values, HID axis values, and magnet-detection status.
- Starts calibration separately for left (L) and right (R) levers.
- Records MIN and MAX from the current sensor reading. Move the lever to the desired endpoint before pressing each capture button.
- Changes axis inversion and filter percentage.
- Reads configuration, loads defaults into RAM, and explicitly saves to EEPROM.

## Recommended first calibration

1. Connect the device and select its COM port. Close Arduino Serial Monitor first; only one program can normally own the port.
2. Wait for live sensor values and confirm both sensors report a valid signal.
3. For the left lever, click **Начать калибровку**, move it to one mechanical endpoint, then click **Запомнить MIN**. Move it to the other endpoint and click **Запомнить MAX**.
4. Repeat for the right lever.
5. Check that each lever reaches the expected full range and toggle inversion if its direction is reversed.
6. Click **Сохранить в EEPROM** only after checking the result.

The MIN/MAX names refer to the capture sequence, not necessarily to the physical low/high direction of travel. Calibration uses a circular 12-bit raw angle; endpoints must be distinct.

## Safety and current limitations

- `LOAD_DEFAULTS` changes RAM only. EEPROM is modified only by the explicit save command.
- The firmware still emits diagnostic lines on the same Serial connection. The GUI recognizes `STATUS,` and `CONFIG,` records and shows other lines in the log.
- This is an initial source version, not a signed installer or packaged EXE.
- Automated firmware tests and the Pro Micro compile passed in GitHub Actions; the GUI has not yet been validated against physical hardware.
