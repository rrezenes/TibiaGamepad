cmake_minimum_required(VERSION 3.16)
project(TibiaGamepad VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()

# Configure Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Download and include nlohmann/json
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/ActionExecutor.cpp
    src/GamepadHandler.cpp
    src/InterceptionWrapper.cpp
    src/MainWindow.cpp
    resources.rc
)

# WIN32 creates a Windows subsystem executable (no console window by default)
add_executable(TibiaGamepad WIN32 ${SOURCES})

# Include and link directories for Interception API
target_include_directories(TibiaGamepad PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_directories(TibiaGamepad PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# Link necessary libraries
target_link_libraries(TibiaGamepad PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    nlohmann_json::nlohmann_json
    Xinput
    User32
    Shell32
    interception
)

# Deployment (Qt and Interception DLLs)
if(WIN32)
    # Find windeployqt
    get_target_property(_qt_bin_dir Qt6::Core LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_bin_dir}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET TibiaGamepad POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                    --no-compiler-runtime
                    --no-opengl-sw
                    --no-translations
                    --no-system-d3d-compiler
                    "$<TARGET_FILE:TibiaGamepad>"
            COMMENT "Running windeployqt..."
        )
    endif()

    # Copy interception.dll
    set(INTERCEPTION_DLL "${CMAKE_SOURCE_DIR}/interception_ext/Interception/library/x64/interception.dll")
    if(EXISTS "${INTERCEPTION_DLL}")
        add_custom_command(TARGET TibiaGamepad POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${INTERCEPTION_DLL}"
                "$<TARGET_FILE_DIR:TibiaGamepad>/interception.dll"
            COMMENT "Copying interception.dll to build directory..."
        )
    endif()

    # Copy icon.png for runtime loading
    if(EXISTS "${CMAKE_SOURCE_DIR}/icon.ico")
        add_custom_command(TARGET TibiaGamepad POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_SOURCE_DIR}/icon.ico"
                "$<TARGET_FILE_DIR:TibiaGamepad>/icon.ico"
            COMMENT "Copying icon.ico to build directory..."
        )
    endif()
endif()
