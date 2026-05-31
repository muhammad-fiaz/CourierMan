#pragma once

#include <QPushButton>

namespace courierman::ui {

class HoverButton : public QPushButton {
public:
    explicit HoverButton(const QString& text, QWidget* parent = nullptr);
};

}  // namespace courierman::ui
