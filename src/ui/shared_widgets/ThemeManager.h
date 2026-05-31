#pragma once

#include "core/ConfigManager.h"

#include <QString>

namespace courierman::ui {

class ThemeManager {
public:
    [[nodiscard]] static QString styleSheetFor(const core::AppSettings& settings);
};

}  // namespace courierman::ui
