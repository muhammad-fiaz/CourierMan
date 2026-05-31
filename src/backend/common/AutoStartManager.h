#pragma once

#include "core/Result.h"

namespace courierman::backend {

class AutoStartManager {
public:
    [[nodiscard]] static core::VoidResult setEnabled(bool enabled);
    [[nodiscard]] static bool isEnabled();
};

}  // namespace courierman::backend
