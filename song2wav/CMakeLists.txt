cmake_minimum_required(VERSION 3.31)
set(QT_FORCE_CMP0156_TO_VALUE NEW)
project(song2wav LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)

add_executable(song2wav
   song2wav.cpp
   stv/os.cpp
   stv/midi.cpp
   stv/wav.cpp
   stv/snd.cpp
   stv/syn.cpp
)

target_include_directories(song2wav PRIVATE
   ${PIPEWIRE_INCLUDE_DIRS})
target_compile_options(    song2wav PRIVATE
   ${PIPEWIRE_CFLAGS_OTHER})
target_link_libraries(     song2wav Qt6::Core
   asound ${PIPEWIRE_LIBRARIES})

install(TARGETS song2wav
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
