set_project("CourierMan")
set_version("1.0.0")
set_license("GPL-3.0-or-later")
set_xmakever("2.9.8")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_languages("c++latest")

option("with_protocol_extras")
set_default(false)
set_showmenu(true)
set_description("Enable optional gRPC, MQTT, Socket.IO, and libcurl protocol packages")
option_end()

if is_plat("windows") then
    set_toolchains("clang-cl")
    add_cxxflags("/std:c++latest", "/permissive-", "/Zc:__cplusplus", { tools = { "clang_cl", "cl" } })
elseif is_plat("linux", "macosx") then
    set_toolchains("clang")
    add_cxxflags("-std=c++2c", "-Wall", "-Wextra", "-Wpedantic", { tools = { "clang", "gcc" } })
end

add_requires("qt6widgets", { system = true })
add_requires("qt6network", { system = true })
add_requires("qt6sql", { system = true })
add_requires("qt6concurrent", { system = true })
add_requires("spdlog", { configs = { header_only = false } })
add_requires("tomlplusplus")
add_requires("nlohmann_json")
add_requires("gtest")

if has_config("with_protocol_extras") then
    add_requires("grpc", "protobuf-cpp", "paho-mqtt-cpp", "curl")
end

local source_files = {
    "src/app/*.cpp",
    "src/core/*.cpp",
    "src/backend/common/*.cpp",
    "src/backend/database/*.cpp",
    "src/network/rest/*.cpp",
    "src/ui/main/*.cpp",
    "src/ui/settings/*.cpp",
    "src/ui/dialogs/*.cpp",
    "src/ui/shared_widgets/*.cpp"
}

local header_files = {
    "src/core/*.h",
    "src/backend/common/*.h",
    "src/backend/database/*.h",
    "src/network/rest/*.h",
    "src/ui/main/*.h",
    "src/ui/settings/*.h",
    "src/ui/dialogs/*.h",
    "src/ui/shared_widgets/*.h"
}

target("CourierMan")
set_kind("binary")
add_rules("qt.widgetapp")
add_files(source_files)
add_files(header_files)
add_files("resources/resources.qrc")
add_includedirs("src", { public = true })
add_packages("qt6widgets", "qt6network", "qt6sql", "qt6concurrent")
add_packages("spdlog", "tomlplusplus", "nlohmann_json")
if has_config("with_protocol_extras") then
    add_packages("grpc", "protobuf-cpp", "paho-mqtt-cpp", "curl")
    add_defines("COURIERMAN_WITH_PROTOCOL_EXTRAS")
end
add_defines("COURIERMAN_VERSION=\"1.0.0\"")
if is_plat("windows") then
    add_links("Advapi32", "Shell32")
end
target_end()

target("courierman_tests")
set_kind("binary")
add_rules("qt.console")
add_files("src/core/*.h")
add_files("src/backend/common/*.h")
add_files("src/backend/database/*.h")
add_files("tests/**/*.cpp")
add_files("src/core/*.cpp")
add_files("src/backend/common/*.cpp")
add_files("src/backend/database/*.cpp")
add_includedirs("src", { public = true })
add_packages("qt6widgets", "qt6network", "qt6sql", "qt6concurrent")
add_packages("spdlog", "tomlplusplus", "nlohmann_json", "gtest")
add_defines("COURIERMAN_TESTING", "COURIERMAN_VERSION=\"1.0.0\"")
target_end()

package("CourierMan")
set_kind("binary")
add_targets("CourierMan")
package_end()
