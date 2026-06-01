#include "ui/shared_widgets/ThemeManager.h"

#include <QFile>
#include <QGuiApplication>
#include <QPalette>

namespace courierman::ui {
namespace {

struct PaletteTokens {
    QString window;
    QString surface;
    QString elevated;
    QString border;
    QString text;
    QString muted;
    QString subtle;
    QString hover;
    QString pressed;
    QString danger;
};

[[nodiscard]] bool systemPrefersDark() {
    const auto color = QGuiApplication::palette().color(QPalette::Window);
    return color.lightness() < 128;
}

[[nodiscard]] PaletteTokens tokensFor(const QString& theme) {
    if (theme == QStringLiteral("dark") ||
        (theme == QStringLiteral("system") && systemPrefersDark())) {
        return {QStringLiteral("#0f141b"),
                QStringLiteral("#151b23"),
                QStringLiteral("#1d2530"),
                QStringLiteral("#2d3846"),
                QStringLiteral("#e7edf5"),
                QStringLiteral("#9aa8ba"),
                QStringLiteral("#202a36"),
                QStringLiteral("#263343"),
                QStringLiteral("#314154"),
                QStringLiteral("#f87171")};
    }

    return {QStringLiteral("#f5f7fb"),
            QStringLiteral("#ffffff"),
            QStringLiteral("#f8fafc"),
            QStringLiteral("#d8e0ea"),
            QStringLiteral("#172033"),
            QStringLiteral("#607085"),
            QStringLiteral("#edf2f7"),
            QStringLiteral("#eef6fb"),
            QStringLiteral("#dcecf7"),
            QStringLiteral("#b42318")};
}

}  // namespace

QString ThemeManager::styleSheetFor(const core::AppSettings& settings) {
    QFile base(QStringLiteral(":/courierman/styles/courierman.qss"));
    QString style;
    if (base.open(QIODevice::ReadOnly | QIODevice::Text)) {
        style = QString::fromUtf8(base.readAll());
    }

    const auto tokens = tokensFor(settings.theme);
    const QString accent = settings.accentColor.isEmpty() ? QStringLiteral("#0f91c8") : settings.accentColor;

    style += QStringLiteral(R"QSS(
        QWidget {
            background: %1;
            color: %5;
            font-family: "Segoe UI", "Inter", "SF Pro Text", "Ubuntu", sans-serif;
            font-size: 13px;
        }

        QWidget#appRoot,
        QWidget#workspacePage,
        QWidget#centerSurface,
        QMainWindow,
        QDialog {
            background: %1;
        }

        QWidget#titleBar,
        QWidget#leftPanel,
        QWidget#rightPanel,
        QWidget#settingsNav {
            background: %2;
            border-color: %4;
        }

        QWidget#titleBar {
            border-bottom: 1px solid %4;
        }

        QWidget#leftPanel {
            border-right: 1px solid %4;
        }

        QWidget#rightPanel {
            border-left: 1px solid %4;
        }

        QWidget#settingsNav {
            border-right: 1px solid %4;
        }

        QLabel#centerTitleText,
        QLabel#titleText,
        QLabel#sectionTitle,
        QLabel#settingsTitle {
            color: %5;
            background: transparent;
        }

        QLabel#centerTitleText {
            font-size: 14px;
            font-weight: 700;
        }

        QLabel#sectionTitle {
            font-size: 14px;
            font-weight: 700;
        }

        QLabel#settingsTitle {
            font-size: 24px;
            font-weight: 700;
        }

        QLabel#mutedLabel {
            color: %6;
            background: transparent;
        }

        QMenuBar,
        QMenuBar::item {
            background: transparent;
            color: %5;
        }

        QMenuBar::item {
            padding: 6px 9px;
            margin: 0 1px;
        }

        QMenuBar::item:selected {
            background: %8;
            border-radius: 5px;
        }

        QMenu,
        QComboBox QAbstractItemView {
            background: %2;
            border: 1px solid %4;
            color: %5;
        }

        QMenu::item:selected {
            background: %8;
            color: %5;
        }

        QSplitter::handle {
            background: %7;
        }

        QPushButton,
        QToolButton {
            background: %2;
            border: 1px solid %4;
            border-radius: 6px;
            color: %5;
            padding: 7px 12px;
        }

        QPushButton:hover,
        QToolButton:hover {
            background: %8;
            border-color: %10;
        }

        QPushButton:pressed,
        QToolButton:pressed {
            background: %9;
        }

        QPushButton#primaryButton {
            background: %10;
            border-color: %10;
            color: #ffffff;
            font-weight: 600;
        }

        QPushButton#panelToggle {
            min-width: 30px;
            max-width: 36px;
            padding: 6px;
            border: 0;
        }

        QLineEdit,
        QComboBox,
        QSpinBox,
        QPlainTextEdit,
        QTextEdit,
        QTableWidget,
        QTreeWidget,
        QListWidget {
            background: %2;
            border: 1px solid %4;
            border-radius: 6px;
            color: %5;
            selection-background-color: %10;
            selection-color: #ffffff;
        }

        QPlainTextEdit,
        QTextEdit {
            font-family: "%12", "Cascadia Mono", Consolas, monospace;
            font-size: %13px;
        }

        QHeaderView::section {
            background: %3;
            color: %6;
            border: 0;
            border-right: 1px solid %4;
            border-bottom: 1px solid %4;
            padding: 7px;
        }

        QTableWidget::item,
        QTreeWidget::item,
        QListWidget::item {
            padding: 5px 6px;
            border: 0;
        }

        QTableWidget::item:hover,
        QTreeWidget::item:hover,
        QListWidget::item:hover {
            background: %8;
        }

        QTableWidget::item:selected,
        QTreeWidget::item:selected,
        QListWidget::item:selected {
            background: %10;
            color: #ffffff;
        }

        QTableWidget {
            gridline-color: %4;
            alternate-background-color: %7;
        }

        QTableWidget::indicator {
            width: 16px;
            height: 16px;
        }

        QTabWidget::pane {
            border: 1px solid %4;
            border-radius: 7px;
            background: %2;
        }

        QTabBar::tab {
            background: transparent;
            color: %6;
            padding: 9px 12px;
            margin-right: 2px;
        }

        QTabBar::tab:selected {
            color: %10;
            border-bottom: 2px solid %10;
        }

        QStatusBar {
            background: %7;
            border-top: 1px solid %4;
            color: %6;
        }
    )QSS")
                 .arg(tokens.window,
                      tokens.surface,
                      tokens.elevated,
                      tokens.border,
                      tokens.text,
                      tokens.muted,
                      tokens.subtle,
                      tokens.hover,
                      tokens.pressed,
                      accent,
                      tokens.danger,
                      settings.editorFontFamily.isEmpty() ? QStringLiteral("Consolas")
                                                          : settings.editorFontFamily)
                 .arg(settings.editorFontSize);

    return style;
}

}  // namespace courierman::ui
