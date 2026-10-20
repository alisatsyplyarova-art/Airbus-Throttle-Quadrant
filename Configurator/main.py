#!/usr/bin/env python3
"""Windows GUI configurator for Airbus Throttle Quadrant firmware 1.0.05."""
from __future__ import annotations

import sys
from datetime import datetime

import serial
from serial.tools import list_ports
from PySide6.QtCore import QTimer, Qt
from PySide6.QtWidgets import (
    QApplication, QCheckBox, QComboBox, QGroupBox, QHBoxLayout, QLabel,
    QMainWindow, QMessageBox, QPushButton, QPlainTextEdit, QProgressBar,
    QSlider, QSpinBox, QVBoxLayout, QWidget
)

APP_VERSION = "0.1.0"
BAUD_RATE = 115200


def parse_fields(line: str, prefix: str) -> dict[str, str] | None:
    """Parse PREFIX,KEY=value,...; ignore diagnostic text from firmware."""
    line = line.strip()
    if not line.startswith(prefix + ","):
        return None
    result: dict[str, str] = {}
    for item in line.split(",")[1:]:
        if "=" in item:
            key, value = item.split("=", 1)
            result[key.strip()] = value.strip()
    return result


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle(f"Airbus Throttle Quadrant — конфигуратор {APP_VERSION}")
        self.resize(900, 760)
        self.port: serial.Serial | None = None
        self.rx_buffer = ""
        self.last_status: dict[str, str] = {}
        self.last_config: dict[str, str] = {}
        self._updating_controls = False

        root = QWidget()
        layout = QVBoxLayout(root)
        self.setCentralWidget(root)

        connection = QGroupBox("Подключение")
        row = QHBoxLayout(connection)
        self.port_combo = QComboBox()
        self.refresh_button = QPushButton("Обновить порты")
        self.connect_button = QPushButton("Подключить")
        self.connection_label = QLabel("Не подключено")
        row.addWidget(QLabel("COM-порт:"))
        row.addWidget(self.port_combo, 1)
        row.addWidget(self.refresh_button)
        row.addWidget(self.connect_button)
        row.addWidget(self.connection_label)
        layout.addWidget(connection)

        live = QGroupBox("Текущие показания")
        live_row = QHBoxLayout(live)
        self.axis_widgets: dict[str, dict[str, QWidget]] = {}
        for axis, caption in (("L", "Левый рычаг (X)"), ("R", "Правый рычаг (Y)")):
            box = QGroupBox(caption)
            v = QVBoxLayout(box)
            raw_label = QLabel("RAW: —")
            hid_label = QLabel("HID: —")
            sensor_label = QLabel("Датчик: нет данных")
            bar = QProgressBar()
            bar.setRange(0, 1023)
            bar.setValue(0)
            bar.setFormat("%v / 1023")
            v.addWidget(raw_label)
            v.addWidget(hid_label)
            v.addWidget(bar)
            v.addWidget(sensor_label)
            live_row.addWidget(box)
            self.axis_widgets[axis] = {
                "raw": raw_label, "hid": hid_label, "sensor": sensor_label, "bar": bar
            }
        layout.addWidget(live)

        calibration = QGroupBox("Калибровка хода рычагов")
        cal_row = QHBoxLayout(calibration)
        for axis, caption in (("L", "Левый"), ("R", "Правый")):
            box = QGroupBox(caption)
            v = QVBoxLayout(box)
            endpoints = QLabel("MIN: —    MAX: —")
            start = QPushButton("Начать калибровку")
            mark_min = QPushButton("Запомнить MIN")
            mark_max = QPushButton("Запомнить MAX")
            for button in (start, mark_min, mark_max):
                v.addWidget(button)
            v.addWidget(endpoints)
            start.clicked.connect(lambda checked=False, a=axis: self.send_command(f"CAL_START {a}"))
            mark_min.clicked.connect(lambda checked=False, a=axis: self.send_command(f"CAL_MARK_MIN {a}"))
            mark_max.clicked.connect(lambda checked=False, a=axis: self.calibrate_max(a))
            self.axis_widgets[axis]["endpoints"] = endpoints
            cal_row.addWidget(box)
        layout.addWidget(calibration)

        settings = QGroupBox("Настройки")
        settings_row = QHBoxLayout(settings)
        self.invert_l = QCheckBox("Инвертировать левый рычаг")
        self.invert_r = QCheckBox("Инвертировать правый рычаг")
        self.filter_slider = QSlider(Qt.Orientation.Horizontal)
        self.filter_slider.setRange(1, 100)
        self.filter_slider.setValue(25)
        self.filter_label = QLabel("25 %")
        settings_row.addWidget(self.invert_l)
        settings_row.addWidget(self.invert_r)
        settings_row.addWidget(QLabel("Фильтр:"))
        settings_row.addWidget(self.filter_slider, 1)
        settings_row.addWidget(self.filter_label)
        layout.addWidget(settings)

        actions = QHBoxLayout()
        self.read_config_button = QPushButton("Прочитать настройки")
        self.defaults_button = QPushButton("Загрузить значения по умолчанию")
        self.save_button = QPushButton("Сохранить в EEPROM")
        actions.addWidget(self.read_config_button)
        actions.addWidget(self.defaults_button)
        actions.addStretch(1)
        actions.addWidget(self.save_button)
        layout.addLayout(actions)

        self.log = QPlainTextEdit()
        self.log.setReadOnly(True)
        self.log.setMaximumBlockCount(500)
        self.log.setPlaceholderText("Журнал обмена с прошивкой")
        layout.addWidget(self.log, 1)

        self.refresh_button.clicked.connect(self.refresh_ports)
        self.connect_button.clicked.connect(self.toggle_connection)
        self.read_config_button.clicked.connect(lambda: self.send_command("GET_CONFIG"))
        self.defaults_button.clicked.connect(self.load_defaults)
        self.save_button.clicked.connect(self.save_config)
        self.invert_l.stateChanged.connect(lambda value: self._set_invert("L", value))
        self.invert_r.stateChanged.connect(lambda value: self._set_invert("R", value))
        self.filter_slider.valueChanged.connect(self._filter_changed)
        self.filter_slider.sliderReleased.connect(self._send_filter)
        self.poll_timer = QTimer(self)
        self.poll_timer.setInterval(30)
        self.poll_timer.timeout.connect(self.poll_serial)
        self.poll_timer.start()
        self.status_timer = QTimer(self)
        self.status_timer.setInterval(1000)
        self.status_timer.timeout.connect(self.request_status)
        self.refresh_ports()

    def log_line(self, message: str) -> None:
        stamp = datetime.now().strftime("%H:%M:%S")
        self.log.appendPlainText(f"[{stamp}] {message}")

    def refresh_ports(self) -> None:
        selected = self.port_combo.currentData()
        self.port_combo.clear()
        ports = list(list_ports.comports())
        for item in ports:
            self.port_combo.addItem(f"{item.device} — {item.description}", item.device)
        if selected:
            index = self.port_combo.findData(selected)
            if index >= 0:
                self.port_combo.setCurrentIndex(index)
        if not ports:
            self.port_combo.addItem("COM-порты не найдены", None)

    def toggle_connection(self) -> None:
        if self.port and self.port.is_open:
            self.disconnect_port()
            return
        device = self.port_combo.currentData()
        if not device:
            QMessageBox.warning(self, "Нет COM-порта", "Подключите контроллер и обновите список портов.")
            return
        try:
            self.port = serial.Serial(device, BAUD_RATE, timeout=0, write_timeout=0.2)
            self.rx_buffer = ""
            self.connect_button.setText("Отключить")
            self.connection_label.setText(f"Подключено: {device}")
            self.log_line(f"Подключено к {device}, {BAUD_RATE} бод.")
            self.status_timer.start(700)
            QTimer.singleShot(1200, self.request_initial_data)
        except (serial.SerialException, OSError) as exc:
            self.port = None
            QMessageBox.critical(self, "Ошибка подключения", str(exc))

    def disconnect_port(self) -> None:
        self.status_timer.stop()
        if self.port:
            try:
                self.port.close()
            except serial.SerialException:
                pass
        self.port = None
        self.connect_button.setText("Подключить")
        self.connection_label.setText("Не подключено")
        self.log_line("Отключено.")

    def request_initial_data(self) -> None:
        self.send_command("GET_CONFIG")
        self.send_command("GET_STATUS")

    def request_status(self) -> None:
        self.send_command("GET_STATUS", quiet=True)

    def send_command(self, command: str, quiet: bool = False) -> None:
        if not self.port or not self.port.is_open:
            if not quiet:
                self.log_line(f"Не отправлено (нет соединения): {command}")
            return
        try:
            self.port.write((command + "\n").encode("ascii"))
            if not quiet:
                self.log_line(f"> {command}")
        except (serial.SerialException, OSError) as exc:
            self.log_line(f"Ошибка отправки: {exc}")
            self.disconnect_port()

    def poll_serial(self) -> None:
        if not self.port or not self.port.is_open:
            return
        try:
            data = self.port.read(512)
        except (serial.SerialException, OSError) as exc:
            self.log_line(f"Ошибка чтения: {exc}")
            self.disconnect_port()
            return
        if not data:
            return
        self.rx_buffer += data.decode("utf-8", errors="replace")
        if len(self.rx_buffer) > 8192:
            self.rx_buffer = self.rx_buffer[-4096:]
        while "\n" in self.rx_buffer:
            line, self.rx_buffer = self.rx_buffer.split("\n", 1)
            self.handle_line(line.rstrip("\r"))

    def handle_line(self, line: str) -> None:
        if not line.strip():
            return
        status = parse_fields(line, "STATUS")
        if status is not None:
            self.last_status = status
            self.update_status(status)
            return
        config = parse_fields(line, "CONFIG")
        if config is not None:
            self.last_config = config
            self.update_config(config)
            self.log_line("< " + line)
            return
        self.log_line("< " + line)

    def update_status(self, data: dict[str, str]) -> None:
        for axis in ("L", "R"):
            widgets = self.axis_widgets[axis]
            raw = data.get(f"{axis}_RAW", "—")
            hid = data.get(f"{axis}_HID", "-1")
            valid = data.get(f"{axis}_VALID", "0") == "1"
            widgets["raw"].setText(f"RAW: {raw}")
            widgets["hid"].setText(f"HID: {hid}")
            try:
                value = max(0, min(1023, int(hid)))
            except ValueError:
                value = 0
            widgets["bar"].setValue(value)
            md = data.get(f"{axis}_MD", "0") == "1"
            ml = data.get(f"{axis}_ML", "0") == "1"
            mh = data.get(f"{axis}_MH", "0") == "1"
            if valid and md:
                detail = "магнит обнаружен"
                if ml:
                    detail += "; слабое поле"
                if mh:
                    detail += "; сильное поле"
            else:
                detail = "нет валидного сигнала"
            widgets["sensor"].setText("Датчик: " + detail)
        self.connection_label.setText(f"Подключено · FW {data.get('FW', '?')}")

    def update_config(self, data: dict[str, str]) -> None:
        self._updating_controls = True
        try:
            self.invert_l.setChecked(data.get("L_INV", "0") == "1")
            self.invert_r.setChecked(data.get("R_INV", "0") == "1")
            value = int(data.get("FILTER", "25"))
            value = max(1, min(100, value))
            self.filter_slider.setValue(value)
            self.filter_label.setText(f"{value} %")
            for axis in ("L", "R"):
                self.axis_widgets[axis]["endpoints"].setText(
                    f"MIN: {data.get(axis + '_MIN', '—')}    MAX: {data.get(axis + '_MAX', '—')}"
                )
        except ValueError:
            self.log_line("Не удалось разобрать конфигурацию контроллера.")
        finally:
            self._updating_controls = False

    def calibrate_max(self, axis: str) -> None:
        self.send_command(f"CAL_MARK_MAX {axis}")
        QTimer.singleShot(200, lambda: self.send_command("GET_CONFIG", quiet=True))

    def _set_invert(self, axis: str, value: int) -> None:
        if self._updating_controls:
            return
        self.send_command(f"SET_INVERT {axis} {1 if value else 0}")

    def _filter_changed(self, value: int) -> None:
        self.filter_label.setText(f"{value} %")

    def _send_filter(self) -> None:
        if not self._updating_controls:
            self.send_command(f"SET_FILTER {self.filter_slider.value()}")

    def load_defaults(self) -> None:
        answer = QMessageBox.question(
            self, "Значения по умолчанию",
            "Загрузить настройки по умолчанию в оперативную память контроллера? "
            "EEPROM не изменится, пока не нажать «Сохранить в EEPROM».",
        )
        if answer == QMessageBox.StandardButton.Yes:
            self.send_command("LOAD_DEFAULTS")
            QTimer.singleShot(200, lambda: self.send_command("GET_CONFIG", quiet=True))

    def save_config(self) -> None:
        answer = QMessageBox.question(
            self, "Сохранение",
            "Записать текущие настройки в EEPROM контроллера?",
        )
        if answer == QMessageBox.StandardButton.Yes:
            self.send_command("SAVE_CONFIG")

    def closeEvent(self, event) -> None:  # noqa: N802 - Qt API name
        self.disconnect_port()
        event.accept()


def main() -> int:
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
