#pragma once

#include "backend/common/ApiTypes.h"

#include <QList>
#include <QString>

namespace courierman::backend {

struct ScriptTestResult {
    QString name;
    bool passed{false};
    QString message;
};

class ScriptRunner {
public:
    [[nodiscard]] static QList<ScriptTestResult> runPostResponseTests(const QString& script,
                                                                      const ResponseEnvelope& response);
};

}  // namespace courierman::backend
