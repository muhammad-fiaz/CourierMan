#include "backend/database/DatabaseManager.h"

#include <gtest/gtest.h>

#include <QDir>
#include <QSqlQuery>
#include <QTemporaryDir>

TEST(DatabaseManagerTests, InitializesSchema) {
    QTemporaryDir temp;
    ASSERT_TRUE(temp.isValid());
    const QString path = QDir(temp.path()).filePath(QStringLiteral("test.sqlite3"));
    courierman::backend::DatabaseManager manager(path);

    const auto initialized = manager.initialize();
    ASSERT_TRUE(initialized.has_value()) << initialized.error().toStdString();
    EXPECT_EQ(manager.schemaVersion(), 1);

    QSqlQuery query(manager.database());
    ASSERT_TRUE(query.exec(QStringLiteral("SELECT name FROM sqlite_master WHERE type='table' AND name='requests'")));
    EXPECT_TRUE(query.next());
}
