#
# This file is part of Weather Station project <https://github.com/VeniaminGH/Weather-Station>.
# Copyright (c) 2024 Veniamin Milevski
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed WITHOUT ANY WARRANTY. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(wst_sensor_sim)

target_include_directories(app PRIVATE
  ../../../src/
  ../../../include/
)

FILE(GLOB wst_app_sources
  ../../../src/wst_sensor_thread.c
  ../../../src/wst_sensor_utils.c
  ../../../src/wst_sensor_config.c
  ../../../src/wst_shared.c
)

target_sources(app PRIVATE
  ${wst_app_sources}
  src/main.c
  src/wst_app_thread.c
)
