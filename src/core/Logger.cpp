#include "core/Logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfoList>

namespace courierman::core {
namespace {

std::shared_ptr<spdlog::logger> g_logger;

}  // namespace

VoidResult Logger::initialize(const QString& logDirectory, int retentionDays, int maxFileMb) {
    if (!QDir().mkpath(logDirectory)) {
        return std::unexpected(QStringLiteral("Unable to create log directory: %1").arg(logDirectory));
    }

    try {
        if (!g_logger) {
            spdlog::init_thread_pool(8192, 1);
            const auto logFile = QDir(logDirectory).filePath(QStringLiteral("courierman.log"));
            const auto maxBytes = static_cast<std::size_t>(maxFileMb) * 1024U * 1024U;
            g_logger = spdlog::rotating_logger_mt<spdlog::async_factory>(
                "courierman",
                logFile.toStdString(),
                maxBytes,
                5);
            g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
            g_logger->flush_on(spdlog::level::warn);
            spdlog::set_default_logger(g_logger);
        }
        pruneOldLogs(logDirectory, retentionDays);
    } catch (const spdlog::spdlog_ex& error) {
        return std::unexpected(QStringLiteral("Logger initialization failed: %1")
                                   .arg(QString::fromUtf8(error.what())));
    }

    return {};
}

void Logger::shutdown() {
    if (g_logger) {
        g_logger->flush();
        g_logger.reset();
    }
    spdlog::shutdown();
}

std::shared_ptr<spdlog::logger> Logger::get() {
    return g_logger ? g_logger : spdlog::default_logger();
}

void Logger::pruneOldLogs(const QString& logDirectory, int retentionDays) {
    if (retentionDays <= 0) {
        return;
    }

    const auto now = QDateTime::currentDateTimeUtc();
    const QFileInfoList files =
        QDir(logDirectory).entryInfoList(QStringList{QStringLiteral("*.log*")}, QDir::Files);
    for (const auto& file : files) {
        if (file.lastModified().toUTC().daysTo(now) > retentionDays) {
            QFile::remove(file.absoluteFilePath());
        }
    }
}

}  // namespace courierman::core
