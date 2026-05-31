#pragma once

#include "core/Result.h"

#include <QSqlDatabase>
#include <QString>

namespace courierman::backend {

/**
 * RAII SQLite manager. All schema setup is centralized here so repositories can
 * stay thin and migrations remain auditable.
 */
class DatabaseManager {
public:
    explicit DatabaseManager(QString databasePath);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&&) = delete;
    DatabaseManager& operator=(DatabaseManager&&) = delete;

    [[nodiscard]] core::VoidResult initialize();
    [[nodiscard]] QSqlDatabase database() const;
    [[nodiscard]] int schemaVersion() const;

private:
    [[nodiscard]] core::VoidResult configurePragmas();
    [[nodiscard]] core::VoidResult applyMigrations();
    [[nodiscard]] core::VoidResult migrateToV1();
    [[nodiscard]] core::VoidResult execute(const QString& sql);
    [[nodiscard]] core::VoidResult setSchemaVersion(int version);

    QString m_databasePath;
    QString m_connectionName;
    QSqlDatabase m_database;
};

}  // namespace courierman::backend
