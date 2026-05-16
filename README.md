# Vertex Graphics

A native **C++ / Qt6** GUI tool for Arch Linux.  
Detects your GPU(s) and installs the correct drivers via **pacman** and the **AUR**.  
Inherits the active **KDE Plasma** color theme automatically.

---

## Quick Start

```bash
# 1. Install build dependencies (one-time)
sudo pacman -S --needed cmake make gcc qt6-base pciutils

# 2. Build
chmod +x build.sh
./build.sh

# 3. Run
./build/arch-gpu-driver-installer

# Or install system-wide
./build.sh --install
arch-gpu-driver-installer
```

---

## Supported Drivers

| GPU | Driver | Source |
|-----|--------|--------|
| NVIDIA RTX / GTX 900+ (Maxwell+) | `nvidia` + `nvidia-utils` | pacman |
| NVIDIA GTX 600/700 (Kepler) | `nvidia-470xx-dkms` | AUR |
| NVIDIA GTX 400/500 (Fermi) | `nvidia-390xx-dkms` | AUR |
| AMD (all) | `mesa` + `vulkan-radeon` + `xf86-video-amdgpu` | pacman |
| Intel (all) | `mesa` + `vulkan-intel` + `intel-media-driver` | pacman |

AUR packages require **yay**, **paru**, or **pikaur** to be installed.

---

## Project Structure

```
arch-gpu-driver-installer/
├── CMakeLists.txt          # CMake build definition
├── build.sh                # One-command build + run + install script
├── README.md
└── src/
    ├── main.cpp            # QApplication entry point
    ├── GpuDetector.h/cpp   # lspci parsing & vendor/generation detection
    ├── PackageResolver.h/cpp  # GPU → package list mapping
    ├── Workers.h/cpp       # Background threads (detect + install)
    └── MainWindow.h/cpp    # Qt6 Widgets UI
```

---

## Notes

- **Root access**: `pkexec` (polkit) is used to run `pacman` — a GUI password
  prompt appears. No terminal or `sudo` needed.
- **AUR helpers**: called without root since they handle privilege escalation internally.
- **KDE theme**: Plasma sets `QT_QPA_PLATFORMTHEME=kde` in every session, so the
  app's `QPalette` matches your active color scheme automatically.
- **Reboot prompt**: offered automatically after a successful install.
