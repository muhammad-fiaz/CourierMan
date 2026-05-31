#include "backend/common/FeatureCatalog.h"

#include <gtest/gtest.h>

TEST(FeatureCatalogTests, ContainsFreeAndPaidElsewhereCapabilities) {
    const auto features = courierman::backend::FeatureCatalog::allFeatures();
    ASSERT_GT(features.size(), 10);

    bool hasAi = false;
    bool hasEnterpriseStyleFeature = false;
    for (const auto& feature : features) {
        hasAi = hasAi || feature.category == QStringLiteral("AI");
        hasEnterpriseStyleFeature = hasEnterpriseStyleFeature || feature.paidElsewhere;
    }

    EXPECT_TRUE(hasAi);
    EXPECT_TRUE(hasEnterpriseStyleFeature);
}

TEST(FeatureCatalogTests, ExposesExpectedProtocolsAndSnippetLanguages) {
    const auto protocols = courierman::backend::FeatureCatalog::protocolNames();
    const auto languages = courierman::backend::FeatureCatalog::codeGenerationLanguages();

    EXPECT_TRUE(protocols.contains(QStringLiteral("REST")));
    EXPECT_TRUE(protocols.contains(QStringLiteral("gRPC")));
    EXPECT_TRUE(protocols.contains(QStringLiteral("MQTT")));
    EXPECT_TRUE(languages.contains(QStringLiteral("C++")));
    EXPECT_TRUE(languages.contains(QStringLiteral("Rust")));
    EXPECT_TRUE(languages.contains(QStringLiteral("HTTPie")));
}
