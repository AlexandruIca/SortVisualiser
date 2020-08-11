set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/log/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/event/)

add_executable(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE project::options project::warnings SDL2::SDL2 glad::glad
                                                    sortvis::log sortvis::event)
