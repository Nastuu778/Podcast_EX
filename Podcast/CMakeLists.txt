cmake_minimum_required(VERSION 3.16)

project(Podcast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Podcast main.cpp)

include(GNUInstallDirs)
install(TARGETS Podcast
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
