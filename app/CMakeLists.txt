cmake_minimum_required(VERSION 3.20)
project(BingoApp LANGUAGES CXX)

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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Find the dlfcn-win32 package (must be installed or available via fetch)
    # If not installed system-wide, you might need to use FetchContent (see below)
    add_subdirectory(deps/dlfcn-win32)
    if(TARGET dl)
        set(CMAKE_DL_LIBS dl)
    elseif(TARGET dlfcn-win32)
        set(CMAKE_DL_LIBS dlfcn-win32)
    else()
        # Fallback: Debug available targets if neither works
        get_property(all_targets GLOBAL PROPERTY ALL_TARGETS)
        message(FATAL_ERROR "dlfcn-win32 target not found. Available targets containing 'dl': ${all_targets}")
    endif()

    # Define the Shared Library (Plugin)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# Define source and header variables
file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "src/*.h")

# Create the executable or library
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add the Plugin Project (use the 'Release' version of the dll)
set(BINGO_PLUGIN_NAME "BingoPlugin")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(BINGO_PLUGIN_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../plugin/lib/linux/BingoPlugin/include")
    set(PLUGIN_SO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../plugin/lib/linux/BingoPlugin/libBingoPlugin.so")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(BINGO_PLUGIN_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../plugin/lib/win/BingoPlugin/include")
    set(PLUGIN_DLL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../plugin/lib/win/BingoPlugin/Release/BingoPlugin.dll")
    set(PLUGIN_DLL_D_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../plugin/lib/win/BingoPlugin/Debug/BingoPlugin.dll")
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${BINGO_PLUGIN_INC_DIR})

# Link the Dynamic Loading Library (only the release version)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_DL_LIBS})

# Set build target output directory
# Attempt to set default binary execution arguments
# Move the release version of the dll to both Release and Debug targets
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(${PROJECT_NAME} ${PLUGIN_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/linux"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMENT "Copying plugin so to the execution directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/../bin/linux"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PLUGIN_SO_FILE}" "${CMAKE_BINARY_DIR}/../bin/linux/"
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set_target_properties(${PROJECT_NAME} ${CMAKE_DL_LIBS} ${PLUGIN_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../bin/win"
        VS_DEBUGGER_COMMAND_ARGUMENTS "-s ${CMAKE_SOURCE_DIR}/settings.json -l ${CMAKE_SOURCE_DIR}/logs -t ${CMAKE_SOURCE_DIR}/testing.json"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMENT "Copying plugin dll to the execution directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/../bin/win/Debug"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PLUGIN_DLL_FILE}" "${CMAKE_BINARY_DIR}/../bin/win/Debug/"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/../bin/win/Release"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PLUGIN_DLL_FILE}" "${CMAKE_BINARY_DIR}/../bin/win/Release/"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/../bin/win/Debug"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PLUGIN_DLL_D_FILE}" "${CMAKE_BINARY_DIR}/../bin/win/Debug/"
    )
endif()