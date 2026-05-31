#include "backend/common/SnippetGenerator.h"

#include <gtest/gtest.h>

TEST(SnippetGeneratorTests, GeneratesCurlWithHeadersAndBody) {
    courierman::backend::RequestDefinition request;
    request.method = QStringLiteral("POST");
    request.url = QStringLiteral("https://api.example.test/users");
    request.headers.append({QStringLiteral("Authorization"), QStringLiteral("Bearer token"), true});
    request.body = QByteArrayLiteral("{\"name\":\"Ada\"}");

    const auto snippet = courierman::backend::SnippetGenerator::generate(request, QStringLiteral("cURL"));

    ASSERT_TRUE(snippet.has_value()) << snippet.error().toStdString();
    EXPECT_TRUE(snippet->contains(QStringLiteral("curl -X POST")));
    EXPECT_TRUE(snippet->contains(QStringLiteral("Authorization: Bearer token")));
    EXPECT_TRUE(snippet->contains(QStringLiteral("Ada")));
}

TEST(SnippetGeneratorTests, RejectsUnknownLanguage) {
    courierman::backend::RequestDefinition request;
    request.url = QStringLiteral("https://api.example.test");

    const auto snippet = courierman::backend::SnippetGenerator::generate(request, QStringLiteral("BrainScript"));

    EXPECT_FALSE(snippet.has_value());
}
