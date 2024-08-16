#!/bin/bash

# 检查参数数量
if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <output_image> <boot_partition_image> <rootfs_image>"
    exit 1
fi

OUTPUT_IMAGE=$1
BOOT_PARTITION_IMAGE=$2
ROOTFS_IMAGE=$3


# 下载并准备工具
ver="v0.3.1"
curl -L -o ./AmlImg https://github.com/hzyitc/AmlImg/releases/download/$ver/AmlImg_${ver}_linux_amd64
chmod +x ./AmlImg
curl -L -o ./uboot.img https://github.com/shiyu1314/u-boot-onecloud/releases/download/build-20230901-0443/eMMC.burn.img
./AmlImg unpack ./uboot.img burn/

# 转换镜像格式
img2simg ${BOOT_PARTITION_IMAGE} burn/boot.simg
img2simg ${ROOTFS_IMAGE} burn/rootfs.simg

# 创建命令文件
cat <<EOF >>burn/commands.txt
PARTITION:boot:sparse:boot.simg
PARTITION:rootfs:sparse:rootfs.simg
EOF

# 打包生成最终镜像
./AmlImg pack ${OUTPUT_IMAGE} burn/

# 清理临时文件夹
rm -rf burn
