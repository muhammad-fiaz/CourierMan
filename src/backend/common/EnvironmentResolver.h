#pragma once

#include "backend/common/ApiTypes.h"

#include <QHash>
#include <QString>

namespace courierman::backend {

class EnvironmentResolver {
public:
    using Variables = QHash<QString, QString>;

    [[nodiscard]] static QString resolveText(const QString& input, const Variables& variables);
    [[nodiscard]] static QByteArray resolveBytes(const QByteArray& input, const Variables& variables);
    [[nodiscard]] static RequestDefinition resolveRequest(const RequestDefinition& request,
                                                          const Variables& variables);
    [[nodiscard]] static QStringList missingVariables(const QString& input, const Variables& variables);
};

}  // namespace courierman::backend
