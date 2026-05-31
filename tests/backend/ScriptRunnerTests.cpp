#include "backend/common/ScriptRunner.h"

#include <gtest/gtest.h>

TEST(ScriptRunnerTests, EvaluatesStatusAndBodyContainsAssertions) {
    courierman::backend::ResponseEnvelope response;
    response.statusCode = 200;
    response.body = QByteArrayLiteral("{\"status\":\"ok\"}");

    const auto results = courierman::backend::ScriptRunner::runPostResponseTests(
        QStringLiteral(
            "pm.test('status is 200', () => pm.response.to.have.status(200));\n"
            "pm.test('body has ok', () => pm.expect(pm.response.text()).to.include('ok'));"),
        response);

    ASSERT_EQ(results.size(), 2);
    EXPECT_TRUE(results.at(0).passed);
    EXPECT_TRUE(results.at(1).passed);
}

TEST(ScriptRunnerTests, ReportsAssertionFailures) {
    courierman::backend::ResponseEnvelope response;
    response.statusCode = 404;

    const auto results = courierman::backend::ScriptRunner::runPostResponseTests(
        QStringLiteral("pm.test('status is 200', () => pm.response.to.have.status(200));"),
        response);

    ASSERT_EQ(results.size(), 1);
    EXPECT_FALSE(results.first().passed);
    EXPECT_TRUE(results.first().message.contains(QStringLiteral("Expected HTTP 200")));
}
