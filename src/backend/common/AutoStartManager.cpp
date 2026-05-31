#include "backend/common/AutoStartManager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QSettings>
#include <QTextStream>

namespace courierman::backend {
namespace {

constexpr auto APP_KEY = "CourierMan";

#if defined(Q_OS_LINUX)
[[nodiscard]] QString desktopFilePath() {
    const QString config = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return QDir(config).filePath(QStringLiteral("autostart/CourierMan.desktop"));
}
#endif

#if defined(Q_OS_MACOS)
[[nodiscard]] QString launchAgentPath() {
    const QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return QDir(home).filePath(QStringLiteral("Library/LaunchAgents/com.muhammadfiaz.CourierMan.plist"));
}
#endif

}  // namespace

core::VoidResult AutoStartManager::setEnabled(bool enabled) {
#if defined(Q_OS_WIN)
    QSettings runKey(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     QSettings::NativeFormat);
    if (enabled) {
        runKey.setValue(QString::fromLatin1(APP_KEY),
                        QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        runKey.remove(QString::fromLatin1(APP_KEY));
    }
    return {};
#elif defined(Q_OS_LINUX)
    const QString path = desktopFilePath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return std::unexpected(QStringLiteral("Unable to create autostart directory"));
    }
    if (!enabled) {
        QFile::remove(path);
        return {};
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return std::unexpected(QStringLiteral("Unable to write autostart desktop file"));
    }
    QTextStream stream(&file);
    stream << "[Desktop Entry]\n"
           << "Type=Application\n"
           << "Name=CourierMan\n"
           << "Exec=" << QCoreApplication::applicationFilePath() << "\n"
           << "Terminal=false\n"
           << "X-GNOME-Autostart-enabled=true\n";
    return {};
#elif defined(Q_OS_MACOS)
    const QString path = launchAgentPath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return std::unexpected(QStringLiteral("Unable to create LaunchAgents directory"));
    }
    if (!enabled) {
        QFile::remove(path);
        return {};
    }
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return std::unexpected(QStringLiteral("Unable to write launch agent"));
    }
    QTextStream stream(&file);
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           << "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
              "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
           << "<plist version=\"1.0\"><dict>\n"
           << "<key>Label</key><string>com.muhammadfiaz.CourierMan</string>\n"
           << "<key>ProgramArguments</key><array><string>"
           << QCoreApplication::applicationFilePath()
           << "</string></array>\n"
           << "<key>RunAtLoad</key><true/>\n"
           << "</dict></plist>\n";
    return {};
#else
    Q_UNUSED(enabled)
    return std::unexpected(QStringLiteral("Autostart is not supported on this platform"));
#endif
}

bool AutoStartManager::isEnabled() {
#if defined(Q_OS_WIN)
    QSettings runKey(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     QSettings::NativeFormat);
    return runKey.contains(QString::fromLatin1(APP_KEY));
#elif defined(Q_OS_LINUX)
    return QFileInfo::exists(desktopFilePath());
#elif defined(Q_OS_MACOS)
    return QFileInfo::exists(launchAgentPath());
#else
    return false;
#endif
}

}  // namespace courierman::backend
