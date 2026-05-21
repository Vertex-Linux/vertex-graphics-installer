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
    // Resolve the full path to vpkg — pkexec sanitises PATH so we can't rely on it.
    QProcess whichProc;
    whichProc.start("which", {"vpkg"});
    whichProc.waitForFinished(3000);
    const QString vpkgPath = QString::fromLocal8Bit(whichProc.readAllStandardOutput()).trimmed();
    if (vpkgPath.isEmpty()) {
        emit finished(false, "vpkg is not installed or not in PATH.");
        return;
    }

    // ── Step 1: pacman (via vpkg pm install, elevated with pkexec) ────────────
    QStringList missing = PackageResolver::missing(m_pacmanPkgs);

    if (!missing.isEmpty()) {
        emit logLine("==> Installing from official repositories via vpkg...");
        emit logLine("    Packages: " + missing.join("  "));
        emit logLine("");

        // pkexec <vpkg-full-path> pm install --noconfirm <pkgs>
        QStringList args;
        args << vpkgPath << "pm" << "install" << "--noconfirm" << missing;

        if (!runCommand("pkexec", args)) {
            emit finished(false, "vpkg pm install failed. See log for details.");
            return;
        }
        emit logLine("\n==> Repo packages installed.\n");
    } else {
        emit logLine("==> All repo packages already installed.\n");
    }

    // ── Step 2: AUR (via vpkg aur install — must NOT run as root) ────────────
    QStringList missingAur = PackageResolver::missing(m_aurPkgs);

    if (!missingAur.isEmpty()) {
        emit logLine("==> Installing from AUR via vpkg...");
        emit logLine("    Packages: " + missingAur.join("  "));
        emit logLine("");

        QStringList args;
        args << "aur" << "install" << "--noconfirm" << missingAur;

        if (!runCommand(vpkgPath, args)) {
            emit finished(false, "vpkg aur install failed. See log for details.");
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
