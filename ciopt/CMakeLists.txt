cmake_minimum_required(VERSION 3.10)
project(ciopt C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_RELEASE "-O2")

# Source files
set(SOURCES
    src/ciopt_core.c
    src/ciopt_analyzer.c
    src/ciopt_report.c
    src/ciopt_parser.c
)

# Header files
set(HEADERS
    include/ciopt.h
)

# Create library
add_library(ciopt STATIC ${SOURCES} ${HEADERS})
target_include_directories(ciopt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Main executable (example usage)
add_executable(ciopt_cli examples/main.c)
target_link_libraries(ciopt_cli ciopt)

# Install targets
install(TARGETS ciopt DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)
install(TARGETS ciopt_cli DESTINATION bin)

# Testing
enable_testing()
add_executable(test_ciopt tests/test_ciopt.c)
target_link_libraries(test_ciopt ciopt)
add_test(NAME test_ciopt COMMAND test_ciopt)
