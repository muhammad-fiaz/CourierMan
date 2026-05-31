#pragma once

#include "core/Result.h"

#include <memory>

#include <QString>

namespace spdlog {
class logger;
}

namespace courierman::core {

/**
 * Owns process-wide async logging. Logs are rotated by size and old rotated
 * files are pruned according to the TOML retention setting.
 */
class Logger {
public:
    [[nodiscard]] static VoidResult initialize(const QString& logDirectory,
                                               int retentionDays,
                                               int maxFileMb);
    static void shutdown();
    [[nodiscard]] static std::shared_ptr<spdlog::logger> get();

private:
    static void pruneOldLogs(const QString& logDirectory, int retentionDays);
};

}  // namespace courierman::core
