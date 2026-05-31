#include "backend/common/IssueReporter.h"

#include <gtest/gtest.h>

#include <QUrlQuery>

TEST(IssueReporterTests, BuildsPrefilledGitHubIssueUrl) {
    const auto url = courierman::backend::IssueReporter::buildGitHubIssueUrl(
        {QStringLiteral("Crash on send"),
         QStringLiteral("1.0.0"),
         QStringLiteral("Windows"),
         QStringLiteral("C:/logs/courierman.log"),
         QStringLiteral("Stack trace goes here")});

    EXPECT_EQ(url.host(), QStringLiteral("github.com"));
    const QUrlQuery query(url);
    EXPECT_EQ(query.queryItemValue(QStringLiteral("template")), QStringLiteral("bug_report.yml"));
    EXPECT_TRUE(query.queryItemValue(QStringLiteral("title")).contains(QStringLiteral("Crash on send")));
    EXPECT_TRUE(query.queryItemValue(QStringLiteral("body")).contains(QStringLiteral("Stack trace goes here")));
}
