import json
import os
from PyQt6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QLineEdit, QFormLayout, QTabWidget, QTableWidget, QTableWidgetItem,
    QFileDialog, QMessageBox, QSpinBox, QHeaderView
)
from PyQt6.QtCore import Qt

CONFIG_PATH = "config.json"


class ConfigUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("CheapDeck Configurator")
        self.resize(950, 650)

        self.tabs = QTabWidget()
        self.serial_tab = QWidget()
        self.volume_tab = QWidget()
        self.buttons_tab = QWidget()
        self.logging_tab = QWidget()

        self.tabs.addTab(self.serial_tab, "Serial")
        self.tabs.addTab(self.volume_tab, "Volume")
        self.tabs.addTab(self.buttons_tab, "Buttons")
        self.tabs.addTab(self.logging_tab, "Logging")

        layout = QVBoxLayout()
        layout.addWidget(self.tabs)

        btns = QHBoxLayout()
        self.save_btn = QPushButton("💾 Save Config")
        self.reload_btn = QPushButton("↻ Reload")
        btns.addWidget(self.save_btn)
        btns.addWidget(self.reload_btn)
        layout.addLayout(btns)
        self.setLayout(layout)

        # Load config
        self.load_config()
        self.setup_serial_tab()
        self.setup_volume_tab()
        self.setup_buttons_tab()
        self.setup_logging_tab()

        # Connect actions
        self.save_btn.clicked.connect(self.save_config)
        self.reload_btn.clicked.connect(self.load_config)

    # -------------------------------
    # Load configuration
    # -------------------------------
    def load_config(self):
        if not os.path.exists(CONFIG_PATH):
            QMessageBox.warning(self, "Error", "config.json not found!")
            self.config = {}
            return
        with open(CONFIG_PATH, "r") as f:
            self.config = json.load(f)
        print("✓ Config loaded")

    # -------------------------------
    # Serial Tab
    # -------------------------------
    def setup_serial_tab(self):
        form = QFormLayout()
        s = self.config.get("serial", {})
        self.port = QLineEdit(s.get("port", "COM7"))
        self.baud = QSpinBox(); self.baud.setRange(300, 115200); self.baud.setValue(int(s.get("baud_rate", 9600)))
        self.timeout = QLineEdit(str(s.get("timeout", 1.0)))

        form.addRow("Port:", self.port)
        form.addRow("Baud Rate:", self.baud)
        form.addRow("Timeout:", self.timeout)

        test_btn = QPushButton("Test Serial Connection")
        test_btn.clicked.connect(self.test_serial)
        form.addRow(test_btn)
        self.serial_tab.setLayout(form)

    # -------------------------------
    # Volume Tab (Add/Delete)
    # -------------------------------
    def setup_volume_tab(self):
        layout = QVBoxLayout()

        self.volume_table = QTableWidget()
        self.volume_table.setColumnCount(3)
        self.volume_table.setHorizontalHeaderLabels(["Slider ID", "App Name", "Process"])
        self.volume_table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)

        # Add / Delete buttons
        btn_row = QHBoxLayout()
        add_btn = QPushButton("➕ Add Volume App")
        del_btn = QPushButton("🗑️ Delete Selected")
        btn_row.addWidget(add_btn)
        btn_row.addWidget(del_btn)
        layout.addLayout(btn_row)
        layout.addWidget(self.volume_table)

        add_btn.clicked.connect(self.add_volume_row)
        del_btn.clicked.connect(lambda: self.delete_row(self.volume_table))

        self.volume_tab.setLayout(layout)
        self.refresh_volume_table()

    def refresh_volume_table(self):
        apps = self.config.get("volume", {}).get("applications", {})
        self.volume_table.setRowCount(len(apps))
        for i, (key, val) in enumerate(apps.items()):
            self.volume_table.setItem(i, 0, QTableWidgetItem(key))
            self.volume_table.setItem(i, 1, QTableWidgetItem(val.get("name", "")))
            self.volume_table.setItem(i, 2, QTableWidgetItem(val.get("process", "")))

    def add_volume_row(self):
        row = self.volume_table.rowCount()
        self.volume_table.insertRow(row)
        self.volume_table.setItem(row, 0, QTableWidgetItem(f"slider_{row+1}"))
        self.volume_table.setItem(row, 1, QTableWidgetItem("New App"))
        self.volume_table.setItem(row, 2, QTableWidgetItem("process.exe"))

    # -------------------------------
    # Buttons Tab (Add/Delete)
    # -------------------------------
    def setup_buttons_tab(self):
        layout = QVBoxLayout()
        self.buttons_table = QTableWidget()
        self.buttons_table.setColumnCount(4)
        self.buttons_table.setHorizontalHeaderLabels(["Button ID", "Action", "Value", "Description"])
        self.buttons_table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)

        btn_row = QHBoxLayout()
        add_btn = QPushButton("➕ Add Button")
        del_btn = QPushButton("🗑️ Delete Selected")
        btn_row.addWidget(add_btn)
        btn_row.addWidget(del_btn)
        layout.addLayout(btn_row)
        layout.addWidget(self.buttons_table)

        add_btn.clicked.connect(self.add_button_row)
        del_btn.clicked.connect(lambda: self.delete_row(self.buttons_table))

        self.buttons_tab.setLayout(layout)
        self.refresh_buttons_table()

    def refresh_buttons_table(self):
        buttons = self.config.get("buttons", {})
        self.buttons_table.setRowCount(len(buttons))
        for i, (key, val) in enumerate(buttons.items()):
            self.buttons_table.setItem(i, 0, QTableWidgetItem(key))
            self.buttons_table.setItem(i, 1, QTableWidgetItem(val.get("action", "")))
            self.buttons_table.setItem(i, 2, QTableWidgetItem(str(val.get("value", ""))))
            self.buttons_table.setItem(i, 3, QTableWidgetItem(val.get("description", "")))

    def add_button_row(self):
        row = self.buttons_table.rowCount()
        self.buttons_table.insertRow(row)
        self.buttons_table.setItem(row, 0, QTableWidgetItem(f"BTN_{row+1}"))
        self.buttons_table.setItem(row, 1, QTableWidgetItem("key"))
        self.buttons_table.setItem(row, 2, QTableWidgetItem("k"))
        self.buttons_table.setItem(row, 3, QTableWidgetItem("New button"))

    def delete_row(self, table):
        selected = table.currentRow()
        if selected >= 0:
            confirm = QMessageBox.question(self, "Delete Row", "Delete selected row?",
                                           QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if confirm == QMessageBox.StandardButton.Yes:
                table.removeRow(selected)

    # -------------------------------
    # Logging Tab
    # -------------------------------
    def setup_logging_tab(self):
        form = QFormLayout()
        log = self.config.get("logging", {})
        self.log_level = QLineEdit(log.get("level", "INFO"))
        self.log_file = QLineEdit(log.get("file", "cheapdeck.log"))
        form.addRow("Level:", self.log_level)
        form.addRow("Log File:", self.log_file)
        self.logging_tab.setLayout(form)

    # -------------------------------
    # Save Config
    # -------------------------------
    def save_config(self):
        # Serial
        self.config["serial"] = {
            "port": self.port.text(),
            "baud_rate": int(self.baud.value()),
            "timeout": float(self.timeout.text())
        }

        # Volume
        apps = {}
        for i in range(self.volume_table.rowCount()):
            key = self.volume_table.item(i, 0).text()
            if not key:
                continue
            apps[key] = {
                "name": self.volume_table.item(i, 1).text(),
                "process": self.volume_table.item(i, 2).text()
            }
        self.config.setdefault("volume", {})["applications"] = apps

        # Buttons
        btns = {}
        for i in range(self.buttons_table.rowCount()):
            key = self.buttons_table.item(i, 0).text()
            if not key:
                continue
            btns[key] = {
                "action": self.buttons_table.item(i, 1).text(),
                "value": self.buttons_table.item(i, 2).text(),
                "description": self.buttons_table.item(i, 3).text()
            }
        self.config["buttons"] = btns

        # Logging
        self.config["logging"] = {
            "level": self.log_level.text(),
            "file": self.log_file.text()
        }

        # Save
        with open(CONFIG_PATH, "w") as f:
            json.dump(self.config, f, indent=2)
        QMessageBox.information(self, "Saved", "Configuration saved successfully!")

    # -------------------------------
    # Serial Test
    # -------------------------------
    def test_serial(self):
        import serial
        try:
            s = serial.Serial(self.port.text(), int(self.baud.value()), timeout=float(self.timeout.text()))
            s.close()
            QMessageBox.information(self, "Serial", "Connection successful!")
        except Exception as e:
            QMessageBox.critical(self, "Serial Error", str(e))


if __name__ == "__main__":
    app = QApplication([])
    ui = ConfigUI()
    ui.show()
    app.exec()
