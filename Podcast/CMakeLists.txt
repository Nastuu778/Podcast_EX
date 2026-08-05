cmake_minimum_required(VERSION 3.16)

project(Podcast VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Multimedia)

# === СЕРВЕР ===
add_executable(PodcastServer
    server_main.cpp
    network/tcpserver.cpp
    network/tcpserver.h
)

target_include_directories(PodcastServer PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(PodcastServer PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

# === КЛИЕНТ ===
add_executable(PodcastClient
    client_main.cpp
    network/tcpclient.cpp
    network/tcpclient.h
    ui/roledialog.cpp
    ui/roledialog.h
    ui/mainwindow.cpp
    ui/mainwindow.h
    README.md
)

target_include_directories(PodcastClient PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(PodcastClient PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
)

target_include_directories(PodcastClient PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(PodcastClient PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Multimedia
)