# SPDX-License-Identifier: Apache-2.0
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(wst_sensor_sim)

target_include_directories(app PRIVATE
  ../../../src/
)

FILE(GLOB wst_app_sources
  ../../../src/wst_sensor_thread.c
  ../../../src/wst_sensor_utils.c
  ../../../src/wst_shared.c
)

target_sources(app PRIVATE
  ${wst_app_sources}
  src/main.c
  src/wst_app_thread.c
  src/wst_sensor_config.c
)
