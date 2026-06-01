set_project("CourierMan")
set_version("1.0.0")
set_license("GPL-3.0-or-later")
set_xmakever("2.9.8")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_languages("c++latest")

includes("@builtin/qt")

option("with_protocol_extras")
set_default(false)
set_showmenu(true)
set_description("Enable optional gRPC, MQTT, Socket.IO, and libcurl protocol packages")
option_end()

if is_plat("windows") then
    set_toolchains("msvc")
    add_cxxflags("/std:c++latest", "/permissive-", "/Zc:__cplusplus", { tools = { "cl" } })
else
    add_cxxflags("-std=c++2c", "-Wall", "-Wextra", "-Wpedantic")
end

add_requires("spdlog", { configs = { header_only = true }, system = false })
add_requires("toml++", { system = false })
add_requires("nlohmann_json", { system = false })

if has_config("with_protocol_extras") then
    add_requires("grpc", "protobuf-cpp", "paho-mqtt-cpp", "curl", { system = false })
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

local moc_header_files = {
    "src/network/rest/RestClient.h",
    "src/ui/main/MainWindow.h",
    "src/ui/main/TitleBar.h",
    "src/ui/settings/SettingsView.h",
    "src/ui/dialogs/UpdateDialog.h"
}

target("CourierMan")
set_kind("binary")
add_rules("qt.widgetapp")
add_frameworks("QtNetwork", "QtSql", "QtConcurrent")
add_files(source_files)
add_files(moc_header_files)
add_headerfiles(header_files)
add_files("resources/resources.qrc")
add_includedirs("src", { public = true })
add_packages("spdlog", "toml++", "nlohmann_json")
if has_config("with_protocol_extras") then
    add_packages("grpc", "protobuf-cpp", "paho-mqtt-cpp", "curl")
    add_defines("COURIERMAN_WITH_PROTOCOL_EXTRAS")
end
add_defines("COURIERMAN_VERSION=\"1.0.0\"")
if is_plat("windows") then
    add_links("Advapi32", "Shell32")
end
after_build(function (target)
    if is_plat("windows") then
        local qt_root = get_config("qt") or os.getenv("QT_ROOT_DIR")
        if qt_root then
            local deploy = path.join(qt_root, "bin", "windeployqt.exe")
            if os.isfile(deploy) then
                os.vrunv(deploy, {"--no-translations", "--no-compiler-runtime", target:targetfile()})
            end
        end
    end
end)
target_end()

target("courierman_tests")
set_kind("binary")
add_rules("qt.console")
add_frameworks("QtNetwork", "QtSql", "QtConcurrent")
add_files("tests/main.cpp")
add_files("tests/**/*.cpp")
add_files("src/core/*.cpp")
add_files("src/backend/common/*.cpp")
add_files("src/backend/database/*.cpp")
add_headerfiles("src/core/*.h")
add_headerfiles("src/backend/common/*.h")
add_headerfiles("src/backend/database/*.h")
add_includedirs("tests/support")
add_includedirs("src", { public = true })
add_packages("spdlog", "toml++", "nlohmann_json")
add_defines("COURIERMAN_TESTING", "COURIERMAN_VERSION=\"1.0.0\"")
target_end()
