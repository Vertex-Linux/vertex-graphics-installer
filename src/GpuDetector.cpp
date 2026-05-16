#include "GpuDetector.h"

#include <QProcess>
#include <QRegularExpression>

static const QStringList KEPLER_HINTS = {
    "GTX 6", "GTX 7", "GT 6", "GT 7",
    "Quadro K", "Tesla K", "NVS 5", "NVS 6"
};

static const QStringList LEGACY390_HINTS = {
    "GTX 4", "GTX 5", "GT 4", "GT 5",
    "GTS 4", "GTS 5", "Quadro 4", "Quadro 5",
    "Tesla C", "Tesla M2", "Tesla S"
};

// ─── Public ───────────────────────────────────────────────────────────────────

QList<GpuInfo> GpuDetector::detect() const
{
    QList<GpuInfo> result;

    QProcess proc;
    proc.start("lspci", QStringList() << "-mm" << "-nn");
    proc.waitForFinished(5000);

    const QString out = proc.readAllStandardOutput();
    if (out.isEmpty()) {
        GpuInfo unknown;
        unknown.model       = "lspci returned no output (is pciutils installed?)";
        unknown.driverLabel = "Unable to detect GPU";
        result.append(unknown);
        return result;
    }

    for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
        const QString low = line.toLower();
        if (low.contains("vga") || low.contains("display") ||
            low.contains("3d controller") || low.contains("2d video"))
        {
            result.append(parseLspciLine(line));
        }
    }

    if (result.isEmpty()) {
        GpuInfo unknown;
        unknown.model       = "No display adapter found via lspci";
        unknown.driverLabel = "No GPU detected";
        result.append(unknown);
    }

    return result;
}

// ─── Private ──────────────────────────────────────────────────────────────────

GpuInfo GpuDetector::parseLspciLine(const QString &line) const
{
    GpuInfo gpu;

    static QRegularExpression pciRe(R"(\[([0-9a-fA-F]{4}:[0-9a-fA-F]{4})\])");
    auto m = pciRe.match(line);
    if (m.hasMatch())
        gpu.pciId = m.captured(1);

    gpu.model  = extractModel(line);
    gpu.vendor = detectVendor(line);

    if (gpu.vendor == GpuVendor::NVIDIA)
        gpu.nvGen = detectNvidiaGen(gpu.model);

    gpu.driverLabel = driverLabel(gpu);
    return gpu;
}

QString GpuDetector::extractModel(const QString &line) const
{
    // lspci -mm wraps each field in quotes; device name is the 3rd quoted token
    static QRegularExpression qRe("\"([^\"]*)\"");
    QRegularExpressionMatchIterator it = qRe.globalMatch(line);
    int idx = 0;
    while (it.hasNext()) {
        auto match = it.next();
        if (idx == 2) return match.captured(1);
        ++idx;
    }
    return line.trimmed();
}

GpuVendor GpuDetector::detectVendor(const QString &line) const
{
    const QString low = line.toLower();
    if (low.contains("nvidia"))                                    return GpuVendor::NVIDIA;
    if (low.contains("amd") || low.contains("radeon") ||
        low.contains("ati"))                                       return GpuVendor::AMD;
    if (low.contains("intel"))                                     return GpuVendor::Intel;
    return GpuVendor::Unknown;
}

NvidiaGeneration GpuDetector::detectNvidiaGen(const QString &model) const
{
    for (const QString &hint : LEGACY390_HINTS)
        if (model.contains(hint, Qt::CaseInsensitive))
            return NvidiaGeneration::Legacy390;

    for (const QString &hint : KEPLER_HINTS)
        if (model.contains(hint, Qt::CaseInsensitive))
            return NvidiaGeneration::Kepler;

    return NvidiaGeneration::Current;
}

QString GpuDetector::driverLabel(const GpuInfo &gpu) const
{
    switch (gpu.vendor) {
    case GpuVendor::NVIDIA:
        switch (gpu.nvGen) {
        case NvidiaGeneration::Current:
            return "NVIDIA – Official Driver (nvidia + nvidia-utils)";
        case NvidiaGeneration::Kepler:
            return "NVIDIA Kepler – Legacy 470xx Driver (AUR)";
        case NvidiaGeneration::Legacy390:
            return "NVIDIA Fermi – Legacy 390xx Driver (AUR)";
        default:
            return "NVIDIA – Driver (generation unknown)";
        }
    case GpuVendor::AMD:
        return "AMD – AMDGPU + Mesa + Vulkan (open source)";
    case GpuVendor::Intel:
        return "Intel – i915 + Mesa + Vulkan (open source)";
    default:
        return "Unknown GPU – no driver selected";
    }
}
