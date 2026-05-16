#pragma once

#include "GpuDetector.h"
#include <QThread>
#include <QStringList>

// Runs GPU detection + package resolution off the GUI thread.
class DetectWorker : public QThread
{
    Q_OBJECT
public:
    explicit DetectWorker(QObject *parent = nullptr);
    void run() override;
signals:
    void finished(const QList<GpuInfo> &gpus, const QString &aurHelper);
};


// Runs pacman / AUR installation off the GUI thread.
class InstallWorker : public QThread
{
    Q_OBJECT
public:
    explicit InstallWorker(
        const QStringList &pacmanPkgs,
        const QStringList &aurPkgs,
        const QString     &aurHelper,
        QObject *parent = nullptr);
    void run() override;

signals:
    void logLine(const QString &line);
    void finished(bool success, const QString &message);

private:
    bool runCommand(const QString &program, const QStringList &args);

    QStringList m_pacmanPkgs;
    QStringList m_aurPkgs;
    QString     m_aurHelper;
};
