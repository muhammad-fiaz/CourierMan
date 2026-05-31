#pragma once

#include <QList>
#include <QString>

namespace courierman::backend {

struct FeatureDefinition {
    QString category;
    QString name;
    QString description;
    bool paidElsewhere{false};
    bool implementedInFoundation{false};
};

class FeatureCatalog {
public:
    [[nodiscard]] static QList<FeatureDefinition> allFeatures();
    [[nodiscard]] static QStringList protocolNames();
    [[nodiscard]] static QStringList codeGenerationLanguages();
};

}  // namespace courierman::backend
