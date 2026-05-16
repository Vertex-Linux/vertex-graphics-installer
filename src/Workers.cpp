#include "Workers.h"
#include "GpuDetector.h"
#include "PackageResolver.h"

#include <QProcess>

// ─── DetectWorker ─────────────────────────────────────────────────────────────

DetectWorker::DetectWorker(QObject *parent) : QThread(parent) {}

void DetectWorker::run()
{
    GpuDetector     detector;
    PackageResolver resolver;

    QList<GpuInfo> gpus = detector.detect();
    resolver.resolve(gpus);

    emit finished(gpus, PackageResolver::aurHelper());
}

// ─── InstallWorker ────────────────────────────────────────────────────────────

InstallWorker::InstallWorker(
    const QStringList &pacmanPkgs,
    const QStringList &aurPkgs,
    const QString     &aurHelper,
    QObject *parent)
    : QThread(parent)
    , m_pacmanPkgs(pacmanPkgs)
    , m_aurPkgs(aurPkgs)
    , m_aurHelper(aurHelper)
{}

void InstallWorker::run()
{
    // ── Step 1: pacman ────────────────────────────────────────────────────────
    QStringList missing = PackageResolver::missing(m_pacmanPkgs);

    if (!missing.isEmpty()) {
        emit logLine("==> Installing from official repositories...");
        emit logLine("    Packages: " + missing.join("  "));
        emit logLine("");

        QStringList args;
        args << "pacman" << "--noconfirm" << "--needed" << "-S" << missing;

        if (!runCommand("pkexec", args)) {
            emit finished(false, "pacman installation failed. See log for details.");
            return;
        }
        emit logLine("\n==> Repo packages installed.\n");
    } else {
        emit logLine("==> All repo packages already installed.\n");
    }

    // ── Step 2: AUR ───────────────────────────────────────────────────────────
    QStringList missingAur = PackageResolver::missing(m_aurPkgs);

    if (!missingAur.isEmpty()) {
        if (m_aurHelper.isEmpty()) {
            emit logLine("⚠  AUR packages required but no AUR helper found.");
            emit logLine("   Install yay or paru, then re-run this tool.");
            emit finished(false, "No AUR helper (yay/paru/pikaur) found.");
            return;
        }

        emit logLine("==> Installing from AUR via " + m_aurHelper + "...");
        emit logLine("    Packages: " + missingAur.join("  "));
        emit logLine("");

        // AUR helpers must NOT run as root — they escalate internally.
        QStringList args;
        args << "--noconfirm" << "--needed" << "-S" << missingAur;

        if (!runCommand(m_aurHelper, args)) {
            emit finished(false, "AUR installation failed. See log for details.");
            return;
        }
        emit logLine("\n==> AUR packages installed.\n");

    } else if (!m_aurPkgs.isEmpty()) {
        emit logLine("==> All AUR packages already installed.\n");
    }

    emit finished(true, "All drivers installed successfully!\nA reboot is recommended.");
}

bool InstallWorker::runCommand(const QString &program, const QStringList &args)
{
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);

    connect(&proc, &QProcess::readyRead, [&]() {
        while (proc.canReadLine()) {
            const QString line = QString::fromLocal8Bit(proc.readLine()).trimmed();
            if (!line.isEmpty())
                emit logLine(line);
        }
    });

    proc.start(program, args);
    proc.waitForFinished(-1);

    // Drain remaining output
    while (proc.canReadLine()) {
        const QString line = QString::fromLocal8Bit(proc.readLine()).trimmed();
        if (!line.isEmpty())
            emit logLine(line);
    }

    return proc.exitCode() == 0;
}
