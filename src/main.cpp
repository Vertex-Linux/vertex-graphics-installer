#include "MainWindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Vertex Graphics Installer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("arch-gpu-installer");

    // Use Breeze if available (KDE), otherwise falls back to the platform default.
    // QT_QPA_PLATFORMTHEME=kde is already set by Plasma, so the palette is
    // automatically the user's active KDE theme.
    app.setStyle("breeze");

    app.setWindowIcon(QIcon::fromTheme("video-display",
                          QIcon::fromTheme("preferences-desktop-display")));

    MainWindow w;
    w.show();
    return app.exec();
}
