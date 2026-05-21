#include "PackageResolver.h"
#include <QProcess>

// ─── Package tables ───────────────────────────────────────────────────────────

static const QStringList NV_CURRENT_PACMAN = {
    "nvidia-open", "nvidia-utils", "nvidia-settings",
    "lib32-nvidia-utils", "opencl-nvidia"
};

static const QStringList NV_KEPLER_PACMAN = { "linux-headers" };
static const QStringList NV_KEPLER_AUR    = {
    "nvidia-470xx-dkms", "nvidia-470xx-utils",
    "nvidia-470xx-settings", "lib32-nvidia-470xx-utils"
};

static const QStringList NV_390_PACMAN = { "linux-headers" };
static const QStringList NV_390_AUR    = {
    "nvidia-390xx-dkms", "nvidia-390xx-utils",
    "lib32-nvidia-390xx-utils"
};

static const QStringList AMD_PACMAN = {
    "mesa", "lib32-mesa",
    "vulkan-radeon", "lib32-vulkan-radeon",
    "xf86-video-amdgpu"
};

static const QStringList INTEL_PACMAN = {
    "mesa", "lib32-mesa",
    "vulkan-intel", "lib32-vulkan-intel",
    "intel-media-driver",
    "libva-intel-driver",
    "xf86-video-intel"
};

static const QStringList COMMON_PACMAN = {
    "mesa-utils", "vulkan-tools"
};

// ─── Public ───────────────────────────────────────────────────────────────────

void PackageResolver::resolve(QList<GpuInfo> &gpus) const
{
    for (GpuInfo &gpu : gpus) {
        switch (gpu.vendor) {
        case GpuVendor::NVIDIA: resolveNvidia(gpu); break;
        case GpuVendor::AMD:    resolveAmd(gpu);    break;
        case GpuVendor::Intel:  resolveIntel(gpu);  break;
        default: break;
        }
        for (const QString &p : COMMON_PACMAN)
            if (!gpu.pacmanPackages.contains(p))
                gpu.pacmanPackages.append(p);
    }
}

QString PackageResolver::aurHelper()
{
    QProcess p;
    p.start("which", QStringList() << "vpkg");
    p.waitForFinished(3000);
    return p.exitCode() == 0 ? QStringLiteral("vpkg") : QString();
}

bool PackageResolver::isInstalled(const QString &pkg)
{
    QProcess p;
    p.start("pacman", QStringList() << "-Q" << pkg);
    p.waitForFinished(3000);
    return p.exitCode() == 0;
}

QStringList PackageResolver::missing(const QStringList &pkgs)
{
    QStringList result;
    for (const QString &pkg : pkgs)
        if (!isInstalled(pkg))
            result.append(pkg);
    return result;
}

// ─── Private ──────────────────────────────────────────────────────────────────

void PackageResolver::resolveNvidia(GpuInfo &gpu) const
{
    switch (gpu.nvGen) {
    case NvidiaGeneration::Current:
        gpu.pacmanPackages = NV_CURRENT_PACMAN;
        break;
    case NvidiaGeneration::Kepler:
        gpu.pacmanPackages = NV_KEPLER_PACMAN;
        gpu.aurPackages    = NV_KEPLER_AUR;
        break;
    case NvidiaGeneration::Legacy390:
        gpu.pacmanPackages = NV_390_PACMAN;
        gpu.aurPackages    = NV_390_AUR;
        break;
    default:
        gpu.pacmanPackages = NV_CURRENT_PACMAN;
        break;
    }
}

void PackageResolver::resolveAmd(GpuInfo &gpu) const
{
    gpu.pacmanPackages = AMD_PACMAN;
}

void PackageResolver::resolveIntel(GpuInfo &gpu) const
{
    gpu.pacmanPackages = INTEL_PACMAN;
}
