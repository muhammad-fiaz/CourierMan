#include "core/ConfigManager.h"

#include <toml++/toml.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <fstream>

namespace courierman::core {
namespace {

[[nodiscard]] QString appLocation(QStandardPaths::StandardLocation location) {
    QString path = QStandardPaths::writableLocation(location);
    if (path.isEmpty()) {
        path = QDir::home().filePath(QStringLiteral(".courierman"));
    }
    return QDir::cleanPath(path);
}

[[nodiscard]] QString stringValue(const toml::node_view<const toml::node>& node,
                                  const QString& fallback) {
    if (const auto value = node.value<std::string>()) {
        return QString::fromStdString(*value);
    }
    return fallback;
}

[[nodiscard]] bool boolValue(const toml::node_view<const toml::node>& node, bool fallback) {
    if (const auto value = node.value<bool>()) {
        return *value;
    }
    return fallback;
}

[[nodiscard]] int intValue(const toml::node_view<const toml::node>& node, int fallback) {
    if (const auto value = node.value<int64_t>()) {
        return static_cast<int>(*value);
    }
    return fallback;
}

}  // namespace

ConfigManager::ConfigManager()
    : m_configFilePath(configFilePath()) {}

QString ConfigManager::configDirectory() {
    return appLocation(QStandardPaths::AppConfigLocation);
}

QString ConfigManager::dataDirectory() {
    return appLocation(QStandardPaths::AppDataLocation);
}

QString ConfigManager::logDirectory() {
    return QDir(dataDirectory()).filePath(QStringLiteral("logs"));
}

QString ConfigManager::cacheDirectory() {
    return appLocation(QStandardPaths::CacheLocation);
}

QString ConfigManager::databasePath() {
    return QDir(dataDirectory()).filePath(QStringLiteral("courierman.sqlite3"));
}

QString ConfigManager::configFilePath() {
    return QDir(configDirectory()).filePath(QStringLiteral("settings.toml"));
}

VoidResult ConfigManager::ensureDirectories() const {
    const QStringList directories{configDirectory(), dataDirectory(), logDirectory(), cacheDirectory()};
    for (const auto& directory : directories) {
        if (!QDir().mkpath(directory)) {
            return std::unexpected(QStringLiteral("Unable to create directory: %1").arg(directory));
        }
    }
    return {};
}

Result<AppSettings> ConfigManager::load() const {
    if (auto ensured = ensureDirectories(); !ensured) {
        return std::unexpected(ensured.error());
    }

    AppSettings settings;
    if (!QFileInfo::exists(m_configFilePath)) {
        if (auto saved = save(settings); !saved) {
            return std::unexpected(saved.error());
        }
        return settings;
    }

    try {
        const toml::table table = toml::parse_file(m_configFilePath.toStdString());
        settings.theme = stringValue(table["general"]["theme"], settings.theme);
        settings.activeEnvironmentId =
            stringValue(table["general"]["active_environment_id"], settings.activeEnvironmentId);
        settings.aiProvider = stringValue(table["ai"]["provider"], settings.aiProvider);
        settings.accentColor = stringValue(table["appearance"]["accent_color"], settings.accentColor);
        settings.editorFontFamily =
            stringValue(table["editor"]["font_family"], settings.editorFontFamily);
        settings.proxyUrl = stringValue(table["network"]["proxy_url"], settings.proxyUrl);
        settings.certificatePath =
            stringValue(table["certificates"]["client_certificate"], settings.certificatePath);
        settings.startupMode = stringValue(table["startup"]["mode"], settings.startupMode);
        settings.minimizeToTray = boolValue(table["window"]["minimize_to_tray"], settings.minimizeToTray);
        settings.closeToTray = boolValue(table["window"]["close_to_tray"], settings.closeToTray);
        settings.autoStart = boolValue(table["system"]["auto_start"], settings.autoStart);
        settings.autoCheckUpdates =
            boolValue(table["updates"]["auto_check"], settings.autoCheckUpdates);
        settings.silentUpdates = boolValue(table["updates"]["silent"], settings.silentUpdates);
        settings.verifyDownloads =
            boolValue(table["updates"]["verify_downloads"], settings.verifyDownloads);
        settings.telemetryEnabled =
            boolValue(table["privacy"]["telemetry_enabled"], settings.telemetryEnabled);
        settings.editorFontSize = intValue(table["editor"]["font_size"], settings.editorFontSize);
        settings.networkTimeoutMs = intValue(table["network"]["timeout_ms"], settings.networkTimeoutMs);
        settings.retryCount = intValue(table["network"]["retry_count"], settings.retryCount);
        settings.logRetentionDays =
            intValue(table["logging"]["retention_days"], settings.logRetentionDays);
        settings.maxLogFileMb = intValue(table["logging"]["max_file_mb"], settings.maxLogFileMb);
    } catch (const toml::parse_error& error) {
        const auto description = error.description();
        return std::unexpected(QStringLiteral("TOML parse error in %1: %2")
                                   .arg(m_configFilePath,
                                        QString::fromUtf8(description.data(),
                                                          static_cast<qsizetype>(description.size()))));
    }

    return settings;
}

VoidResult ConfigManager::save(const AppSettings& settings) const {
    if (auto ensured = ensureDirectories(); !ensured) {
        return ensured;
    }

    toml::table table{
        {"general",
         toml::table{{"theme", settings.theme.toStdString()},
                     {"active_environment_id", settings.activeEnvironmentId.toStdString()}}},
        {"appearance", toml::table{{"accent_color", settings.accentColor.toStdString()}}},
        {"editor",
         toml::table{{"font_family", settings.editorFontFamily.toStdString()},
                     {"font_size", settings.editorFontSize}}},
        {"network",
         toml::table{{"timeout_ms", settings.networkTimeoutMs},
                     {"retry_count", settings.retryCount},
                     {"proxy_url", settings.proxyUrl.toStdString()}}},
        {"certificates",
         toml::table{{"client_certificate", settings.certificatePath.toStdString()}}},
        {"window",
         toml::table{{"minimize_to_tray", settings.minimizeToTray},
                     {"close_to_tray", settings.closeToTray}}},
        {"startup", toml::table{{"mode", settings.startupMode.toStdString()}}},
        {"system", toml::table{{"auto_start", settings.autoStart}}},
        {"updates",
         toml::table{{"auto_check", settings.autoCheckUpdates},
                     {"silent", settings.silentUpdates},
                     {"verify_downloads", settings.verifyDownloads}}},
        {"privacy", toml::table{{"telemetry_enabled", settings.telemetryEnabled}}},
        {"ai", toml::table{{"provider", settings.aiProvider.toStdString()}}},
        {"logging",
         toml::table{{"retention_days", settings.logRetentionDays},
                     {"max_file_mb", settings.maxLogFileMb}}},
    };

    std::ofstream output(m_configFilePath.toStdString(), std::ios::trunc);
    if (!output.is_open()) {
        return std::unexpected(QStringLiteral("Unable to write config: %1").arg(m_configFilePath));
    }

    output << table;
    return {};
}

}  // namespace courierman::core
