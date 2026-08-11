#include "styles.h"

QString Styles::applicationStyle()
{
    return R"(
        /* === Окна === */
        QMainWindow, QDialog {
            background-color: #1e1e2b;
        }

        /* === Группы === */
        QGroupBox {
            background-color: #262633;
            border: 1px solid #3a3a4d;
            border-radius: 8px;
            margin-top: 12px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }

        /* === Кнопки === */
        QPushButton {
            background-color: #3a3a4d;
            color: #e0e0e0;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: #4a4a5d;
        }
        QPushButton:pressed {
            background-color: #2f2f40;
        }
        QPushButton:disabled {
            background-color: #2a2a38;
            color: #666666;
        }

        /* === Поля ввода === */
        QLineEdit {
            background-color: #2d2d3d;
            color: #e0e0e0;
            border: 1px solid #3a3a4d;
            border-radius: 6px;
            padding: 6px;
        }
        QLineEdit:focus {
            border: 1px solid #7c4dff;
        }

        /* === Чат === */
        QTextEdit {
            background-color: #23232f;
            color: #e0e0e0;
            border: 1px solid #3a3a4d;
            border-radius: 6px;
        }

        /* === Списки участников === */
        QListWidget {
            background-color: #23232f;
            color: #e0e0e0;
            border: 1px solid #3a3a4d;
            border-radius: 6px;
        }
        QListWidget::item {
            padding: 6px;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background-color: #3a3a5d;
        }

        /* === Ползунок громкости === */
        QSlider::groove:horizontal {
            height: 6px;
            background: #3a3a4d;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            width: 14px;
            margin: -4px 0;
            background: #7c4dff;
            border-radius: 7px;
        }

        /* === Выпадающий список (микрофоны) === */
        QComboBox {
            background-color: #2d2d3d;
            color: #e0e0e0;
            border: 1px solid #3a3a4d;
            border-radius: 6px;
            padding: 4px 8px;
        }

        /* === Радиокнопки и метки === */
        QRadioButton, QLabel {
            color: #e0e0e0;
        }

        /* === Индикатор уровня микрофона === */
        QProgressBar {
            background-color: #2d2d3d;
            border: none;
            border-radius: 4px;
        }
        QProgressBar::chunk {
            background-color: #4CAF50;
            border-radius: 4px;
        }
    )";
}