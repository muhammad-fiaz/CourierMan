#pragma once

#include <QColor>
#include <QIcon>
#include <QString>

namespace courierman::ui {

class IconFactory {
public:
    [[nodiscard]] static QIcon icon(const QString& name, const QColor& color = QColor(QStringLiteral("#64748b")));
};

}  // namespace courierman::ui
