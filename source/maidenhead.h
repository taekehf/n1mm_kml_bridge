#pragma once
#include <optional>
#include <QString>
#include <cctype>

struct LatLon { double lat, lon; };

/// Convert a Maidenhead grid locator (4 or 6 characters) to the centre lat/lon.
inline std::optional<LatLon> gridToLatLon(const QString& grid)
{
    if (grid.size() < 4) return std::nullopt;

    const QChar f0 = grid[0].toUpper();
    const QChar f1 = grid[1].toUpper();
    if (f0 < 'A' || f0 > 'R' || f1 < 'A' || f1 > 'R') return std::nullopt;

    double lon = (f0.unicode() - 'A') * 20.0 - 180.0;
    double lat = (f1.unicode() - 'A') * 10.0 -  90.0;

    if (!grid[2].isDigit() || !grid[3].isDigit()) return std::nullopt;
    lon += (grid[2].unicode() - '0') * 2.0;
    lat += (grid[3].unicode() - '0') * 1.0;

    if (grid.size() >= 6) {
        const QChar s0 = grid[4].toUpper();
        const QChar s1 = grid[5].toUpper();
        lon += (s0.unicode() - 'A') * (2.0 / 24.0) + (1.0 / 24.0);   // centre
        lat += (s1.unicode() - 'A') * (1.0 / 24.0) + (0.5 / 24.0);
    } else {
        lon += 1.0;   // centre of 2° square
        lat += 0.5;
    }

    return LatLon{lat, lon};
}
