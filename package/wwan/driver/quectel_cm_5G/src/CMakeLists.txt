cmake_minimum_required(VERSION 2.4) 

project(quectel-CM) 
add_definitions(-Wall -Wextra -Werror -O1)
option(USE_QRTR "Enable QRTR" OFF) 

set( QL_CM_SRC
     QmiWwanCM.c GobiNetCM.c main.c MPQMUX.c QMIThread.c util.c qmap_bridge_mode.c mbim-cm.c device.c
     atc.c atchannel.c at_tok.c
     udhcpc.c
     )

if(USE_QRTR) 
add_definitions(-DCONFIG_QRTR)
set( QRTR_SRC qrtr.c rmnetctl.c)
endif() 

add_executable(quectel-CM ${QL_CM_SRC} ${QRTR_SRC})
target_link_libraries(quectel-CM PUBLIC pthread) 
install (TARGETS quectel-CM DESTINATION bin)

add_executable(quectel-qmi-proxy quectel-qmi-proxy.c)
target_link_libraries(quectel-qmi-proxy PUBLIC pthread) 
install (TARGETS quectel-qmi-proxy DESTINATION bin)

add_executable(quectel-mbim-proxy quectel-mbim-proxy.c)
target_link_libraries(quectel-mbim-proxy PUBLIC pthread) 
install (TARGETS quectel-mbim-proxy DESTINATION bin)

add_executable(quectel-atc-proxy quectel-atc-proxy.c atchannel.c at_tok.c util.c)
target_link_libraries(quectel-atc-proxy PUBLIC pthread) 
install (TARGETS quectel-atc-proxy DESTINATION bin)

add_executable(quectel-qrtr-proxy quectel-qrtr-proxy.c)
target_link_libraries(quectel-qrtr-proxy PUBLIC pthread) 
install (TARGETS quectel-qrtr-proxy DESTINATION bin)
