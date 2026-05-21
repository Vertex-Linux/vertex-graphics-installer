#include "MainWindow.h"
#include "PackageResolver.h"

#include <QApplication>
#include <QMessageBox>
#include <QSizePolicy>
#include <QPalette>
#include <QTextCursor>
#include <QProcess>
#include <QSpacerItem>
#include <QIcon>
#include <algorithm>

// ─── Constructor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Vertex Graphics Installer");
    setMinimumSize(740, 580);
    resize(820, 660);

    buildUi();
    showDetecting();

    auto *worker = new DetectWorker(this);
    connect(worker, &DetectWorker::finished, this, &MainWindow::onDetectFinished);
    connect(worker, &DetectWorker::finished, worker, &QObject::deleteLater);
    worker->start();
}

// ─── UI Construction ──────────────────────────────────────────────────────────

void MainWindow::buildUi()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_rootLayout = new QVBoxLayout(m_centralWidget);
    m_rootLayout->setContentsMargins(0, 0, 0, 0);
    m_rootLayout->setSpacing(0);

    m_rootLayout->addWidget(buildHeader());

    // ── Detection pane ────────────────────────────────────────────────────────
    m_detectionPane = new QWidget;
    auto *dLayout = new QVBoxLayout(m_detectionPane);
    dLayout->setAlignment(Qt::AlignCenter);

    m_statusLabel = new QLabel("Scanning for GPUs…");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    QFont sf = m_statusLabel->font();
    sf.setPointSize(sf.pointSize() + 2);
    m_statusLabel->setFont(sf);

    m_detectProgress = new QProgressBar;
    m_detectProgress->setRange(0, 0);
    m_detectProgress->setFixedWidth(280);
    m_detectProgress->setFixedHeight(6);
    m_detectProgress->setTextVisible(false);

    dLayout->addStretch();
    dLayout->addWidget(m_statusLabel);
    dLayout->addSpacing(16);
    dLayout->addWidget(m_detectProgress, 0, Qt::AlignCenter);
    dLayout->addStretch();

    m_rootLayout->addWidget(m_detectionPane, 1);

    // ── GPU scroll area ───────────────────────────────────────────────────────
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    m_gpuPane   = new QWidget;
    m_gpuLayout = new QVBoxLayout(m_gpuPane);
    m_gpuLayout->setContentsMargins(24, 18, 24, 18);
    m_gpuLayout->setSpacing(14);
    m_gpuLayout->addStretch();

    m_scrollArea->setWidget(m_gpuPane);
    m_rootLayout->addWidget(m_scrollArea, 1);
    m_scrollArea->hide();

    // ── Log panel ─────────────────────────────────────────────────────────────
    m_logPane = buildLogPanel();
    m_rootLayout->addWidget(m_logPane);
    m_logPane->hide();

    // ── Button bar ────────────────────────────────────────────────────────────
    m_rootLayout->addWidget(buildButtonBar());
}

QWidget *MainWindow::buildHeader()
{
    auto *header = new QFrame;
    header->setObjectName("AppHeader");
    header->setFixedHeight(72);

    const QPalette &pal = QApplication::palette();
    const QColor accent     = pal.color(QPalette::Highlight);
    const QColor accentDark = accent.darker(140);

    header->setStyleSheet(QString(
        "QFrame#AppHeader {"
        "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "    stop:0 %1, stop:1 %2);"
        "}"
    ).arg(accent.name(), accentDark.name()));

    auto *layout = new QHBoxLayout(header);
    layout->setContentsMargins(22, 0, 22, 0);

    QIcon winIcon = QIcon::fromTheme("video-display",
                        QIcon::fromTheme("preferences-desktop-display"));
    auto *iconLabel = new QLabel;
    if (!winIcon.isNull())
        iconLabel->setPixmap(winIcon.pixmap(38, 38));
    else
        iconLabel->setText("🖥");

    auto *title = new QLabel("Vertex Graphics Installer");
    QFont tf = title->font();
    tf.setPointSize(tf.pointSize() + 5);
    tf.setWeight(QFont::DemiBold);
    title->setFont(tf);

    auto *subtitle = new QLabel("Detect and install the best drivers for your hardware");
    QFont stf = subtitle->font();
    stf.setPointSize(stf.pointSize() - 1);
    subtitle->setFont(stf);

    // White text on accent background
    auto makeWhite = [](QWidget *w, int alpha = 255) {
        QPalette p = w->palette();
        p.setColor(QPalette::WindowText, QColor(255, 255, 255, alpha));
        w->setPalette(p);
    };
    makeWhite(iconLabel);
    makeWhite(title);
    makeWhite(subtitle, 200);

    auto *col = new QVBoxLayout;
    col->setSpacing(2);
    col->addWidget(title);
    col->addWidget(subtitle);

    layout->addWidget(iconLabel);
    layout->addSpacing(14);
    layout->addLayout(col);
    layout->addStretch();

    return header;
}

QWidget *MainWindow::buildGpuCard(const GpuInfo &gpu, int index)
{
    const QPalette &pal   = QApplication::palette();
    const QColor base     = pal.color(QPalette::Base);
    const QColor border   = pal.color(QPalette::Mid);
    const QColor dimText  = pal.color(QPalette::PlaceholderText);

    auto *card = new QFrame;
    card->setFrameShape(QFrame::StyledPanel);
    card->setStyleSheet(QString(
        "QFrame {"
        "  background-color: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}"
    ).arg(base.name(), border.name()));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(8);

    // ── Top row: vendor badge + GPU index ─────────────────────────────────────
    auto *topRow = new QHBoxLayout;

    auto *badge = new QLabel(vendorEmoji(gpu.vendor) + "  " + vendorName(gpu.vendor));
    QFont bf = badge->font();
    bf.setWeight(QFont::Bold);
    bf.setPointSize(bf.pointSize() + 1);
    badge->setFont(bf);
    badge->setStyleSheet("color: " + vendorColor(gpu.vendor) + ";");

    auto *idxLabel = new QLabel(QString("GPU %1").arg(index + 1));
    idxLabel->setStyleSheet("color: " + dimText.name() + "; font-size: 11px;");

    topRow->addWidget(badge);
    topRow->addStretch();
    topRow->addWidget(idxLabel);

    // ── Model ─────────────────────────────────────────────────────────────────
    auto *modelLabel = new QLabel(gpu.model);
    modelLabel->setWordWrap(true);
    QFont mf = modelLabel->font();
    mf.setPointSize(mf.pointSize() + 1);
    modelLabel->setFont(mf);

    // ── Driver description ────────────────────────────────────────────────────
    auto *driverRow = new QHBoxLayout;

    QIcon statusIcon = QIcon::fromTheme(
        gpu.needsAur() ? "dialog-warning" : "emblem-default");
    auto *iconLbl = new QLabel;
    if (!statusIcon.isNull())
        iconLbl->setPixmap(statusIcon.pixmap(16, 16));

    auto *driverLbl = new QLabel(gpu.driverLabel);
    QFont df = driverLbl->font();
    df.setItalic(true);
    driverLbl->setFont(df);
    driverLbl->setStyleSheet("color: " + dimText.name() + ";");

    driverRow->addWidget(iconLbl);
    driverRow->addWidget(driverLbl);
    driverRow->addStretch();

    // ── Separator + package list ──────────────────────────────────────────────
    auto *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: " + border.name() + ";");

    QString pkgHtml;
    if (!gpu.pacmanPackages.isEmpty())
        pkgHtml += "<b>pacman:</b> " + gpu.pacmanPackages.join(", ");
    if (!gpu.aurPackages.isEmpty()) {
        if (!pkgHtml.isEmpty()) pkgHtml += "<br>";
        pkgHtml += "<b>AUR &nbsp;&nbsp;:</b> " + gpu.aurPackages.join(", ");
    }
    if (pkgHtml.isEmpty())
        pkgHtml = "<i>No packages determined for this GPU</i>";

    auto *pkgLbl = new QLabel(pkgHtml);
    pkgLbl->setWordWrap(true);
    pkgLbl->setTextFormat(Qt::RichText);
    QFont pf = pkgLbl->font();
    pf.setPointSize(pf.pointSize() - 1);
    pkgLbl->setFont(pf);

    layout->addLayout(topRow);
    layout->addWidget(modelLabel);
    layout->addLayout(driverRow);
    layout->addWidget(sep);
    layout->addWidget(pkgLbl);

    return card;
}

QWidget *MainWindow::buildLogPanel()
{
    auto *pane   = new QWidget;
    auto *layout = new QVBoxLayout(pane);
    layout->setContentsMargins(24, 10, 24, 6);
    layout->setSpacing(6);

    auto *heading = new QLabel("Installation Log");
    QFont hf = heading->font();
    hf.setWeight(QFont::DemiBold);
    heading->setFont(hf);

    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setFixedHeight(210);
    m_logView->setFont(QFont("monospace", 9));
    m_logView->setLineWrapMode(QTextEdit::NoWrap);

    layout->addWidget(heading);
    layout->addWidget(m_logView);
    return pane;
}

QWidget *MainWindow::buildButtonBar()
{
    auto *bar    = new QWidget;
    bar->setFixedHeight(66);
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(24, 10, 24, 10);

    m_aurLabel = new QLabel;
    m_aurLabel->setStyleSheet(
        "color: " + QApplication::palette()
                        .color(QPalette::PlaceholderText).name() + ";");
    layout->addWidget(m_aurLabel);
    layout->addStretch();

    m_installBtn = new QPushButton("  Install Drivers  ");
    m_installBtn->setFixedHeight(40);
    QFont bf = m_installBtn->font();
    bf.setWeight(QFont::DemiBold);
    m_installBtn->setFont(bf);
    m_installBtn->setEnabled(false);
    connect(m_installBtn, &QPushButton::clicked, this, &MainWindow::onInstallClicked);

    layout->addWidget(m_installBtn);
    return bar;
}

// ─── State helpers ────────────────────────────────────────────────────────────

void MainWindow::showDetecting()
{
    m_detectionPane->show();
    m_scrollArea->hide();
    m_logPane->hide();
    m_installBtn->setEnabled(false);
    m_aurLabel->clear();
}

void MainWindow::showGpus(const QList<GpuInfo> &gpus)
{
    m_detectionPane->hide();
    m_scrollArea->show();

    // Remove old cards (leave the trailing stretch)
    while (m_gpuLayout->count() > 1)
        delete m_gpuLayout->takeAt(0)->widget();

    for (int i = 0; i < gpus.size(); ++i)
        m_gpuLayout->insertWidget(i, buildGpuCard(gpus[i], i));
}

void MainWindow::showInstalling()
{
    m_logPane->show();
    m_logView->clear();
    m_installBtn->setEnabled(false);
}

// ─── Slots ────────────────────────────────────────────────────────────────────

void MainWindow::onDetectFinished(const QList<GpuInfo> &gpus, const QString &aurHelper)
{
    m_gpus      = gpus;
    m_aurHelper = aurHelper;

    showGpus(gpus);

    bool allUnknown = std::all_of(gpus.begin(), gpus.end(),
        [](const GpuInfo &g){ return g.isUnknown(); });

    m_installBtn->setEnabled(!allUnknown);

    if (aurHelper.isEmpty())
        m_aurLabel->setText("⚠  vpkg not found – AUR drivers won't be installable.");
    else
        m_aurLabel->setText("Package manager: vpkg");
}

void MainWindow::onInstallLog(const QString &line)
{
    m_logView->append(line);
    m_logView->moveCursor(QTextCursor::End);
}

void MainWindow::onInstallFinished(bool success, const QString &message)
{
    if (success) {
        auto btn = QMessageBox::information(this, "Installation Complete",
            message + "\n\nWould you like to reboot now?",
            QMessageBox::Yes | QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            QProcess::startDetached("systemctl", {"reboot"});
            QApplication::quit();
        }
    } else {
        QMessageBox::critical(this, "Installation Failed", message);
        m_installBtn->setEnabled(true);
    }
}

void MainWindow::onInstallClicked()
{
    QStringList allPacman, allAur;
    for (const GpuInfo &gpu : m_gpus) {
        for (const QString &p : gpu.pacmanPackages)
            if (!allPacman.contains(p)) allPacman.append(p);
        for (const QString &p : gpu.aurPackages)
            if (!allAur.contains(p)) allAur.append(p);
    }

    if (!allAur.isEmpty() && m_aurHelper.isEmpty()) {
        QMessageBox::warning(this, "vpkg Not Found",
            "An AUR package is required for your GPU driver but vpkg\n"
            "was not found on this system.\n\n"
            "Install vpkg and restart the application.");
        return;
    }

    showInstalling();

    auto *worker = new InstallWorker(allPacman, allAur, m_aurHelper, this);
    connect(worker, &InstallWorker::logLine,  this, &MainWindow::onInstallLog);
    connect(worker, &InstallWorker::finished, this, &MainWindow::onInstallFinished);
    connect(worker, &InstallWorker::finished, worker, &QObject::deleteLater);
    worker->start();
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

QString MainWindow::vendorEmoji(GpuVendor v) const
{
    switch (v) {
    case GpuVendor::NVIDIA: return "🟢";
    case GpuVendor::AMD:    return "🔴";
    case GpuVendor::Intel:  return "🔵";
    default:                return "⬜";
    }
}

QString MainWindow::vendorName(GpuVendor v) const
{
    switch (v) {
    case GpuVendor::NVIDIA: return "NVIDIA";
    case GpuVendor::AMD:    return "AMD";
    case GpuVendor::Intel:  return "Intel";
    default:                return "Unknown";
    }
}

QString MainWindow::vendorColor(GpuVendor v) const
{
    switch (v) {
    case GpuVendor::NVIDIA: return "#76b900";
    case GpuVendor::AMD:    return "#ed1c24";
    case GpuVendor::Intel:  return "#0071c5";
    default:
        return QApplication::palette().color(QPalette::Highlight).name();
    }
}
