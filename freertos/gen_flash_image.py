#!/usr/bin/env python3
"""
AN8855 (NDS32) SPI NOR Flash 烧录镜像生成工具

功能:
  - 组装 bootloader + 固件为完整的烧录镜像
  - 支持单镜像/双镜像布局
  - 支持不同 SystemSize 配置
  - 查看已有镜像的分区信息
  - 从完整镜像中提取分区

用法:
  python3 gen_flash_image.py --bootloader bootloader/project/an8855/build/airRTOSBoot.bin --firmware project/an8855/build/airRTOSSystem.bin -o an8855_flash.bin
  python gen_flash_image.py --bootloader boot.bin --firmware fw.img --output flash.bin
  python gen_flash_image.py --firmware fw.img --output upgrade.img
  python gen_flash_image.py --info flash.bin
  python gen_flash_image.py --extract flash.bin --output-dir ./parts
"""

import argparse
import os
import struct
import sys
import zlib

# ============================================================================
# Flash 布局常量 (匹配 platform.h)
# ============================================================================
S_256 = 0x100
S_1K = 0x400
S_4K = 0x1000
S_16K = 0x4000
S_32K = 0x8000
S_64K = 0x10000
S_112K = 0x1C000
S_128K = 0x20000
S_256K = 0x40000
S_512K = 0x80000
S_1M = 0x100000

# SystemSize 选项 (匹配 platform.h DEFAULT_SYSTEMSIZE)
SYSTEM_SIZE_SMALL = S_1M + S_512K            # 1.5 MB (#else 分支, 无 AIR_MW)
SYSTEM_SIZE_LARGE = S_1M + S_1M + S_512K     # 2.5 MB (默认, AN8855 带 AIR_MW)

# image_header_t 魔数
FW_MAGIC_NUM = b"ARHT"
IMAGE_HEADER_SIZE = 256

# Flash 填充字节 (NOR Flash 擦除后为 0xFF)
FLASH_FILL_BYTE = b'\xff'


# ============================================================================
# Flash 分区布局类
# ============================================================================
class FlashLayout:
    def __init__(self, system_size=SYSTEM_SIZE_LARGE, dual_image=True, fw_signature=False):
        self.system_size = system_size
        self.dual_image = dual_image
        self.fw_signature = fw_signature

        self.bootloader_base = 0x20000000
        self.bootloader_size = S_112K

        self.bootconfig_base = self.bootloader_base + self.bootloader_size
        self.bootconfig_size = 12 * 1024  # 12KB

        self.manufacture_base = self.bootconfig_base + self.bootconfig_size
        self.manufacture_size = S_4K

        self.mainsystem_base = self.manufacture_base + self.manufacture_size
        self.mainsystem_size = system_size

        if dual_image:
            self.backupsystem_base = self.mainsystem_base + self.mainsystem_size
            self.backupsystem_size = system_size
            self.systemconfig_base = self.backupsystem_base + self.backupsystem_size
        else:
            self.backupsystem_base = None
            self.backupsystem_size = 0
            self.systemconfig_base = self.mainsystem_base + self.mainsystem_size

        self.systemconfig_size = S_64K

        self.tempsystem_base = self.systemconfig_base + self.systemconfig_size
        self.tempsystem_size = system_size

        if fw_signature:
            self.fwsignature_base = self.tempsystem_base + self.tempsystem_size
            self.fwsignature_size = S_64K
        else:
            self.fwsignature_base = None
            self.fwsignature_size = 0

        self.flash_end = (self.fwsignature_base + self.fwsignature_size
                          if fw_signature
                          else self.tempsystem_base + self.tempsystem_size)

    def get_partitions(self):
        parts = [
            ("BootLoader", self.bootloader_base, self.bootloader_size, True),
            ("BootConfig", self.bootconfig_base, self.bootconfig_size, False),
            ("Manufacture", self.manufacture_base, self.manufacture_size, False),
            ("MainSystem", self.mainsystem_base, self.mainsystem_size, True),
        ]
        if self.dual_image:
            parts.append(("BackupSystem", self.backupsystem_base, self.backupsystem_size, False))
        parts.append(("SystemConfig", self.systemconfig_base, self.systemconfig_size, False))
        parts.append(("TempSystem", self.tempsystem_base, self.tempsystem_size, False))
        if self.fw_signature:
            parts.append(("FwSignature", self.fwsignature_base, self.fwsignature_size, False))
        return parts


# ============================================================================
# image_header_t 结构体 (匹配 C 定义)
# ============================================================================
class ImageHeader:
    FORMAT = '<IIIIIII I 56I'  # little-endian
    SIZE = IMAGE_HEADER_SIZE

    def __init__(self):
        self.magic_num = FW_MAGIC_NUM
        self.img_model = 0
        self.img_version_major = 0
        self.img_version_minor = 0
        self.img_version_build = 1
        self.img_offset = IMAGE_HEADER_SIZE
        self.img_len = 0
        self.img_crc = 0
        self.sig_offset = 0
        self.sig_len = 0
        self.reserved = [0] * 56

    def pack(self):
        version_raw = (self.img_version_major |
                       (self.img_version_minor << 8) |
                       (self.img_version_build << 16))
        data = struct.pack(
            self.FORMAT,
            struct.unpack('<I', self.magic_num)[0],
            self.img_model,
            version_raw,
            self.img_offset,
            self.img_len,
            self.img_crc,
            self.sig_offset,
            self.sig_len,
            *self.reserved
        )
        return data

    @classmethod
    def unpack(cls, data):
        if len(data) < cls.SIZE:
            return None
        fields = struct.unpack(cls.FORMAT, data[:cls.SIZE])
        hdr = cls()
        hdr.magic_num = struct.pack('<I', fields[0])
        hdr.img_model = fields[1]
        version_raw = fields[2]
        hdr.img_version_major = version_raw & 0xFF
        hdr.img_version_minor = (version_raw >> 8) & 0xFF
        hdr.img_version_build = (version_raw >> 16) & 0xFFFF
        hdr.img_offset = fields[3]
        hdr.img_len = fields[4]
        hdr.img_crc = fields[5]
        hdr.sig_offset = fields[6]
        hdr.sig_len = fields[7]
        hdr.reserved = list(fields[8:])
        return hdr

    def has_valid_magic(self):
        return self.magic_num == FW_MAGIC_NUM


# ============================================================================
# CRC32 (匹配 fwheader.c 实现)
# ============================================================================
def crc32_airoha(data):
    return zlib.crc32(data) & 0xFFFFFFFF


# ============================================================================
# 核心功能
# ============================================================================
def build_flash_image(layout, bootloader_data, firmware_payload):
    total_size = layout.flash_end - layout.bootloader_base
    image = bytearray(FLASH_FILL_BYTE * total_size)

    # 写入 bootloader
    bootloader_offset = layout.bootloader_base - layout.bootloader_base
    image[bootloader_offset:bootloader_offset + len(bootloader_data)] = bootloader_data

    # 写入固件裸二进制到 MainSystemBase
    # boot-rtos 直接跳转到 MainSystemBase 执行, 不能带 image_header_t 头部
    # 调用者已确保 firmware_payload 是剥离头部后的裸二进制
    fw_offset = layout.mainsystem_base - layout.bootloader_base
    image[fw_offset:fw_offset + len(firmware_payload)] = firmware_payload

    # 双镜像：备份系统和主系统相同
    if layout.dual_image:
        bk_offset = layout.backupsystem_base - layout.bootloader_base
        main_start = layout.mainsystem_base - layout.bootloader_base
        main_end = main_start + layout.mainsystem_size
        image[bk_offset:bk_offset + layout.backupsystem_size] = image[main_start:main_end]

    return bytes(image)


def print_image_info(layout, image_data=None, bootloader_path=None, firmware_path=None):
    print("=" * 70)
    print("AN8855 Flash 烧录镜像信息")
    print("=" * 70)

    config_str = []
    config_str.append(f"SystemSize: 0x{layout.system_size:X} ({layout.system_size // 1024} KB)")
    config_str.append(f"DUAL_IMAGE: {'启用' if layout.dual_image else '禁用'}")
    config_str.append(f"FW_SIGNATURE: {'启用' if layout.fw_signature else '禁用'}")
    print(f"配置: {', '.join(config_str)}")
    print()

    print(f"{'分区名':<20} {'基地址':<16} {'结束地址':<16} {'大小':<12} {'状态'}")
    print("-" * 70)

    for name, base, size, has_data in layout.get_partitions():
        end = base + size
        base_str = f"0x{base:08X}"
        end_str = f"0x{end:08X}"
        size_str = f"{size // 1024} KB" if size >= 1024 else f"{size} B"
        status = ""

        if image_data is not None:
            offset = base - layout.bootloader_base
            chunk = image_data[offset:offset + min(size, 64)]
            is_empty = all(b == 0xFF for b in chunk)
            if is_empty:
                status = "空 (0xFF)"
            else:
                non_zero = sum(1 for b in image_data[offset:offset + size] if b != 0xFF)
                pct = non_zero * 100.0 / size if size > 0 else 0
                status = f"已使用 {pct:.1f}%"
        elif has_data and (bootloader_path or firmware_path):
            status = "将写入数据"
        else:
            status = "填充 0xFF"

        print(f"{name:<20} {base_str:<16} {end_str:<16} {size_str:<12} {status}")

    print("-" * 70)
    if image_data is not None:
        total_mb = len(image_data) / (1024 * 1024)
        print(f"总大小: {len(image_data)} 字节 ({total_mb:.2f} MB)")


def extract_partitions(layout, image_data, output_dir):
    os.makedirs(output_dir, exist_ok=True)
    extracted = []

    for name, base, size, _ in layout.get_partitions():
        offset = base - layout.bootloader_base
        chunk = image_data[offset:offset + size]
        filename = f"{name}.bin"
        filepath = os.path.join(output_dir, filename)
        with open(filepath, 'wb') as f:
            f.write(chunk)
        extracted.append((name, filepath, len(chunk)))

    return extracted


# ============================================================================
# 主入口
# ============================================================================
def main():
    parser = argparse.ArgumentParser(
        description="AN8855 SPI NOR Flash 烧录镜像生成工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
使用示例:
  %(prog)s --bootloader boot.bin --firmware fw.img --output flash.bin
  %(prog)s --firmware fw.bin --output upgrade.img
  %(prog)s --info flash.bin
  %(prog)s --extract flash.bin --output-dir ./parts
        """
    )

    mode_group = parser.add_argument_group("操作模式")
    mode_group.add_argument("--info", metavar="FILE", help="查看已有镜像的分区信息")
    mode_group.add_argument("--extract", metavar="FILE", help="从已有镜像中提取各分区")

    build_group = parser.add_argument_group("构建参数（默认模式）")
    build_group.add_argument("--bootloader", metavar="FILE", help="bootloader 二进制文件路径")
    build_group.add_argument("--firmware", metavar="FILE", help="固件镜像文件路径 (.img 或 .bin)")
    build_group.add_argument("--output", "-o", metavar="FILE", help="输出文件路径")

    config_group = parser.add_argument_group("布局配置")
    config_group.add_argument("--system-size", metavar="SIZE", type=lambda x: int(x, 0),
                              default=SYSTEM_SIZE_LARGE,
                              help=f"SystemSize (默认: 0x{SYSTEM_SIZE_LARGE:X} = {SYSTEM_SIZE_LARGE // S_1M}.{int((SYSTEM_SIZE_LARGE % S_1M) / S_512K * 0.5)}MB)")
    config_group.add_argument("--small-system", action="store_true",
                              help=f"使用小系统尺寸 (0x{SYSTEM_SIZE_SMALL:X}, 无 AIR_MW_SUPPORT)")
    config_group.add_argument("--dual-image", action="store_true", default=True,
                              help="启用双镜像布局 (默认启用)")
    config_group.add_argument("--no-dual-image", action="store_false", dest="dual_image",
                              help="禁用双镜像布局")
    config_group.add_argument("--fw-signature", action="store_true", default=False,
                              help="启用固件签名分区")
    config_group.add_argument("--output-dir", metavar="DIR", default="./extracted",
                              help="提取模式输出目录 (默认: ./extracted)")

    args = parser.parse_args()

    # 处理 system-size
    if args.small_system:
        args.system_size = SYSTEM_SIZE_SMALL

    layout = FlashLayout(
        system_size=args.system_size,
        dual_image=args.dual_image,
        fw_signature=args.fw_signature
    )

    # --- 信息模式 ---
    if args.info:
        if not os.path.exists(args.info):
            print(f"错误: 文件不存在: {args.info}", file=sys.stderr)
            sys.exit(1)
        with open(args.info, 'rb') as f:
            image_data = f.read()
        expected_size = layout.flash_end - layout.bootloader_base
        if len(image_data) != expected_size:
            print(f"警告: 文件大小 0x{len(image_data):X} 与预期 0x{expected_size:X} 不符")
        print_image_info(layout, image_data=image_data)
        return

    # --- 提取模式 ---
    if args.extract:
        if not os.path.exists(args.extract):
            print(f"错误: 文件不存在: {args.extract}", file=sys.stderr)
            sys.exit(1)
        with open(args.extract, 'rb') as f:
            image_data = f.read()
        print_image_info(layout, image_data=image_data)
        print(f"\n正在提取分区到: {args.output_dir}")
        extracted = extract_partitions(layout, image_data, args.output_dir)
        for name, path, size in extracted:
            size_str = f"{size / 1024:.1f} KB" if size >= 1024 else f"{size} B"
            print(f"  {name:<20} -> {path}  ({size_str})")
        print("提取完成")
        return

    # --- 构建模式 ---
    if not args.firmware:
        parser.print_help()
        print("\n错误: 必须指定 --firmware (或使用 --info / --extract 模式)", file=sys.stderr)
        sys.exit(1)

    # 读取固件
    if not os.path.exists(args.firmware):
        print(f"错误: 固件文件不存在: {args.firmware}", file=sys.stderr)
        sys.exit(1)

    with open(args.firmware, 'rb') as f:
        firmware_data = f.read()

    # 检测是否为 .img (带头部)
    firmware_is_img = False
    fw_payload = firmware_data
    fw_header = None
    if len(firmware_data) >= IMAGE_HEADER_SIZE:
        hdr = ImageHeader.unpack(firmware_data)
        if hdr and hdr.has_valid_magic():
            firmware_is_img = True
            fw_header = hdr
            fw_payload = firmware_data[IMAGE_HEADER_SIZE:IMAGE_HEADER_SIZE + hdr.img_len]
            print(f"固件镜像: {args.firmware} (已含 image_header 头部, CRC32: 0x{hdr.img_crc:08X})")
        else:
            print(f"固件镜像: {args.firmware} (裸二进制)")

    fw_size = len(fw_payload)
    fw_size_mb = fw_size / (1024 * 1024)
    sys_size_kb = layout.system_size / 1024
    if fw_size > layout.system_size:
        print(f"错误: 固件实际数据大小 ({fw_size} 字节, {fw_size_mb:.2f} MB) "
              f"超过 SystemSize ({sys_size_kb:.0f} KB)", file=sys.stderr)
        sys.exit(1)

    if fw_size_mb > 1:
        print(f"固件实际数据大小: {fw_size} 字节 ({fw_size_mb:.2f} MB)")
    else:
        print(f"固件实际数据大小: {fw_size} 字节 ({fw_size / 1024:.1f} KB)")
    print(f"SystemSize: {sys_size_kb:.0f} KB")

    # === 模式1: 固件升级镜像模式 (无 bootloader) ===
    # 输出一个带 image_header 头部的 .img 文件, 用于 bootloader 的 upgrade 命令
    if not args.bootloader:
        print("\n构建固件升级镜像...")
        if firmware_is_img:
            output_data = firmware_data
        else:
            hdr = ImageHeader()
            hdr.img_len = fw_size
            hdr.img_crc = crc32_airoha(fw_payload)
            output_data = hdr.pack() + fw_payload

        output_path = args.output or "firmware_upgrade.img"
        with open(output_path, 'wb') as f:
            f.write(output_data)

        img_mb = len(output_data) / (1024 * 1024)
        print(f"输出文件: {output_path} ({len(output_data)} 字节, {img_mb:.2f} MB)")
        if not firmware_is_img:
            print(f"已添加 image_header 头部 (CRC32: 0x{hdr.img_crc:08X}, 长度: {fw_size})")
        else:
            print(f"固件 CRC32 校验: 通过 (0x{fw_header.img_crc:08X})")
        print("(image_header 头部已保留, 供 bootloader upgrade 命令识别)")
        print("\n完成!")
        return

    # === 模式2: 完整烧录镜像模式 (含 bootloader) ===
    # 读取 bootloader
    if not os.path.exists(args.bootloader):
        print(f"错误: bootloader 文件不存在: {args.bootloader}", file=sys.stderr)
        sys.exit(1)

    with open(args.bootloader, 'rb') as f:
        bootloader_data = f.read()
    bl_size_kb = len(bootloader_data) / 1024
    print(f"Bootloader: {args.bootloader} ({bl_size_kb:.1f} KB)")

    if len(bootloader_data) > layout.bootloader_size:
        print(f"错误: Bootloader 大小 ({len(bootloader_data)} 字节) "
              f"超过分区大小 ({layout.bootloader_size} 字节)", file=sys.stderr)
        sys.exit(1)

    # 构建完整 Flash 镜像
    print("\n构建完整烧录镜像...")
    if firmware_is_img:
        print("(已自动剥离 image_header 头部, MainSystemBase 写入裸二进制)")
    image_data = build_flash_image(layout, bootloader_data, fw_payload)

    # 确定输出文件名
    output_path = args.output or "flash_image.bin"

    # 写入
    with open(output_path, 'wb') as f:
        f.write(image_data)

    image_mb = len(image_data) / (1024 * 1024)
    print(f"输出文件: {output_path} ({len(image_data)} 字节, {image_mb:.2f} MB)")

    # 显示布局
    print()
    print_image_info(layout, image_data=image_data)

    # 固件 CRC 校验
    if firmware_is_img and fw_header:
        actual_crc = crc32_airoha(fw_payload)
        if actual_crc == fw_header.img_crc:
            print(f"固件 CRC32 校验: 通过 (0x{actual_crc:08X})")
        else:
            print(f"固件 CRC32 校验: 失败 (期望 0x{fw_header.img_crc:08X}, 实际 0x{actual_crc:08X})")
            print("警告: 固件镜像可能已损坏", file=sys.stderr)

    print("\n完成!")


if __name__ == "__main__":
    main()
