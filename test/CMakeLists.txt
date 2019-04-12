cmake_minimum_required(VERSION 3.6)
project(CSV)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Disable deprecation for windows
if (WIN32)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# CSV executable
file(GLOB CSV_TEST_SOURCES
    "*.cpp"
    "*.hpp"
    "../include/csv/*.hpp"
)
ADD_EXECUTABLE(CSV ${CSV_TEST_SOURCES})
INCLUDE_DIRECTORIES("../include" ".")
set_target_properties(CSV PROPERTIES OUTPUT_NAME tests)
set_property(TARGET CSV PROPERTY CXX_STANDARD 17)

# Set ${PROJECT_NAME} as the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT CSV)