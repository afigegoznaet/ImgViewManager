cmake_minimum_required(VERSION 3.21)

project(ivm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Qt 6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
find_package(TBB REQUIRED)
add_executable(ivm
    main.cpp
    FileMoverDelegate.cpp
    FileProgressDialog.cpp
    ImgListView.cpp
    ImgThumbnailDelegate.cpp
    MainWindow.cpp
    SystemTreeView.cpp
    ThumbnailsFileModel.cpp
    ThumbnailsSorter.cpp

    FileMoverDelegate.hpp
    FileProgressDialog.hpp
    ImgListView.hpp
    ImgThumbnailDelegate.hpp
    MainWindow.hpp
    SystemTreeView.hpp
    ThumbnailsFileModel.hpp
    ThumbnailsSorter.hpp

    mainwindow.ui
    progressDialog.ui
    imgviewmanager.qrc
)

target_link_libraries(ivm PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    TBB::tbb
)

target_include_directories(ivm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# qmake: DEFINES += QT_DEPRECATED_WARNINGS
target_compile_definitions(ivm PRIVATE QT_DEPRECATED_WARNINGS)

# qmake: win32 { QT += winextras }
if(WIN32)
    find_package(Qt6 REQUIRED COMPONENTS WinExtras)
    target_link_libraries(ivm PRIVATE Qt6::WinExtras)
endif()

# qmake: win32-* { RC_ICONS = ... }
# Create Resources/ivm.rc containing:
#   IDI_ICON1 ICON "tropical_paradise_icon_meJ_icon.ico"
if(WIN32)
    target_sources(ivm PRIVATE Resources/ivm.rc)
endif()

# qmake: win32-msvc* { /STACK + /MP }
if(MSVC)
    target_link_options(ivm PRIVATE "/STACK:10485760,655360")
    target_compile_options(ivm PRIVATE /MP)
endif()

# qmake: linux { installs + flags + asan in Debug }
if(UNIX AND NOT APPLE)
    #target_compile_options(ivm PRIVATE -fno-exceptions)
    target_compile_options(ivm PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
    target_link_options(ivm PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)

    # user requested style
    set(DESKTOP
        ivm.desktop
    )
    set(ICONS
        Resources/tropical_paradise_icon.png
    )

    install(TARGETS ivm
            RUNTIME DESTINATION /usr/local/bin)

    install(FILES ${DESKTOP}
            DESTINATION /usr/share/applications)

    install(FILES ${ICONS}
            DESTINATION /usr/share/icons/hicolor/16x16/apps)
endif()

