# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2022 Eneas Ulir de Queiroz

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
project(uencrypt LANGUAGES C)

option(USE_WOLFSSL "Use WolfSSL as crypto provider" OFF)
if (USE_WOLFSSL)
	add_definitions(-DUSE_WOLFSSL)
	find_library(WOLFSSL_LIBRARY wolfssl REQUIRED)
	set(CRYPTO_LIBRARIES ${WOLFSSL_LIBRARY})
else()
	find_package(OpenSSL REQUIRED)
	set(CRYPTO_LIBRARIES ${OPENSSL_CRYPTO_LIBRARY})
endif()

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c)
target_link_libraries(${PROJECT_NAME} ${CRYPTO_LIBRARIES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
