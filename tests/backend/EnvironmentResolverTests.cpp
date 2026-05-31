#include "backend/common/EnvironmentResolver.h"

#include <gtest/gtest.h>

TEST(EnvironmentResolverTests, ResolvesVariablesInRequest) {
    courierman::backend::RequestDefinition request;
    request.url = QStringLiteral("https://{{ host }}/v1/{{resource}}");
    request.headers.append({QStringLiteral("Authorization"), QStringLiteral("Bearer {{token}}"), true});
    request.body = QByteArrayLiteral("{\"id\":\"{{id}}\"}");

    courierman::backend::EnvironmentResolver::Variables variables;
    variables.insert(QStringLiteral("host"), QStringLiteral("api.example.test"));
    variables.insert(QStringLiteral("resource"), QStringLiteral("users"));
    variables.insert(QStringLiteral("token"), QStringLiteral("secret"));
    variables.insert(QStringLiteral("id"), QStringLiteral("42"));

    const auto resolved = courierman::backend::EnvironmentResolver::resolveRequest(request, variables);

    EXPECT_EQ(resolved.url, QStringLiteral("https://api.example.test/v1/users"));
    EXPECT_EQ(resolved.headers.first().value, QStringLiteral("Bearer secret"));
    EXPECT_EQ(QString::fromUtf8(resolved.body), QStringLiteral("{\"id\":\"42\"}"));
}

TEST(EnvironmentResolverTests, ReportsMissingVariables) {
    courierman::backend::EnvironmentResolver::Variables variables;
    variables.insert(QStringLiteral("host"), QStringLiteral("api.example.test"));

    const auto missing = courierman::backend::EnvironmentResolver::missingVariables(
        QStringLiteral("{{host}}/{{missing}}/{{host}}"),
        variables);

    ASSERT_EQ(missing.size(), 1);
    EXPECT_EQ(missing.first(), QStringLiteral("missing"));
}
