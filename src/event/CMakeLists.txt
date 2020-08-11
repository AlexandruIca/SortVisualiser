set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(sortvis_event STATIC ${CMAKE_CURRENT_SOURCE_DIR}/event.cpp)
add_library(sortvis::event ALIAS sortvis_event)

target_include_directories(sortvis_event PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(sortvis_event PUBLIC project::options project::warnings sortvis::log)
