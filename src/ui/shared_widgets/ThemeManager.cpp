#include "ui/shared_widgets/ThemeManager.h"

#include <QFile>

namespace courierman::ui {

QString ThemeManager::styleSheetFor(const core::AppSettings& settings) {
    QFile base(QStringLiteral(":/courierman/styles/courierman.qss"));
    QString style;
    if (base.open(QIODevice::ReadOnly | QIODevice::Text)) {
        style = QString::fromUtf8(base.readAll());
    }

    if (settings.theme == QStringLiteral("dark")) {
        style += QStringLiteral(R"QSS(
            QMainWindow, QDialog, QWidget { background: #111827; color: #e5e7eb; }
            QMenu, QLineEdit, QComboBox, QPlainTextEdit, QTextEdit, QTableWidget {
                background: #1f2937; color: #e5e7eb; border-color: #374151;
            }
            QStatusBar { background: #0f172a; border-top-color: #374151; }
        )QSS");
    }

    if (!settings.accentColor.isEmpty()) {
        style += QStringLiteral("QPushButton#primaryButton { background: %1; border-color: %1; }")
                     .arg(settings.accentColor);
    }
    return style;
}

}  // namespace courierman::ui
