file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/inputs
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

csv_add_test(tests "src/main.cpp")
