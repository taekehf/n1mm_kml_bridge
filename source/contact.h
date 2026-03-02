#pragma once
#include <QString>

struct Contact {
    QString callsign{};
    QString band{};
    QString mode{};
    QString grid{};
    double  lat{};
    double  lon{};
    QString timestamp{};   // as received from N1MM
};
