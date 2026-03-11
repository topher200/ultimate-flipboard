cmake_minimum_required(VERSION 3.20.0)

# "unittest" component: links ztest but skips the full Zephyr kernel.
# Perfect for pure-logic modules with no hardware dependencies.
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(test_scoreboard)

target_sources(app PRIVATE
  src/main.c
  ../../src/scoreboard.c
)

target_include_directories(app PRIVATE ../../include)
