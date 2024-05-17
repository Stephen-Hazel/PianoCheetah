cmake_minimum_required(VERSION 3.14)

project(sfz2syn LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(sfz2syn
  main.cpp
  stv/os.cpp
  stv/midi.cpp
  stv/wav.cpp
)
target_link_libraries(sfz2syn Qt${QT_VERSION_MAJOR}::Core asound)

install(TARGETS sfz2syn
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
