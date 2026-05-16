#pragma once

#include "GpuDetector.h"
#include <QStringList>

class PackageResolver
{
public:
    PackageResolver() = default;

    // Populate pacmanPackages / aurPackages for every GPU in the list.
    void resolve(QList<GpuInfo> &gpus) const;

    // Returns the first available AUR helper: yay > paru > pikaur, or "".
    static QString aurHelper();

    // True if a pacman package is already installed.
    static bool isInstalled(const QString &pkg);

    // Filter a list down to only packages not yet installed.
    static QStringList missing(const QStringList &pkgs);

private:
    void resolveNvidia(GpuInfo &gpu) const;
    void resolveAmd(GpuInfo &gpu)   const;
    void resolveIntel(GpuInfo &gpu) const;
};
