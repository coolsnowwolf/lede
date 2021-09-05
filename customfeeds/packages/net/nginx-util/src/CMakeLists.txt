cmake_minimum_required(VERSION 2.6)

PROJECT(nginx-util CXX)
SET(CMAKE_CXX_STANDARD 17)

INCLUDE(CheckFunctionExists)

ADD_DEFINITIONS(-Os -Wall -Werror -Wextra -g3)
ADD_DEFINITIONS(-Wno-unused-parameter -Wmissing-declarations -Wshadow)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

FIND_PATH(uci_include_dir uci.h)
FIND_LIBRARY(uci NAMES uci)
INCLUDE_DIRECTORIES(${uci_include_dir})

FIND_PATH(ubox_include_dir libubox/blobmsg.h)
FIND_LIBRARY(ubox NAMES ubox)
INCLUDE_DIRECTORIES(${ubox_include_dir})

IF(UBUS)

ADD_COMPILE_DEFINITIONS(VERSION=${VERSION})

FIND_PATH(ubus_include_dir libubus.h)
FIND_LIBRARY(ubus NAMES ubus)
INCLUDE_DIRECTORIES(${ubus_include_dir})

ADD_EXECUTABLE(nginx-ssl-util nginx-util.cpp)
TARGET_LINK_LIBRARIES(nginx-ssl-util ${uci} ${ubox} ${ubus} pthread ssl crypto pcre)
INSTALL(TARGETS nginx-ssl-util RUNTIME DESTINATION bin)

ADD_EXECUTABLE(nginx-ssl-util-nopcre nginx-util.cpp)
TARGET_COMPILE_DEFINITIONS(nginx-ssl-util-nopcre PUBLIC -DNO_PCRE)
TARGET_LINK_LIBRARIES(nginx-ssl-util-nopcre ${uci} ${ubox} ${ubus} pthread ssl crypto)
INSTALL(TARGETS nginx-ssl-util-nopcre RUNTIME DESTINATION bin)

ELSE()

ADD_COMPILE_DEFINITIONS(VERSION=0)

CONFIGURE_FILE(test-px5g.sh test-px5g.sh COPYONLY)
CONFIGURE_FILE(test-nginx-util.sh test-nginx-util.sh COPYONLY)
CONFIGURE_FILE(test-nginx-util-root.sh test-nginx-util-root.sh COPYONLY)
CONFIGURE_FILE(../files/nginx.config config-nginx-ssl COPYONLY)
CONFIGURE_FILE(../files/uci.conf.template uci.conf.template COPYONLY)

ADD_EXECUTABLE(px5g px5g.cpp)
TARGET_LINK_LIBRARIES(px5g ssl crypto)
INSTALL(TARGETS px5g RUNTIME DESTINATION bin)

ADD_EXECUTABLE(nginx-ssl-util-noubus nginx-util.cpp)
TARGET_COMPILE_DEFINITIONS(nginx-ssl-util-noubus PUBLIC -DNO_UBUS)
TARGET_LINK_LIBRARIES(nginx-ssl-util-noubus ${uci} ${ubox} pthread ssl crypto pcre)
INSTALL(TARGETS nginx-ssl-util-noubus RUNTIME DESTINATION bin)

ADD_EXECUTABLE(nginx-ssl-util-nopcre-noubus nginx-util.cpp)
TARGET_COMPILE_DEFINITIONS(nginx-ssl-util-nopcre-noubus PUBLIC -DNO_PCRE -DNO_UBUS)
TARGET_LINK_LIBRARIES(nginx-ssl-util-nopcre-noubus ${uci} ${ubox} pthread ssl crypto)
INSTALL(TARGETS nginx-ssl-util-nopcre-noubus RUNTIME DESTINATION bin)

ENDIF()
