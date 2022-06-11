# 配置编译环境

以Debian/Ubuntu为例。

## 安装编译依赖

```bash
sudo apt install build-essential uuid-dev iasl nasm python3-distutils
```

> gcc 11会Warning无法编译，建议使用10及以下版本。
>
> 需要将python3链接到python，否则编译时会提示找不到。可以使用update-alternatives工具配置。

## 加载代码子模块

```bash
git submodule update --init --recursive
```

## 编译EDK2基本工具

子模块加载完成后，进入edk2目录。

```bash
make -C BaseTools
```

## 配置EDK2编译环境变量

```bash
export EDK_TOOLS_PATH=$HOME/{{项目存放目录}}/edk2/BaseTools
source edksetup.sh BaseTools
```

> 可以创建一个脚本用来初始化环境变量，如：
>
> ```bash
> cd $HOME/{{项目存放目录}}/edk2
> export EDK_TOOLS_PATH=$HOME/{{项目存放目录}}/edk2/BaseTools
> source edksetup.sh BaseTools
> ```
>
> 保存为`initedk.sh`。

打开`edk2/Conf/target.txt`，修改以下内容

```
ACTIVE_PLATFORM = {{将要编译的Pkg，见后文需要。}}
TARGET_ARCH = X64
TOOL_CHAIN_TAG = GCC5
```

此时只需在`edk2`目录下执行`build`命令，即可开始编译。

# 配置QEMU虚拟机

## QEMU安装

```bash
sudo apt install qemu qemu-system-x86
```

## 编译UEFI固件

QEMU不包含UEFI固件，需要自行编译一个。

> OVMF: Open Virtual Machine Firmware

在项目外任意位置创建几个目录：

```
ovmf
└── esp
    └── EFI
        └── Boot
```

打开`edk2/Conf/target.txt`，修改以下内容：

```
ACTIVE_PLATFORM = OvmfPkg/OvmfPkgX64.dsc
```

然后输入在`edk2`目录下输入`build`进行编译。

将`edk2/Build/OvmfX64/DEBUG_GCC5/X64/Shell.efi`（UEFI Shell）复制到上述`Boot`目录，并重命名为`BootX64.efi`。QEMU启动后将自动运行此UEFI Shell。

将`edk2/Build/OvmfX64/DEBUG_GCC5/FV/`目录下的`OVMF_CODE.fd`与`OVMF_VARS.fd`（UEFI固件）复制到上述`ovmf`目录。

打开`edk2/Conf/target.txt`，修改以下内容：

```
ACTIVE_PLATFORM = MdeModulePkg/MdeModulePkg.dsc
```

然后输入在`edk2`目录下输入`build`进行编译。

将`edk2/Build/MdeModule/DEBUG_GCC5/X64/HelloWorld.efi`复制到上述`esp`目录下，作为运行示例。

## 启动命令

```bash
alias qemuovmf="qemu-system-x86_64 -m 4096 -drive if=pflash,format=raw,file=/home/{{ovmf所在目录}}/ovmf/OVMF_CODE.fd,readonly=on -drive if=pflash,format=raw,file=/home/{{ovmf所在目录}}/ovmf/OVMF_VARS.fd,readonly=on -drive format=raw,file=fat:rw:/home/{{ovmf所在目录}}/ovmf/esp -net none"
```

执行启动命令，进入UEFI Shell后，输入`fs0:`进入文件系统，输入`ls`查看所有文件，运行`HelloWorld.efi`，如果运行成功，则所有配置工作完成。

# 编译BootLoader

将bootloader代码符号链接到edk2目录内：

```bash
ln -s $HOME/{{项目存放目录}}/bootloader $HOME/{{项目存放目录}}/edk2/GemBootLoaderPkg
```

打开`edk2/Conf/target.txt`，修改以下内容

```
ACTIVE_PLATFORM = GemBootLoaderPkg/GemBootLoaderPkg.dsc
TARGET_ARCH = X64
TOOL_CHAIN_TAG = GCC5
```

在`edk2`目录下运行`build`，开始编译。

# 参考链接

[「Coding Tools」 第3话 Ubuntu下EDK2开发环境搭建 - 哔哩哔哩 (bilibili.com)](https://www.bilibili.com/read/cv12197402/)

[「Coding Tools」 第4话 如何使用Qemu虚拟机运行EDK2编译结果 - 哔哩哔哩 (bilibili.com)](https://www.bilibili.com/read/cv12200267)