#pragma once

#include "GpuDetector.h"
#include "Workers.h"

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onDetectFinished(const QList<GpuInfo> &gpus, const QString &aurHelper);
    void onInstallLog(const QString &line);
    void onInstallFinished(bool success, const QString &message);
    void onInstallClicked();

private:
    void buildUi();
    QWidget *buildHeader();
    QWidget *buildGpuCard(const GpuInfo &gpu, int index);
    QWidget *buildLogPanel();
    QWidget *buildButtonBar();

    void showDetecting();
    void showGpus(const QList<GpuInfo> &gpus);
    void showInstalling();

    QString vendorEmoji(GpuVendor v) const;
    QString vendorColor(GpuVendor v) const;
    QString vendorName(GpuVendor v)  const;

    // Widgets
    QWidget      *m_centralWidget   = nullptr;
    QVBoxLayout  *m_rootLayout      = nullptr;
    QScrollArea  *m_scrollArea      = nullptr;

    QWidget      *m_detectionPane   = nullptr;
    QLabel       *m_statusLabel     = nullptr;
    QProgressBar *m_detectProgress  = nullptr;

    QWidget      *m_gpuPane         = nullptr;
    QVBoxLayout  *m_gpuLayout       = nullptr;

    QWidget      *m_logPane         = nullptr;
    QTextEdit    *m_logView         = nullptr;

    QPushButton  *m_installBtn      = nullptr;
    QLabel       *m_aurLabel        = nullptr;

    // State
    QList<GpuInfo> m_gpus;
    QString        m_aurHelper;
};
