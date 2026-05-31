#include "core/ConfigManager.h"

#include <gtest/gtest.h>

TEST(ConfigManagerTests, ProvidesOsNativeLocations) {
    const auto config = courierman::core::ConfigManager::configDirectory();
    const auto data = courierman::core::ConfigManager::dataDirectory();
    const auto database = courierman::core::ConfigManager::databasePath();

    EXPECT_FALSE(config.isEmpty());
    EXPECT_FALSE(data.isEmpty());
    EXPECT_TRUE(database.endsWith(QStringLiteral("courierman.sqlite3")));
}

TEST(ConfigManagerTests, SavesAndLoadsTomlSettings) {
    courierman::core::ConfigManager manager;
    courierman::core::AppSettings settings;
    settings.theme = QStringLiteral("dark");
    settings.aiProvider = QStringLiteral("ollama");
    settings.accentColor = QStringLiteral("#123456");
    settings.networkTimeoutMs = 17000;
    settings.retryCount = 3;
    settings.logRetentionDays = 11;

    ASSERT_TRUE(manager.save(settings).has_value());
    const auto loaded = manager.load();
    ASSERT_TRUE(loaded.has_value()) << loaded.error().toStdString();
    EXPECT_EQ(loaded->theme, QStringLiteral("dark"));
    EXPECT_EQ(loaded->aiProvider, QStringLiteral("ollama"));
    EXPECT_EQ(loaded->accentColor, QStringLiteral("#123456"));
    EXPECT_EQ(loaded->networkTimeoutMs, 17000);
    EXPECT_EQ(loaded->retryCount, 3);
    EXPECT_EQ(loaded->logRetentionDays, 11);
}
