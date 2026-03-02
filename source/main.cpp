//
// N1MM → QRZ → Google Earth KML Bridge
// Qt6 / C++23, Windows 11, Qt Creator
//
// No external dependencies beyond Qt (QtCore + QtNetwork).
//

#include "config.h"
#include "bridge.h"
#include "loadconfig.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <iostream>

// ── Redirect Qt logging to the console ───────────────────────────────────────
// In Release builds on Windows, Qt routes qInfo/qWarning through
// OutputDebugString (visible only in a debugger). This handler sends
// everything to stdout/stderr instead so it appears in the command prompt.
static void consoleMessageHandler(QtMsgType type,
                                  const QMessageLogContext& /*ctx*/,
                                  const QString& msg)
{
    const QByteArray utf8 = msg.toUtf8();
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
        fprintf(stdout, "%s\n", utf8.constData());
        fflush(stdout);
        break;
    case QtWarningMsg:
    case QtCriticalMsg:
    case QtFatalMsg:
        fprintf(stderr, "%s\n", utf8.constData());
        fflush(stderr);
        break;
    }
}

int main(int argc, char* argv[])
{
    // redirect text to terminal window
    qInstallMessageHandler(consoleMessageHandler);

    QCoreApplication app(argc, argv);
    app.setApplicationName("N1MM KML GE Bridge");
    app.setApplicationVersion("0.1");

    std::cout << "Program starting...";

    Config cfg{};

    Loadconfig lc(cfg);

    qInfo() << "=== N1MM KML Bridge ===";
    qInfo() << "  UDP port :" << cfg.udp_port;
    qInfo() << "  KML file :" << cfg.kml_path;
    qInfo() << "  Home     :" << cfg.home_call
            << QString("(%1, %2)").arg(cfg.home_lat).arg(cfg.home_lon);
    qInfo() << "Press Ctrl+C to stop.";

    // ── Create and start bridge ───────────────────────────────────────────────
    Bridge bridge(cfg);
    if (!bridge.start()) {
        qCritical() << "Failed to start bridge. Exiting.";
        return 1;
    }

    return app.exec();
}
