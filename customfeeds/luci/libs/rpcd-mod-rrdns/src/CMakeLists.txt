cmake_minimum_required(VERSION 2.6)

PROJECT(rpcd-mod-rrdns C)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -g3 -Wmissing-declarations)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

IF(APPLE)
  INCLUDE_DIRECTORIES(/opt/local/include)
  LINK_DIRECTORIES(/opt/local/lib)
ENDIF()

FIND_LIBRARY(resolv NAMES resolv)
IF(resolv STREQUAL "LIBS-NOTFOUND")
  SET(resolv "")
ENDIF()

ADD_LIBRARY(rpcd-mod-rrdns MODULE rrdns.c)
TARGET_LINK_LIBRARIES(rpcd-mod-rrdns ubox ubus ${resolv})
SET_TARGET_PROPERTIES(rpcd-mod-rrdns PROPERTIES OUTPUT_NAME rrdns PREFIX "")

INSTALL(TARGETS rpcd-mod-rrdns LIBRARY DESTINATION lib)
