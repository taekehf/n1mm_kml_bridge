#include "loadconfig.h"

Loadconfig::Loadconfig(Config &cfg) {

    QSettings settings(m_sSettingsFile, QSettings::IniFormat);

    //see if config files exists then read, else create new one, then load it.
    if (settings.allKeys().length() != 0)
        loadSettings(cfg, settings);
    else
    { saveSettings(settings); loadSettings(cfg, settings); }
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/// Read one line from stdin.
/// Pass echo=false to suppress character echo (e.g. for passwords).
/// Returns an empty QString if the user just presses Enter.
QString  Loadconfig::promptLine(const char* prompt, bool echo = true)
{
    QTextStream con(stdout);
    con << prompt << Qt::flush;

#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD  mode   = 0;
    GetConsoleMode(hStdin, &mode);

    if (!echo)
        SetConsoleMode(hStdin, mode & ~ENABLE_ECHO_INPUT);
#endif

    char buf[256] = {};
    fgets(buf, sizeof(buf), stdin);

#ifdef _WIN32
    // Always restore the original mode
    SetConsoleMode(hStdin, mode);

    // When echo is off, the Enter key doesn't print a newline visually,
    // so we add one ourselves to keep the console output tidy.
    if (!echo)
        con << "\n" << Qt::flush;
#endif

    return QString::fromUtf8(buf).trimmed();
}

void Loadconfig::loadSettings(Config &cfg, const QSettings &settings) {
    qInfo() << "Loading settings";
    cfg.home_call = settings.value("Homecall", "no/call").toString();
    cfg.home_lat = settings.value("HomeLat", 0.0).toDouble();
    cfg.home_lon = settings.value("HomeLong", 0.0).toDouble();
    cfg.lookat_range_km = settings.value("Range").toInt();
    cfg.qrz_user = settings.value("QRZUser", "no/user").toString();
    if (settings.value("QRZPass").toString() == "")
        cfg.qrz_pass = promptLine("Enter your QRZ password: ", false);
    else
        cfg.qrz_pass =  settings.value("QRZPass").toString();
    if (settings.value("UDPport").toInt() == 0)
        cfg.udp_port = 12060;
    else
        cfg.udp_port = settings.value("UDPport").toInt();
    if (settings.value("KMLfile").toString() == "")
        cfg.kml_path = "n1mm_contacts.kml";
    else
        cfg.kml_path =  settings.value("KMLfile").toString();
}

void Loadconfig::saveSettings(QSettings &settings) {
    qInfo() << "Create and save settings";

    QTextStream in(stdin);
    QTextStream out(stdout);

    settings.setValue("Homecall", promptLine("Enter your callsign: ", true));
    settings.setValue("HomeLat", promptLine("Enter your latitude dd.mmm (N+ S-) : ", true).toDouble());
    settings.setValue("HomeLong", promptLine("Enter your longitude dd.mmm (E+ W-) : ", true).toDouble());
    out << "Enter viewing range (200 = regional, 50 = city level, 5 = street level) : "; out.flush();
    while(!valid)
    {
        switch (in >> intIn; intIn)
        {
        case 200: case 50: case 5:
            settings.setValue("Range", intIn);
            valid = true;
            break;
        default:
            out << "Invalid input. Please try again."; out.flush();
            break;
        }
    }
    settings.setValue("QRZUser", promptLine("Enter your QRZ username: ", true));
    settings.setValue("QRZPass", promptLine("Enter your QRZ password (stored non-encrypted, leave blank to always enter on start-up): ", true));
    settings.setValue("UDPport", promptLine("Enter N1MM UDP-port number (enter for default 12060): ", true).toInt());
    settings.setValue("KMLfile", promptLine("Enter the location of the KML-file for Google Earth (enter to create the file in the program directory): ", true));
}
