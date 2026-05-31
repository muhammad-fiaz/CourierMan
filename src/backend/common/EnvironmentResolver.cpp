#include "backend/common/EnvironmentResolver.h"

#include <QRegularExpression>

namespace courierman::backend {
namespace {

const QRegularExpression VARIABLE_PATTERN(QStringLiteral(R"(\{\{\s*([A-Za-z_][A-Za-z0-9_.-]*)\s*\}\})"));

}  // namespace

QString EnvironmentResolver::resolveText(const QString& input, const Variables& variables) {
    QString output = input;
    QRegularExpressionMatchIterator iterator = VARIABLE_PATTERN.globalMatch(input);
    while (iterator.hasNext()) {
        const auto match = iterator.next();
        const QString key = match.captured(1);
        if (variables.contains(key)) {
            output.replace(match.captured(0), variables.value(key));
        }
    }
    return output;
}

QByteArray EnvironmentResolver::resolveBytes(const QByteArray& input, const Variables& variables) {
    return resolveText(QString::fromUtf8(input), variables).toUtf8();
}

RequestDefinition EnvironmentResolver::resolveRequest(const RequestDefinition& request,
                                                      const Variables& variables) {
    RequestDefinition resolved = request;
    resolved.url = resolveText(request.url, variables);
    resolved.body = resolveBytes(request.body, variables);
    for (auto& header : resolved.headers) {
        header.value = resolveText(header.value, variables);
    }
    for (auto& parameter : resolved.queryParameters) {
        parameter.value = resolveText(parameter.value, variables);
    }
    return resolved;
}

QStringList EnvironmentResolver::missingVariables(const QString& input, const Variables& variables) {
    QStringList missing;
    QRegularExpressionMatchIterator iterator = VARIABLE_PATTERN.globalMatch(input);
    while (iterator.hasNext()) {
        const auto match = iterator.next();
        const QString key = match.captured(1);
        if (!variables.contains(key) && !missing.contains(key)) {
            missing.append(key);
        }
    }
    return missing;
}

}  // namespace courierman::backend
