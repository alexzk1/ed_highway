cmake_minimum_required(VERSION 3.24)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ed_highway VERSION 0.2 LANGUAGES CXX)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)
find_package(cereal REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

message(STATUS "Detected Qt major version: ${QT_VERSION_MAJOR}")

file(GLOB BASE_FOLDER
        *.h
        *.hpp
        *.cpp
        *.ui
    )

file(GLOB CONFIG_UI
    config_ui/*.cpp
    config_ui/*.hpp
    config_ui/*.h
    config_ui/*.ui
)

file(GLOB UTILS
    utils/*.cpp
    utils/*.hpp
    utils/*.h
    utils/*.ui
    singleapp/*.cpp
    singleapp/*.hpp
    singleapp/*.h
    singleapp/*.ui
    salesman/*.cpp
    salesman/*.hpp
    salesman/*.h
    salesman/*.ui
)

file(GLOB CUSTOM_WIDGETS
    widgets/*.cpp
    widgets/*.hpp
    widgets/*.h
    widgets/*.ui
)


source_group("Main" FILES ${BASE_FOLDER})
source_group("Utils" FILES ${UTILS})
source_group("Config-Ui" FILES ${CONFIG_UI})
source_group("Widgets" FILES ${CUSTOM_WIDGETS})


if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ed_highway
        MANUAL_FINALIZATION
        ${BASE_FOLDER}
        ${CONFIG_UI}
        ${UTILS}
        ${CUSTOM_WIDGETS}
        resources.qrc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MsiFanControlGUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(ed_highway SHARED
             ${BASE_FOLDER}
             ${CONFIG_UI}
                 ${UTILS}
                 ${CUSTOM_WIDGETS}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ed_highway
             ${BASE_FOLDER}
             ${CONFIG_UI}
             ${UTILS}
             ${CUSTOM_WIDGETS}
        )
    endif()
endif()


target_include_directories(ed_highway PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/config_ui
    ${CMAKE_CURRENT_LIST_DIR}/widgets)

target_link_libraries(ed_highway PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    CURL::libcurl
    ZLIB::ZLIB)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.ed_highway)
endif()
set_target_properties(ed_highway PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ed_highway
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ed_highway)
endif()

