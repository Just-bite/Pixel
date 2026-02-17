#include "mainwindow.h"

#include <QApplication>

QString darkTheme = R"(
        /* ТЕМНАЯ ТЕМА - меняются только цвета, размеры сохраняются */

        /* Основной фон приложения */
        QWidget {
            background-color: #2b2b2b;
            color: #e0e0e0;
            font-family: 'Segoe UI', Arial, sans-serif;
        }

        /* Фон для главного окна */
        QMainWindow {
            background-color: #2b2b2b;
        }

        /* Менюбар */
        QMenuBar {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border-bottom: 1px solid #555555;
        }

        QMenuBar::item:selected {
            background-color: #4a4a4a;
        }

        QMenuBar::item:pressed {
            background-color: #5a5a5a;
        }

        /* Выпадающие меню */
        QMenu {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border: 1px solid #555555;
        }

        QMenu::item:selected {
            background-color: #4a4a4a;
        }

        QMenu::item:disabled {
            color: #777777;
        }

        QMenu::separator {
            height: 1px;
            background-color: #555555;
            margin: 4px 0;
        }

        /* Кнопки */
        QPushButton {
            background-color: #929292;
            color: #000000;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 5px 10px;
        }

        QPushButton:hover {
            background-color: #4a4a4a;
            border-color: #666666;
        }

        QPushButton:pressed {
            background-color: #2a2a2a;
        }

        QPushButton:disabled {
            background-color: #333333;
            color: #777777;
            border-color: #444444;
        }

        QPushButton:default {
            border: 2px solid #888888;
        }

        /* Toolbar кнопки */
        QToolButton {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 4px;
        }

        QToolButton:hover {
            background-color: #4a4a4a;
        }

        QToolButton:pressed, QToolButton:checked {
            background-color: #5a5a5a;
            border-color: #777777;
        }

        /* Группы и фреймы */
        QGroupBox {
            border: 1px solid #555555;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
            color: #e0e0e0;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
            background-color: #2b2b2b;
            color: #e0e0e0;
        }

        /* Фреймы */
        QFrame {
            border: 1px solid #555555;
            background-color: #2b2b2b;
        }

        /* Поля ввода */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border: 1px solid #555555;
            border-radius: 3px;
            padding: 4px;
            selection-background-color: #4a6b8f;
        }

        QLineEdit:focus, QTextEdit:focus, QComboBox:focus {
            border-color: #888888;
        }

        QLineEdit:disabled, QTextEdit:disabled {
            background-color: #333333;
            color: #777777;
        }

        /* Комбобокс выпадающий список */
        QComboBox QAbstractItemView {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border: 1px solid #555555;
        }

        QComboBox::drop-down {
            border: none;
            background-color: #3c3c3c;
        }

        QComboBox::down-arrow {
            image: none;
            background-color: #888888;
            width: 8px;
            height: 8px;
        }

        /* Слайдеры */
        QSlider::groove:horizontal {
            border: 1px solid #555555;
            height: 6px;
            background-color: #3c3c3c;
            border-radius: 3px;
        }

        QSlider::handle:horizontal {
            background-color: #888888;
            border: 1px solid #666666;
            width: 14px;
            height: 14px;
            margin: -5px 0;
            border-radius: 7px;
        }

        QSlider::handle:horizontal:hover {
            background-color: #aaaaaa;
        }

        QSlider::groove:vertical {
            border: 1px solid #555555;
            width: 6px;
            background-color: #3c3c3c;
            border-radius: 3px;
        }

        QSlider::handle:vertical {
            background-color: #888888;
            border: 1px solid #666666;
            height: 14px;
            width: 14px;
            margin: 0 -5px;
            border-radius: 7px;
        }

        /* Скроллбары */
        QScrollBar:horizontal {
            background-color: #3c3c3c;
            height: 14px;
            border: 1px solid #555555;
            border-radius: 7px;
        }

        QScrollBar::handle:horizontal {
            background-color: #666666;
            min-width: 20px;
            border-radius: 6px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: #888888;
        }

        QScrollBar:vertical {
            background-color: #3c3c3c;
            width: 14px;
            border: 1px solid #555555;
            border-radius: 7px;
        }

        QScrollBar::handle:vertical {
            background-color: #666666;
            min-height: 20px;
            border-radius: 6px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #888888;
        }

        QScrollBar::add-line, QScrollBar::sub-line {
            border: none;
            background: none;
        }

        /* Табы */
        QTabWidget::pane {
            border: 1px solid #555555;
            background-color: #2b2b2b;
        }

        QTabBar::tab {
            background-color: #3c3c3c;
            border: 1px solid #555555;
            border-bottom: none;
            padding: 6px 12px;
            margin-right: 2px;
            color: #e0e0e0;
        }

        QTabBar::tab:selected {
            background-color: #4a4a4a;
            border-color: #777777;
        }

        QTabBar::tab:hover:!selected {
            background-color: #4a4a4a;
        }

        /* Списки и деревья */
        QListView, QTreeView, QTableView {
            background-color: #2b2b2b;
            color: #e0e0e0;
            border: 1px solid #555555;
            alternate-background-color: #333333;
        }

        QListView::item:selected, QTreeView::item:selected, QTableView::item:selected {
            background-color: #4a6b8f;
            color: #ffffff;
        }

        QHeaderView::section {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border: 1px solid #555555;
            padding: 4px;
        }

        /* Прогресс бар */
        QProgressBar {
            border: 1px solid #555555;
            border-radius: 3px;
            text-align: center;
            color: #e0e0e0;
            background-color: #3c3c3c;
        }

        QProgressBar::chunk {
            background-color: #4a6b8f;
            border-radius: 2px;
        }

        /* Статус бар */
        QStatusBar {
            background-color: #3c3c3c;
            color: #e0e0e0;
            border-top: 1px solid #555555;
        }

        /* Тулбар */
        QToolBar {
            background-color: #3c3c3c;
            border: none;
            spacing: 3px;
            padding: 2px;
        }

        QToolBar::handle {
            background-color: #666666;
            width: 4px;
            height: 4px;
            border-radius: 2px;
        }

        QToolBar::separator {
            background-color: #555555;
            width: 1px;
            height: 16px;
            margin: 4px 2px;
        }

        /* Сплиттеры */
        QSplitter::handle {
            background-color: #3c3c3c;
        }

        QSplitter::handle:horizontal {
            width: 1px;
        }

        QSplitter::handle:vertical {
            height: 1px;
        }

        QSplitter::handle:hover {
            background-color: #666666;
        }

        /* Лейблы */
        QLabel {
            color: #e0e0e0;
        }

        QLabel:disabled {
            color: #777777;
        }

        /* Чекбоксы и радио кнопки */
        QCheckBox, QRadioButton {
            color: #e0e0e0;
        }

        QCheckBox::indicator, QRadioButton::indicator {
            width: 13px;
            height: 13px;
            border: 1px solid #555555;
            background-color: #3c3c3c;
        }

        QCheckBox::indicator:checked {
            background-color: #4a6b8f;
            border-color: #666666;
        }

        QCheckBox::indicator:hover, QRadioButton::indicator:hover {
            border-color: #888888;
        }

        QCheckBox::indicator:disabled {
            background-color: #333333;
            border-color: #444444;
        }

        /* QGraphicsView */
        QGraphicsView {
            background-color: #1e1e1e;
            border: 1px solid #555555;
        }
    )";

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyleSheet(darkTheme);
    MainWindow w;
    w.setWindowTitle("Pixel");
    w.setWindowIcon(QIcon(":/application_icon/palette.svg"));
    w.show();
    return a.exec();
}
