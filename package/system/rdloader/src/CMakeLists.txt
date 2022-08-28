cmake_minimum_required(VERSION 3.15)
set(CMAKE_C_STANDARD 99)
project(rdloader C)

add_executable(rdloader main.c insmod.c insmod.h mknod.c mknod.h mkdev_node.c mkdev_node.h blkid2.c blkid2.h cmdline.c cmdline.h switch_root.c switch_root.h)

target_link_libraries(${PROJECT_NAME} blkid)

install(TARGETS rdloader RUNTIME DESTINATION /usr/sbin)
