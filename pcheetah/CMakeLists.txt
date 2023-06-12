cmake_minimum_required(VERSION 3.5)

project(pcheetah VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS rc)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

add_compile_definitions(USE_SYN)

set(PROJECT_SOURCES
        glo.h
        song.h
        pcheetah.h
        ctlNt.h
        pcheetah.cpp
        ctlNt.cpp
        dlgFL.cpp
        dlgCfg.cpp
        dlgTDr.cpp
        dlgCtl.cpp
        dlgChd.cpp
        dlgCue.cpp
        dlgFng.cpp
        dlgMov.cpp
        dlgTpo.cpp
        dlgTSg.cpp
        dlgKSg.cpp
        dlgHlp.cpp
        song.cpp
        sTime.cpp
        sDevTyp.cpp
        sDevice.cpp
        sFile.cpp
        sRecord.cpp
        sNote.cpp
        sChd.cpp
        sEdit.cpp
        sEdMs.cpp
        sUtil.cpp
        sReDo.cpp
        sCmd.cpp
        sDump.cpp
        ../../stv/os.cpp
        ../../stv/ui.cpp
        ../../stv/uiKey.cpp
        ../../stv/timer.h
        ../../stv/midi.h
        ../../stv/midi.cpp
        ../../stv/midiChd.cpp
        ../../stv/syn.h
        ../../stv/syn.cpp
        ../../stv/snd.h
        ../../stv/snd.cpp
        ../../stv/wav.h
        ../../stv/wav.cpp
        rc/dlgcfg.ui
        rc/dlgchd.ui
        rc/dlgctl.ui
        rc/dlgcue.ui
        rc/dlgfl.ui
        rc/dlgfng.ui
        rc/dlghlp.ui
        rc/dlgksg.ui
        rc/dlgmov.ui
        rc/dlgtdr.ui
        rc/dlgtpo.ui
        rc/dlgtsg.ui
        rc/pcheetah.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(pcheetah
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
        add_executable(pcheetah rc/rc.qrc
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(pcheetah PRIVATE Qt${QT_VERSION_MAJOR}::Widgets asound)

set_target_properties(pcheetah PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(pcheetah)
endif()
