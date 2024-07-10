# This file is the main configuration for building the mudoedit project using CMake

# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.16)

# Define the project name
project(mudoedit)

# Set C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set minimum required versions for Qt and KDE Frameworks
set(QT_MIN_VERSION "6.4.0")
set(KF_MIN_VERSION "6.0.0")

# Find and include Extra CMake Modules (ECM) package
find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

# Include necessary KDE CMake modules
include(KDEInstallDirs)      # Provides install locations for KDE applications
include(KDECMakeSettings)    # Sets up CMake settings for KDE projects
include(KDECompilerSettings NO_POLICY_SCOPE)  # Sets up compiler settings for KDE projects
include(CPack)               # Includes packaging support

# Find required Qt components
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS 
    Core    # Core Qt functionality
    Widgets # GUI widgets
    Gui     # GUI support
)

# Find required KDE Framework components
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons  # Core add-ons
    I18n        # Internationalization support
    XmlGui      # XML-based GUI description support
    TextWidgets # Text editing widgets
    KIO         # KDE Input/Output library
    ConfigWidgets
    XmlGui
)

# Include the source directory in the include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# Define the source files for the project
set(mudoedit_SRCS
    src/main.cpp
    src/MainWindow.cpp
    src/FileIO.cpp
    src/DocumentManager.cpp
    src/AutoSaveManager.cpp
    src/EditOperations.cpp
    src/WindowManagement.cpp
    src/SettingsManagement.cpp
    src/SyntaxHighlighter.cpp
    src/MenuManager.cpp
    src/ToolbarManager.cpp
    src/ZoomManager.cpp
    src/CustomMdiSubWindow.cpp
)

# Define the header files that need to be processed by Qt's Meta-Object Compiler (MOC)
set(mudoedit_MOC_HEADERS
    src/MainWindow.h
    src/FileIO.h
    src/DocumentManager.h
    src/AutoSaveManager.h
    src/EditOperations.h
    src/WindowManagement.h
    src/SettingsManagement.h
    src/SyntaxHighlighter.h
    src/MenuManager.h
    src/ToolbarManager.h
    src/ZoomManager.h
    src/CustomMdiSubWindow.h
)

# Process the MOC headers
qt6_wrap_cpp(mudoedit_MOC_SRCS ${mudoedit_MOC_HEADERS})

# Add the resource file to the project
set(mudoedit_RESOURCES
    resources.qrc
)

# Process the resource file
qt6_add_resources(mudoedit_RCC_SRCS ${mudoedit_RESOURCES})

# Add the processed resource file to the executable
add_executable(mudoedit ${mudoedit_SRCS} ${mudoedit_MOC_SRCS} ${mudoedit_RCC_SRCS})

# Link the necessary libraries to the executable
target_link_libraries(mudoedit
    Qt::Core
    Qt::Widgets
    Qt::Gui
    KF6::CoreAddons
    KF6::I18n
    KF6::XmlGui
    KF6::TextWidgets
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::ConfigWidgets
)

# Install the executable
install(TARGETS mudoedit ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# Install the UI description file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/mudoeditui.rc
        DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/mudoedit)

# Install the desktop file (for application launchers)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/com.tserath.mudoedit.desktop
        DESTINATION ${KDE_INSTALL_APPDIR})

# Install the SVG icon
install(FILES icons/mudoedit.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps RENAME com.tserath.mudoedit.svg)

# Copy the UI description file to the build directory
configure_file(data/mudoeditui.rc ${CMAKE_CURRENT_BINARY_DIR}/mudoeditui.rc COPYONLY)

# Print a summary of the configuration
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
