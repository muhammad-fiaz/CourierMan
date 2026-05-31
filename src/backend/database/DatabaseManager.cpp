#include "backend/database/DatabaseManager.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include <utility>

namespace courierman::backend {
namespace {

constexpr int LATEST_SCHEMA_VERSION = 1;

[[nodiscard]] QString queryError(const QSqlQuery& query) {
    return query.lastError().text();
}

}  // namespace

DatabaseManager::DatabaseManager(QString databasePath)
    : m_databasePath(std::move(databasePath))
    , m_connectionName(QStringLiteral("courierman_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces))) {
    QDir().mkpath(QFileInfo(m_databasePath).absolutePath());
}

DatabaseManager::~DatabaseManager() {
    const QString connection = m_connectionName;
    if (m_database.isOpen()) {
        m_database.close();
    }
    m_database = QSqlDatabase{};
    QSqlDatabase::removeDatabase(connection);
}

core::VoidResult DatabaseManager::initialize() {
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(m_databasePath);
    if (!m_database.open()) {
        return std::unexpected(QStringLiteral("Unable to open database: %1")
                                   .arg(m_database.lastError().text()));
    }
    if (auto pragmas = configurePragmas(); !pragmas) {
        return pragmas;
    }
    return applyMigrations();
}

QSqlDatabase DatabaseManager::database() const {
    return m_database;
}

int DatabaseManager::schemaVersion() const {
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("PRAGMA user_version"))) {
        return -1;
    }
    if (!query.next()) {
        return -1;
    }
    return query.value(0).toInt();
}

core::VoidResult DatabaseManager::configurePragmas() {
    if (auto result = execute(QStringLiteral("PRAGMA journal_mode=WAL")); !result) {
        return result;
    }
    if (auto result = execute(QStringLiteral("PRAGMA foreign_keys=ON")); !result) {
        return result;
    }
    if (auto result = execute(QStringLiteral("PRAGMA synchronous=NORMAL")); !result) {
        return result;
    }
    return execute(QStringLiteral("PRAGMA temp_store=MEMORY"));
}

core::VoidResult DatabaseManager::applyMigrations() {
    const int current = schemaVersion();
    if (current < 0) {
        return std::unexpected(QStringLiteral("Unable to read database schema version"));
    }
    for (int version = current + 1; version <= LATEST_SCHEMA_VERSION; ++version) {
        if (!m_database.transaction()) {
            return std::unexpected(QStringLiteral("Unable to begin migration transaction"));
        }
        core::VoidResult result = version == 1
                                      ? migrateToV1()
                                      : std::unexpected(QStringLiteral("Unknown migration version"));
        if (result) {
            result = setSchemaVersion(version);
        }
        if (!result) {
            m_database.rollback();
            return result;
        }
        if (!m_database.commit()) {
            return std::unexpected(QStringLiteral("Unable to commit migration: %1")
                                       .arg(m_database.lastError().text()));
        }
    }
    return {};
}

core::VoidResult DatabaseManager::migrateToV1() {
    const QStringList statements{
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS workspaces (
                id TEXT PRIMARY KEY NOT NULL,
                name TEXT NOT NULL,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS collections (
                id TEXT PRIMARY KEY NOT NULL,
                workspace_id TEXT NOT NULL,
                parent_id TEXT,
                name TEXT NOT NULL,
                description TEXT NOT NULL DEFAULT '',
                sort_order INTEGER NOT NULL DEFAULT 0,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                FOREIGN KEY(workspace_id) REFERENCES workspaces(id) ON DELETE CASCADE
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS requests (
                id TEXT PRIMARY KEY NOT NULL,
                collection_id TEXT,
                name TEXT NOT NULL,
                method TEXT NOT NULL,
                url TEXT NOT NULL,
                auth_type TEXT NOT NULL DEFAULT 'none',
                body_type TEXT NOT NULL DEFAULT 'json',
                body BLOB NOT NULL DEFAULT '',
                pre_request_script TEXT NOT NULL DEFAULT '',
                test_script TEXT NOT NULL DEFAULT '',
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                FOREIGN KEY(collection_id) REFERENCES collections(id) ON DELETE SET NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS request_key_values (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                request_id TEXT NOT NULL,
                bucket TEXT NOT NULL,
                key_name TEXT NOT NULL,
                key_value TEXT NOT NULL DEFAULT '',
                enabled INTEGER NOT NULL DEFAULT 1,
                sort_order INTEGER NOT NULL DEFAULT 0,
                FOREIGN KEY(request_id) REFERENCES requests(id) ON DELETE CASCADE
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS environments (
                id TEXT PRIMARY KEY NOT NULL,
                workspace_id TEXT NOT NULL,
                name TEXT NOT NULL,
                is_active INTEGER NOT NULL DEFAULT 0,
                created_at TEXT NOT NULL,
                updated_at TEXT NOT NULL,
                FOREIGN KEY(workspace_id) REFERENCES workspaces(id) ON DELETE CASCADE
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS environment_variables (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                environment_id TEXT NOT NULL,
                key_name TEXT NOT NULL,
                key_value TEXT NOT NULL DEFAULT '',
                is_secret INTEGER NOT NULL DEFAULT 0,
                enabled INTEGER NOT NULL DEFAULT 1,
                FOREIGN KEY(environment_id) REFERENCES environments(id) ON DELETE CASCADE
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS history (
                id TEXT PRIMARY KEY NOT NULL,
                method TEXT NOT NULL,
                url TEXT NOT NULL,
                status_code INTEGER NOT NULL,
                elapsed_ms INTEGER NOT NULL,
                error TEXT NOT NULL DEFAULT '',
                sent_at TEXT NOT NULL
            )
        )SQL"),
        QStringLiteral(R"SQL(
            CREATE TABLE IF NOT EXISTS feature_flags (
                key_name TEXT PRIMARY KEY NOT NULL,
                enabled INTEGER NOT NULL DEFAULT 0
            )
        )SQL"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_history_sent_at ON history(sent_at DESC)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_requests_collection ON requests(collection_id)")};

    for (const auto& statement : statements) {
        if (auto result = execute(statement); !result) {
            return result;
        }
    }
    return {};
}

core::VoidResult DatabaseManager::execute(const QString& sql) {
    QSqlQuery query(m_database);
    if (!query.exec(sql)) {
        return std::unexpected(queryError(query));
    }
    return {};
}

core::VoidResult DatabaseManager::setSchemaVersion(int version) {
    return execute(QStringLiteral("PRAGMA user_version=%1").arg(version));
}

}  // namespace courierman::backend
