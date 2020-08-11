set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(sortvis_log STATIC ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp)
add_library(sortvis::log ALIAS sortvis_log)

target_compile_definitions(sortvis_log PUBLIC $<$<CONFIG:Debug>:SORTVIS_DEBUG> $<$<CONFIG:Release>:SORTVIS_RELEASE>)
target_link_libraries(sortvis_log PUBLIC spdlog::spdlog)
