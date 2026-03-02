#pragma once
#include <QString>

struct Config {
    QString  qrz_user{};
    QString  qrz_pass{};
    quint16  udp_port{};
    QString  kml_path{};
    QString  home_call{};
    double   home_lat{};
    double   home_lon{};
    int      lookat_range_km{};
};

