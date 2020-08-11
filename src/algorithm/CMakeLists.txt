set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(sortvis_algo STATIC ${CMAKE_CURRENT_SOURCE_DIR}/algorithm.cpp ${CMAKE_CURRENT_SOURCE_DIR}/random.cpp)
add_library(sortvis::algo ALIAS sortvis_algo)

target_include_directories(sortvis_algo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(sortvis_algo PUBLIC project::options project::warnings sortvis::event)
