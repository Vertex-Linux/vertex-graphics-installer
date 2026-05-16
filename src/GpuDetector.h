#pragma once

#include <QString>
#include <QStringList>
#include <QList>

enum class GpuVendor {
    NVIDIA,
    AMD,
    Intel,
    Unknown
};

enum class NvidiaGeneration {
    Current,    // Maxwell+ → official 'nvidia' package
    Kepler,     // 600/700 series → nvidia-470xx-dkms (AUR)
    Legacy390,  // 400/500 series → nvidia-390xx-dkms (AUR)
    Unknown
};

struct GpuInfo {
    GpuVendor        vendor      = GpuVendor::Unknown;
    NvidiaGeneration nvGen       = NvidiaGeneration::Unknown;
    QString          model;
    QString          pciId;
    QString          driverLabel;

    QStringList pacmanPackages;
    QStringList aurPackages;

    bool needsAur()  const { return !aurPackages.isEmpty(); }
    bool isUnknown() const { return vendor == GpuVendor::Unknown; }
};

class GpuDetector
{
public:
    GpuDetector() = default;
    QList<GpuInfo> detect() const;

private:
    GpuInfo          parseLspciLine(const QString &line) const;
    GpuVendor        detectVendor(const QString &line)   const;
    NvidiaGeneration detectNvidiaGen(const QString &model) const;
    QString          extractModel(const QString &line)   const;
    QString          driverLabel(const GpuInfo &gpu)     const;
};
