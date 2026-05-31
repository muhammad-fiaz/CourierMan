#include "backend/common/IssueReporter.h"

#include <QSysInfo>
#include <QUrlQuery>

namespace courierman::backend {

QUrl IssueReporter::buildGitHubIssueUrl(const DiagnosticReport& report) {
    QUrl url(QStringLiteral("https://github.com/muhammad-fiaz/CourierMan/issues/new"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("template"), QStringLiteral("bug_report.yml"));
    query.addQueryItem(QStringLiteral("title"),
                       QStringLiteral("[Bug]: %1").arg(report.summary.trimmed().isEmpty()
                                                           ? QStringLiteral("CourierMan diagnostic report")
                                                           : report.summary.trimmed()));
    const QString body =
        QStringLiteral("## Diagnostics\n\n"
                       "- Version: %1\n"
                       "- OS: %2\n"
                       "- Logs: %3\n\n"
                       "## Details\n\n%4\n")
            .arg(report.applicationVersion,
                 report.operatingSystem.isEmpty() ? QSysInfo::prettyProductName() : report.operatingSystem,
                 report.logPath,
                 report.details);
    query.addQueryItem(QStringLiteral("body"), body);
    url.setQuery(query);
    return url;
}

}  // namespace courierman::backend
