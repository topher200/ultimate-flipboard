cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ultimate_flipboard_ppr)

target_sources(app PRIVATE src/main.c)

# Share the scoreboard_events.h header with the APP core image.
target_include_directories(app PRIVATE ${APPLICATION_SOURCE_DIR}/../include)
