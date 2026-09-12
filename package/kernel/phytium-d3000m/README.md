# Phytium D3000M out-of-tree drivers

`src/` contains the unmodified DC, GPU, VPU and NPU sources from
`phytium-d3000m-drivers` commit `76e06c2f46ef8b29ca509dd9b67e3a9c87533051`.
The sources were imported from `/home/lean/Desktop/phytium-d3000m-drivers`.
All adaptations are applied through `patches/` during package preparation.

| Package | Modules |
| --- | --- |
| kmod-phytium-dc | ftd330-drm-dc |
| kmod-phytium-gpu | ftg340 |
| kmod-phytium-vpu | ftv310_vpu_driver |
| kmod-phytium-npu | phytium_npu, phytium_npu_platform, phytium_npu_pci, phytium_npu_heap |

The four packages are independently selectable and included by default in
Phytium armv8 EFI images. The FTD330 display driver is separate from the
older in-tree Phytium display driver used by other Phytium boards.
The NPU heap module is loaded at boot so its DMA buffer heap is available;
platform and PCI drivers carry their device aliases.

The package builds kernel drivers only. GPU graphics, video codec and NPU
inference applications need their corresponding userspace runtimes.

Build with `make -j30 package/kernel/phytium-d3000m/compile` after preparing
the selected target kernel. Use `make -j1 V=s` to diagnose failures.
