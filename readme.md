```
git submodule update --init --recursive
```

# 编译EDK2
https://www.bilibili.com/read/cv12197402/?from=readlist

```bash
export EDK_TOOLS_PATH=$HOME/.../edk2/BaseTools
. edksetup.sh BaseTools

build
```
```bash
alias qemuovmf="qemu-system-x86_64 -m 4096 -drive if=pflash,format=raw,file=/home/zheng/workspace/ovmf/OVMF_CODE.fd,readonly=on -drive if=pflash,format=raw,file=/home/zheng/workspace/ovmf/OVMF_VARS.fd,readonly=on -drive format=raw,file=fat:rw:/home/zheng/workspace/ovmf/esp -net none"
```