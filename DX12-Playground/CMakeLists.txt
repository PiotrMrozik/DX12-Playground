cmake_minimum_required(VERSION 3.20)
project(DX12Playground LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Make sure DLL and EXE targets go to the same directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Output directory for static lib (.LIB)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for shared lib (.DLL)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Output directory for executables (.EXE)

enable_testing()

add_subdirectory( Engine )
add_subdirectory( Sample )
add_subdirectory( Tests )

# Set the startup project.
set_directory_properties( PROPERTIES
    VS_STARTUP_PROJECT Sample
)

# clang-format target — run: cmake --build . --target format
find_program( CLANG_FORMAT_EXE NAMES clang-format )
if( CLANG_FORMAT_EXE )
    file( GLOB_RECURSE FORMAT_SOURCE_FILES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Engine/Source/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Sample/Include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Sample/Source/*.cpp
    )
    add_custom_target( format
        COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_SOURCE_FILES}
        COMMENT "Running clang-format on all source files..."
        VERBATIM
    )
    set_target_properties( format PROPERTIES FOLDER "Utility" )
else()
    message( STATUS "clang-format not found — 'format' target unavailable" )
endif()