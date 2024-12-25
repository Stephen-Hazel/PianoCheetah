cmake_minimum_required(VERSION 3.31)

project(song2wav LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(song2wav
  song2wav.cpp
  stv/os.cpp
  stv/midi.cpp
  stv/wav.cpp
  stv/snd.cpp
  stv/syn.cpp
)
target_link_libraries(song2wav Qt${QT_VERSION_MAJOR}::Core asound)

install(TARGETS song2wav
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
