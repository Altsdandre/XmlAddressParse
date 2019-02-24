cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(IcdMonitorInterface)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
    endif ()
else ()
    set (CMAKE_CXX_STANDARD 11)
endif ()

find_package(Qt5 COMPONENTS Core Gui Widgets)

set(HEADERS
    "${PROJECT_SOURCE_DIR}/IcdMonitorInterface.h"
    "${PROJECT_SOURCE_DIR}/ListUpdateThread.h"
)

set(SOURCES
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/IcdMonitorInterface.cpp"
    "${PROJECT_SOURCE_DIR}/ListUpdateThread.cpp"
)

set(FORMS
    "${PROJECT_SOURCE_DIR}/IcdMonitorInterface.ui"
)

if(${CMAKE_VERSION} VERSION_LESS 3.0)
    qt5_wrap_ui(HEADERS ${FORMS})
endif()

add_executable (${PROJECT_NAME} WIN32 ${HEADERS} ${SOURCES} ${FORMS})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)
#target_link_libraries(${PROJECT_NAME} CoordTransformLib)
target_link_libraries(${PROJECT_NAME} IcdLib)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${FOLDER})

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT stc_dev
)

if(WIN32)
    include(QtAdditional)
    QtDeploy(TARGET_EXE ${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}.exe DEFAULT_OPTIONS)
    QtEnvironment()
endif()
