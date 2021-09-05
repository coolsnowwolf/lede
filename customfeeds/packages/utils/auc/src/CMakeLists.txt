cmake_minimum_required(VERSION 2.6...3.12)

PROJECT(auc C)
ADD_DEFINITIONS(-Os -ggdb -Wall --std=gnu99 -Wmissing-declarations)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

find_library(json NAMES json-c json)

ADD_EXECUTABLE(auc auc.c)
TARGET_LINK_LIBRARIES(auc uci ubox ubus uclient blobmsg_json ${json} ${CMAKE_DL_LIBS})
INSTALL(TARGETS auc RUNTIME DESTINATION sbin)
