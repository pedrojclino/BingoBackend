cmake_minimum_required(VERSION 3.20)
project(BingoBackend LANGUAGES CXX)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Set MSVC specific flags
if(MSVC)
    # Enable all warnings and treat them as errors (optional)
    add_compile_options(/W4 /WX)

    # Ensure Unicode support is enabled
    add_definitions(-DUNICODE -D_UNICODE)
endif()

# Define source and header variables
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h" "src/*.hpp")

# Create the executable target
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Create the library target
set(LIBRARY_NAME BingoPlugin)
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/gametest.cpp")
add_library(${LIBRARY_NAME} SHARED ${SOURCES} ${HEADERS})

# Set build target output directory
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/linux"
    )
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib/linux/${LIBRARY_NAME}"
    )
    # Copy the plugin header
    add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
        COMMENT "Copying public headers to include directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/../lib/linux/${LIBRARY_NAME}/include"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/src/bingoPlugin.h"
            "${CMAKE_BINARY_DIR}/../lib/linux/${LIBRARY_NAME}/include/"
    )
elseif(MSVC)
    # Attempt to set default binary execution arguments
    set(EXEC_ARGS "-s ${CMAKE_SOURCE_DIR}/settings.json -l ${CMAKE_SOURCE_DIR}/logs -t ${CMAKE_SOURCE_DIR}/testing.json")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/win/${PROJECT_NAME}"
        VS_DEBUGGER_COMMAND_ARGUMENTS "${EXEC_ARGS}"
    )
    set_target_properties(${LIBRARY_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../lib/win/${LIBRARY_NAME}"
    )
    # Copy the plugin header
    add_custom_command(TARGET ${LIBRARY_NAME} POST_BUILD
        COMMENT "Copying public headers to include directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/../lib/win/${LIBRARY_NAME}/include"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/src/bingoPlugin.h"
            "${CMAKE_BINARY_DIR}/../lib/win/${LIBRARY_NAME}/include/"
    )
endif()