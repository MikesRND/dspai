# Component library
add_library(dspai_comp INTERFACE)
add_library(dspai::comp ALIAS dspai_comp)

# Set include directories
target_include_directories(dspai_comp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Require C++23
target_compile_features(dspai_comp INTERFACE cxx_std_23)

# Tests (only if testing is enabled)
if(BUILD_TESTING)
    add_executable(dspai_comp_test test/component_test.cpp)
    target_link_libraries(dspai_comp_test PRIVATE dspai::comp)

    # Add warnings if enabled
    if(DSPAI_ENABLE_WARNINGS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(dspai_comp_test PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    # Add sanitizers in debug mode if enabled
    if(DSPAI_ENABLE_SANITIZERS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            set(_dspai_sanitizer_flags -fsanitize=address,undefined)
            target_compile_options(dspai_comp_test PRIVATE
                $<$<CONFIG:Debug>:${_dspai_sanitizer_flags}>
            )
            target_link_options(dspai_comp_test PRIVATE
                $<$<CONFIG:Debug>:${_dspai_sanitizer_flags}>
            )
            unset(_dspai_sanitizer_flags)
        endif()
    endif()

    add_test(NAME dspai::comp::test COMMAND dspai_comp_test)
endif()

# Installation
install(TARGETS dspai_comp
    EXPORT dspaiTargets
)
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)
