cmake_minimum_required(VERSION 3.5)

project(pcheetah VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        glo.h
        ctlNt.h
        song.h
        pcheetah.h
        pcheetah.ui
        dlgfl.ui
        dlgcfg.ui
        dlgtdr.ui
        pcheetah.cpp
        ctlNt.cpp
        dlgFL.cpp
        dlgCfg.cpp
        dlgTDr.cpp
        song.cpp
        sTime.cpp
        sDevTyp.cpp
        sDevice.cpp
        sFile.cpp
        sRecord.cpp
        sNote.cpp
        sChrd.cpp
        sEdit.cpp
        sReDo.cpp
        sCmd.cpp
        sDump.cpp
        ../../stv/os.cpp
        ../../stv/ui.cpp
        ../../stv/uiKey.cpp
        ../../stv/timer.h
        ../../stv/midi.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(pcheetah rc/rc.qrc
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET pcheetah APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(pcheetah SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(pcheetah
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(pcheetah PRIVATE Qt${QT_VERSION_MAJOR}::Widgets asound)

set_target_properties(pcheetah PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(pcheetah)
endif()
