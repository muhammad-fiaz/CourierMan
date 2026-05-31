cmake_minimum_required(VERSION 3.28)

project(
    CourierMan
    VERSION 1.0.0
    DESCRIPTION "Native offline-first API testing desktop platform"
    HOMEPAGE_URL "https://muhammad-fiaz.github.io/CourierMan"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(COURIERMAN_BUILD_TESTS "Build CourierMan tests" ON)
option(COURIERMAN_WITH_PROTOCOL_EXTRAS "Enable optional gRPC/MQTT/curl protocol integrations" OFF)

find_package(Qt6 6.10 REQUIRED COMPONENTS Widgets Network Sql Concurrent Test)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(spdlog tomlplusplus nlohmann_json)

set(COURIERMAN_APP_SOURCES
    src/app/main.cpp
    src/core/ConfigManager.cpp
    src/core/Logger.cpp
    src/backend/common/AutoStartManager.cpp
    src/backend/common/FeatureCatalog.cpp
    src/backend/common/SnippetGenerator.cpp
    src/backend/common/EnvironmentResolver.cpp
    src/backend/common/IssueReporter.cpp
    src/backend/common/ScriptRunner.cpp
    src/backend/database/DatabaseManager.cpp
    src/network/rest/RestClient.cpp
    src/ui/dialogs/UpdateDialog.cpp
    src/ui/main/MainWindow.cpp
    src/ui/main/TitleBar.cpp
    src/ui/settings/SettingsView.cpp
    src/ui/shared_widgets/HoverButton.cpp
    src/ui/shared_widgets/ThemeManager.cpp
)

set(COURIERMAN_APP_HEADERS
    src/core/ConfigManager.h
    src/core/Logger.h
    src/core/Result.h
    src/backend/common/ApiTypes.h
    src/backend/common/AutoStartManager.h
    src/backend/common/FeatureCatalog.h
    src/backend/common/SnippetGenerator.h
    src/backend/common/EnvironmentResolver.h
    src/backend/common/IssueReporter.h
    src/backend/common/ScriptRunner.h
    src/backend/database/DatabaseManager.h
    src/network/rest/RestClient.h
    src/ui/dialogs/UpdateDialog.h
    src/ui/main/MainWindow.h
    src/ui/main/TitleBar.h
    src/ui/settings/SettingsView.h
    src/ui/shared_widgets/HoverButton.h
    src/ui/shared_widgets/ThemeManager.h
)

qt_add_executable(CourierMan
    ${COURIERMAN_APP_SOURCES}
    ${COURIERMAN_APP_HEADERS}
    resources/resources.qrc
)

target_include_directories(CourierMan PRIVATE src)
target_compile_definitions(CourierMan PRIVATE COURIERMAN_VERSION="${PROJECT_VERSION}")
if(MSVC)
    target_compile_options(CourierMan PRIVATE /std:c++latest /permissive-)
else()
    target_compile_options(CourierMan PRIVATE -std=c++2b)
endif()
target_link_libraries(CourierMan PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
    Qt6::Concurrent
    spdlog::spdlog
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(CourierMan PRIVATE Advapi32 Shell32)
endif()

if(COURIERMAN_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(courierman_tests
        tests/main.cpp
        tests/core/ConfigManagerTests.cpp
        tests/backend/DatabaseManagerTests.cpp
        tests/backend/FeatureCatalogTests.cpp
        tests/backend/SnippetGeneratorTests.cpp
        tests/backend/EnvironmentResolverTests.cpp
        tests/backend/IssueReporterTests.cpp
        tests/backend/ScriptRunnerTests.cpp
        src/core/ConfigManager.cpp
        src/core/Logger.cpp
        src/backend/common/AutoStartManager.cpp
        src/backend/common/FeatureCatalog.cpp
        src/backend/common/SnippetGenerator.cpp
        src/backend/common/EnvironmentResolver.cpp
        src/backend/common/IssueReporter.cpp
        src/backend/common/ScriptRunner.cpp
        src/backend/database/DatabaseManager.cpp
    )
    target_include_directories(courierman_tests PRIVATE src)
    target_compile_definitions(courierman_tests PRIVATE
        COURIERMAN_TESTING
        COURIERMAN_VERSION="${PROJECT_VERSION}"
    )
    if(MSVC)
        target_compile_options(courierman_tests PRIVATE /std:c++latest /permissive-)
    else()
        target_compile_options(courierman_tests PRIVATE -std=c++2b)
    endif()
    target_link_libraries(courierman_tests PRIVATE
        Qt6::Widgets
        Qt6::Network
        Qt6::Sql
        Qt6::Concurrent
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        nlohmann_json::nlohmann_json
        GTest::gtest
    )
    add_test(NAME courierman_tests COMMAND courierman_tests)
endif()

install(TARGETS CourierMan BUNDLE DESTINATION . RUNTIME DESTINATION bin)
