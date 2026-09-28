
# Notes

## EFI

### GCC

```sh
gcc:
  x86_64-w64-mingw32-gcc \
    -std=c17             \
    -Wall                \
    -Wextra              \
    -Wpedantic           \
    -mno-red-zone        \
    -ffreestanding       \
    -nostdlib            \
    -Wl,--subsystem,10   \
    -e efi_main          \
    -O BOOTX64.EFI       \
    efi.c
```

### Clang

```sh
clang:
  clang efi.c                    \
  -target x86_64-unknown-windows \
  -std=c17                       \
  -Wall                          \
  -Wextra                        \
  -Wpedantic                     \
  -mno-red-zone                  \
  -ffreestanding                 \
  -nostdlib                      \
  -fuse-ld=lld-link              \
  -Wl,-subsystem:efi_application \
  -Wl,-entry:efi_main            \
  -o BOOTX64.EFI
```
