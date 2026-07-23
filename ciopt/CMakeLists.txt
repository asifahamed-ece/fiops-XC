# CMakeLists.txt for ciopt - C Implementation of FiOpt

cmake_minimum_required(VERSION 3.10)
project(ciopt VERSION 0.1.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-O2)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/ciopt_core.c
    src/ciopt_report.c
    src/ciopt_analyzer.c
)

# Create library
add_library(ciopt STATIC ${SOURCES})

# Create CLI executable
add_executable(ciopt_cli src/main.c)
target_link_libraries(ciopt_cli ciopt)
set_target_properties(ciopt_cli PROPERTIES OUTPUT_NAME "ciopt")

# Install targets
install(TARGETS ciopt ciopt_cli
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# Enable testing
enable_testing()

# Example test program
add_executable(test_ciopt tests/test_basic.c)
target_link_libraries(test_ciopt ciopt)
add_test(NAME basic_test COMMAND test_ciopt)
