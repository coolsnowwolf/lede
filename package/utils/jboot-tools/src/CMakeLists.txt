cmake_minimum_required(VERSION 2.6)

PROJECT(jboot-tools C)
ADD_DEFINITIONS(-Wall -Werror --std=gnu99 -Wmissing-declarations)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

ADD_EXECUTABLE(jboot_config_read jboot_config_read.c)
TARGET_LINK_LIBRARIES(jboot_config_read)

INSTALL(TARGETS jboot_config_read RUNTIME DESTINATION bin)
