function(build_test TEST_NAME)
  add_executable(${TEST_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.cpp)
  target_compile_definitions(${TEST_NAME} PUBLIC SORTVIS_TESTING)
  target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src/)
  target_link_libraries(${TEST_NAME} PRIVATE project::options project::warnings doctest::doctest sortvis::event_test
                                             sortvis::log sortvis::algo)
  add_test(${TEST_NAME} ${TEST_NAME})
endfunction()

build_test(event)
build_test(array)
build_test(algorithm)
