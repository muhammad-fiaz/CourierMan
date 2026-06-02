set_project("CourierMan")
set_version("1.0.0")
set_license("GPL-3.0-or-later")
set_xmakever("2.9.8")

add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_languages("c++23")

if is_plat("windows") then
    set_toolchains("msvc")
end

option("with_protocol_extras")
    set_default(false)
    set_showmenu(true)
    set_description("Enable optional gRPC, MQTT, Socket.IO, and libcurl protocol packages")
option_end()

add_requires("spdlog", { configs = { header_only = true } })
add_requires("toml++")
add_requires("nlohmann_json")

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

local moc_header_files = {
    "src/network/rest/RestClient.h",
    "src/ui/main/MainWindow.h",
    "src/ui/main/TitleBar.h",
    "src/ui/settings/SettingsView.h",
    "src/ui/dialogs/UpdateDialog.h"
}

local function configure_windows_runtime(target)
    if is_plat("windows") then
        local qt_root = get_config("qt") or os.getenv("QT_ROOT_DIR")
        if qt_root then
            target:add("runenvs", "PATH", path.join(qt_root, "bin"))
        end
        target:add("runenvs", "PATH", "$(targetdir)")
    end
end

local function deploy_windows_qt(target)
    if is_plat("windows") then
        local qt_root = get_config("qt") or os.getenv("QT_ROOT_DIR")
        if qt_root then
            local deploy = path.join(qt_root, "bin", "windeployqt.exe")
            if os.isfile(deploy) then
                os.rm(path.join(target:targetdir(), "Qt6*.dll"))
                os.rm(path.join(target:targetdir(), "platforms"))
                os.rm(path.join(target:targetdir(), "sqldrivers"))
                os.rm(path.join(target:targetdir(), "imageformats"))
                os.rm(path.join(target:targetdir(), "iconengines"))
                os.rm(path.join(target:targetdir(), "styles"))
                os.vrunv(deploy, {"--no-translations", target:targetfile()})
            end
        end
    end
end

target("CourierMan")
    set_kind("binary")
    add_rules("qt.widgetapp")
    add_frameworks("QtNetwork", "QtSql", "QtConcurrent", "QtSvg")

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

    on_load(configure_windows_runtime)
    after_build(deploy_windows_qt)
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
    on_load(configure_windows_runtime)
    after_build(deploy_windows_qt)
target_end()
