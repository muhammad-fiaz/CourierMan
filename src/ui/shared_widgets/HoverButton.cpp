#include "ui/shared_widgets/HoverButton.h"

#include <QCursor>

namespace courierman::ui {

HoverButton::HoverButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent) {
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(30);
}

}  // namespace courierman::ui
