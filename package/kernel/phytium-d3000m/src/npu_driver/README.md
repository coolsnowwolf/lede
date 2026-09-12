# Phytium D3000M NPU driver

This directory contains the Phytium Linux 6.6 NPU sources ported for the
6.18.18.c1080-trim kernel. The platform driver binds ACPI `PHYT0050` and the
PCI driver binds `1db7:dc24`.

Build the runtime drivers with:

```sh
make KERNEL_SRC=/path/to/kernel ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j30
```

The optional `heap/phytium_npu_heap.c` exporter is enabled with
`BUILD_HEAP=y`. It requires a target kernel built with
`CONFIG_DMABUF_HEAPS=y` and the DMA-BUF heap symbols exported; the current
fnOS 6.18 test kernel has `CONFIG_DMABUF_HEAPS` disabled, so only the core,
platform and PCI modules can be loaded there.
