cmake_minimum_required(VERSION 3.1.0)

project(XmlAddressParse)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)


# Find the QtWidgets library
find_package(Qt5Widgets     CONFIG REQUIRED)
find_package(Qt5Gui         CONFIG REQUIRED)
find_package(Qt5Core        CONFIG REQUIRED)
find_package(Qt5XmlPatterns CONFIG REQUIRED)
find_package(Qt5WebEngine      CONFIG REQUIRED)
find_package(Qt5WebEngineWidgets      CONFIG REQUIRED)


# Populate a CMake variable with the sources
set(XmlAddressParse_SRCS
    XmlAddressParse.ui
    XmlAddressParse.h
    XmlAddressParse.cpp
    main.cpp
)

add_executable (${PROJECT_NAME} ${XmlAddressParse_SRCS})

# Use the Widgets module from Qt 5
target_link_libraries(XmlAddressParse Qt5::Gui Qt5::Widgets Qt5::Core Qt5::XmlPatterns Qt5::WebEngine Qt5::WebEngineWidgets)
