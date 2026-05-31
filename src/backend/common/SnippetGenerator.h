#pragma once

#include "backend/common/ApiTypes.h"
#include "core/Result.h"

#include <QString>

namespace courierman::backend {

class SnippetGenerator {
public:
    [[nodiscard]] static core::Result<QString> generate(const RequestDefinition& request,
                                                        const QString& language);
};

}  // namespace courierman::backend
