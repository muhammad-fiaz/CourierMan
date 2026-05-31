#pragma once

#include <QUrl>

namespace courierman::backend {

struct DiagnosticReport {
    QString summary;
    QString applicationVersion;
    QString operatingSystem;
    QString logPath;
    QString details;
};

class IssueReporter {
public:
    [[nodiscard]] static QUrl buildGitHubIssueUrl(const DiagnosticReport& report);
};

}  // namespace courierman::backend
