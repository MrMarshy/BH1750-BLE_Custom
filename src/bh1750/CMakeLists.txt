zephyr_include_directories(.)
target_sources(app PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/bh1750.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bh1750_event_manager.c
)