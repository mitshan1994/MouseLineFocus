cmake_minimum_required(VERSION 3.5)

project(MouseLineFocus VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        AnchorSettings.h
        AnchorSettings.cpp
        GetInputDialog.h
        GetInputDialog.cpp
        GetInputDialog.ui
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        OverlayScheme.h
        OverlayWidget.h
        OverlayWidget.cpp
        OverlayWidget.ui
        SettingKeys.h
        SettingsDialog.h
        SettingsDialog.cpp
        SettingsDialog.ui
        ShortcutDefine.h

        MouseLineFocus.qrc

        mylog/MyLog.cpp
        mylog/MyLog.h

        HotkeyHook/Hotkey.cpp
        HotkeyHook/Hotkey.h
        HotkeyHook/KeyboardHook.cpp
        HotkeyHook/KeyboardHook.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MouseLineFocus
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MouseLineFocus APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MouseLineFocus SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MouseLineFocus
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(MouseLineFocus PRIVATE 
    mylog
)

target_link_libraries(MouseLineFocus PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(MouseLineFocus PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "MouseLineFocus")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MouseLineFocus)
endif()
