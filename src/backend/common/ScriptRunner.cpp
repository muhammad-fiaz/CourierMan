#include "backend/common/ScriptRunner.h"

#include <QRegularExpression>

namespace courierman::backend {
namespace {

const QRegularExpression STATUS_ASSERTION(
    QStringLiteral(R"(pm\.test\(\s*["']([^"']+)["']\s*,\s*\(\)\s*=>\s*pm\.response\.to\.have\.status\((\d+)\)\s*;?\s*\))"));
const QRegularExpression BODY_CONTAINS_ASSERTION(
    QStringLiteral(R"(pm\.test\(\s*["']([^"']+)["']\s*,\s*\(\)\s*=>\s*pm\.expect\(pm\.response\.text\(\)\)\.to\.include\(["']([^"']+)["']\)\s*;?\s*\))"));

}  // namespace

QList<ScriptTestResult> ScriptRunner::runPostResponseTests(const QString& script,
                                                           const ResponseEnvelope& response) {
    QList<ScriptTestResult> results;

    auto statusIterator = STATUS_ASSERTION.globalMatch(script);
    while (statusIterator.hasNext()) {
        const auto match = statusIterator.next();
        const QString name = match.captured(1);
        const int expected = match.captured(2).toInt();
        const bool passed = response.statusCode == expected;
        results.append({name,
                        passed,
                        passed ? QStringLiteral("Passed")
                               : QStringLiteral("Expected HTTP %1 but got %2").arg(expected).arg(response.statusCode)});
    }

    auto bodyIterator = BODY_CONTAINS_ASSERTION.globalMatch(script);
    while (bodyIterator.hasNext()) {
        const auto match = bodyIterator.next();
        const QString name = match.captured(1);
        const QString needle = match.captured(2);
        const bool passed = QString::fromUtf8(response.body).contains(needle);
        results.append({name,
                        passed,
                        passed ? QStringLiteral("Passed")
                               : QStringLiteral("Response body did not include \"%1\"").arg(needle)});
    }

    if (results.isEmpty() && !script.trimmed().isEmpty()) {
        results.append({QStringLiteral("Script parsed"), false,
                        QStringLiteral("No supported pm.test assertion was found")});
    }

    return results;
}

}  // namespace courierman::backend
