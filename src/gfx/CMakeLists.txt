set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(sortvis_gfx STATIC ${CMAKE_CURRENT_SOURCE_DIR}/window.cpp ${CMAKE_CURRENT_SOURCE_DIR}/graphics.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/sort_view.cpp)
add_library(sortvis::gfx ALIAS sortvis_gfx)

target_include_directories(sortvis_gfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
target_link_libraries(sortvis_gfx PUBLIC project::options project::warnings sortvis::log SDL2::SDL2 glad::glad)
