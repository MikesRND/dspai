cmake_minimum_required(VERSION 3.24)
project(dspai VERSION 1.0.0 LANGUAGES CXX)

# Options
option(DSPAI_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(DSPAI_ENABLE_SANITIZERS "Enable sanitizers in debug builds" ON)

# Standard install directory variables
include(GNUInstallDirs)

# Testing
include(CTest)

# Add libraries
add_subdirectory(libs/comp)

# Export configuration for find_package support
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/dspaiConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(EXPORT dspaiTargets
        FILE dspaiConfig.cmake
        NAMESPACE dspai::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dspai
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/dspaiConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dspai
    )
endif()
