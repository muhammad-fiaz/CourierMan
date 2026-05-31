#pragma once

#include "core/Result.h"

#include <QString>

namespace courierman::core {

/**
 * Stores user-configurable desktop behavior in a TOML file located through
 * QStandardPaths. The class is intentionally value-oriented so it remains easy
 * to test without a running UI.
 */
struct AppSettings {
    QString theme{"system"};
    QString activeEnvironmentId{"local"};
    QString aiProvider{"offline"};
    QString accentColor{"#0f91c8"};
    QString editorFontFamily{"Consolas"};
    QString proxyUrl;
    QString certificatePath;
    QString startupMode{"normal"};
    bool minimizeToTray{true};
    bool closeToTray{false};
    bool autoStart{false};
    bool autoCheckUpdates{true};
    bool silentUpdates{false};
    bool verifyDownloads{true};
    bool telemetryEnabled{false};
    int editorFontSize{13};
    int networkTimeoutMs{30000};
    int retryCount{1};
    int logRetentionDays{7};
    int maxLogFileMb{8};
};

class ConfigManager {
public:
    ConfigManager();

    [[nodiscard]] static QString configDirectory();
    [[nodiscard]] static QString dataDirectory();
    [[nodiscard]] static QString logDirectory();
    [[nodiscard]] static QString cacheDirectory();
    [[nodiscard]] static QString databasePath();
    [[nodiscard]] static QString configFilePath();

    [[nodiscard]] Result<AppSettings> load() const;
    [[nodiscard]] VoidResult save(const AppSettings& settings) const;
    [[nodiscard]] VoidResult ensureDirectories() const;

private:
    QString m_configFilePath;
};

}  // namespace courierman::core
