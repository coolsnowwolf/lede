cmake_minimum_required(VERSION 2.8.1)

project(mapcalc C)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=c99")

FIND_PATH(ubus_include_dir libubus.h)
INCLUDE_DIRECTORIES(${ubus_include_dir})

add_definitions(-D_GNU_SOURCE -Wall -Wno-gnu -Wextra)

add_executable(mapcalc mapcalc.c)
target_link_libraries(mapcalc ubus ubox)

install(TARGETS mapcalc DESTINATION sbin/)


# Packaging rules
set(CPACK_PACKAGE_VERSION "1")
set(CPACK_PACKAGE_CONTACT "Steven Barth <steven@midlink.org>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "hnetd")
set(CPACK_GENERATOR "DEB;RPM;STGZ")
set(CPACK_STRIP_FILES true)

SET(CPACK_DEBIAN_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION})
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${CPACK_DEBIAN_PACKAGE_VERSION}")

include(CPack)
