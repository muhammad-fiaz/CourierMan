#include "backend/database/DatabaseManager.h"
#include "core/ConfigManager.h"
#include "core/Logger.h"
#include "ui/main/MainWindow.h"
#include "ui/shared_widgets/ThemeManager.h"

#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QThreadPool>

#include <memory>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("Muhammad Fiaz"));
    QApplication::setOrganizationDomain(QStringLiteral("muhammadfiaz.com"));
    QApplication::setApplicationName(QStringLiteral("CourierMan"));
    QApplication::setApplicationDisplayName(QStringLiteral("CourierMan"));
    QApplication::setApplicationVersion(QStringLiteral(COURIERMAN_VERSION));

    QThreadPool::globalInstance()->setMaxThreadCount(qMax(4, QThreadPool::globalInstance()->maxThreadCount()));

    courierman::core::ConfigManager configManager;
    auto settings = configManager.load();
    if (!settings) {
        QMessageBox::critical(nullptr, QStringLiteral("CourierMan Config"), settings.error());
        return 1;
    }

    app.setStyleSheet(courierman::ui::ThemeManager::styleSheetFor(*settings));

    if (auto logger = courierman::core::Logger::initialize(
            courierman::core::ConfigManager::logDirectory(),
            settings->logRetentionDays,
            settings->maxLogFileMb);
        !logger) {
        QMessageBox::warning(nullptr, QStringLiteral("CourierMan Logging"), logger.error());
    }

    courierman::backend::DatabaseManager database(courierman::core::ConfigManager::databasePath());
    if (auto db = database.initialize(); !db) {
        QMessageBox::critical(nullptr, QStringLiteral("CourierMan Database"), db.error());
        courierman::core::Logger::shutdown();
        return 1;
    }

    courierman::ui::MainWindow window(&configManager, *settings);
    window.show();

    const int result = QApplication::exec();
    courierman::core::Logger::shutdown();
    return result;
}
