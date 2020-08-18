set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(sortvis_audio STATIC ${CMAKE_CURRENT_SOURCE_DIR}/audio.cpp)
add_library(sortvis::audio ALIAS sortvis_audio)

target_include_directories(sortvis_audio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(sortvis_audio PUBLIC project::options project::warnings sortvis::log SDL2::SDL2)
