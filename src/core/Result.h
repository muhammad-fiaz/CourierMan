#pragma once

#include <expected>

#include <QString>

namespace courierman::core {

template <typename T>
using Result = std::expected<T, QString>;

using VoidResult = std::expected<void, QString>;

}  // namespace courierman::core
