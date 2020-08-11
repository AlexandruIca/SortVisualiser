set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(sortvis_event STATIC ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp)
add_library(sortvis::event ALIAS sortvis_event)

target_include_directories(sortvis_event PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(sortvis_event PUBLIC project::options project::warnings sortvis::log)

add_library(sortvis_event_test STATIC ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp)
add_library(sortvis::event_test ALIAS sortvis_event_test)

target_compile_definitions(sortvis_event_test PUBLIC SORTVIS_TESTING)
target_include_directories(sortvis_event_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(sortvis_event_test PUBLIC project::options project::warnings sortvis::log)
