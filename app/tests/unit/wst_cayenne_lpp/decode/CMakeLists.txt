# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# set(CMAKE_BUILD_TYPE "Debug")

find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(wst)

target_include_directories(testbinary PRIVATE
  ../../../../src/
  ../mocks/
)

FILE(GLOB cayenne_lpp_sources
  ../../../../src/wst_cayenne_lpp.c
)

FILE(GLOB mocks_sources
  ../mocks/assert.c
)

target_sources(testbinary PRIVATE
  ${cayenne_lpp_sources}
  ${mocks_sources}
  src/main.c
)