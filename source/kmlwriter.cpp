#include "kmlwriter.h"

#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QMutexLocker>
#include <QDebug>
#include <cmath>
#include <algorithm>

KMLWriter::KMLWriter(const QString& path,
                     double homeLat, double homeLon,
                     const QString& homeCall,
                     double lookAtRangeKm,
                     QObject* parent)
    : QObject(parent)
    , path_(path)
    , homeLat_(homeLat), homeLon_(homeLon)
    , homeCall_(homeCall)
    , lookAtRangeM_(lookAtRangeKm * 1000.0)
{}

void KMLWriter::write(const QMap<QString, Contact>& contacts, const Contact& last)
{
    QMutexLocker lock(&mu_);

    QFile file(path_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "[KML] Cannot open" << path_ << "for writing:" << file.errorString();
        return;
    }

    QTextStream ts(&file);
    ts.setEncoding(QStringConverter::Utf8);

    ts << R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
  <name>N1MM Contacts</name>
  <description>Live contacts from N1MM Logger+</description>

  <!-- Camera: zoom into the DX location of the last logged contact -->
  <LookAt>
    <longitude>)" << last.lon << R"(</longitude>
    <latitude>)"  << last.lat << R"(</latitude>
    <altitude>0</altitude>
    <heading>0</heading>
    <tilt>0</tilt>
    <range>)" << lookAtRangeM_ << R"(</range>
    <altitudeMode>relativeToGround</altitudeMode>
  </LookAt>

  <Style id="homeStyle">
    <IconStyle>
      <color>ff00ffff</color><scale>1.4</scale>
      <Icon><href>http://maps.google.com/mapfiles/kml/paddle/ylw-stars.png</href></Icon>
    </IconStyle>
    <LabelStyle><color>ff00ffff</color><scale>1.2</scale></LabelStyle>
  </Style>

  <Style id="contactStyle">
    <IconStyle>
      <color>ff0000ff</color><scale>1.0</scale>
      <Icon><href>http://maps.google.com/mapfiles/kml/paddle/red-circle.png</href></Icon>
    </IconStyle>
    <LabelStyle><color>ffffffff</color><scale>1.0</scale></LabelStyle>
    <LineStyle><color>80ff0000</color><width>1</width></LineStyle>
  </Style>

  <!-- Home station -->
  <Placemark>
    <name>)" << esc(homeCall_) << R"(</name>
    <styleUrl>#homeStyle</styleUrl>
    <Point>
      <coordinates>)" << homeLon_ << "," << homeLat_ << R"(,0</coordinates>
    </Point>
  </Placemark>
)";

    for (const Contact& c : contacts) {
        ts << "  <Placemark>\n";
        ts << "    <name>" << esc(c.callsign) << "</name>\n";
        ts << "    <description><![CDATA["
           << "<b>" << esc(c.callsign) << "</b><br/>"
           << "Band: " << c.band << "<br/>"
           << "Mode: " << c.mode << "<br/>"
           << "Grid: " << c.grid << "<br/>"
           << "Time: " << c.timestamp
           << "]]></description>\n";
        ts << "    <styleUrl>#contactStyle</styleUrl>\n";
        ts << "    <MultiGeometry>\n";
        ts << "      <Point>\n"
           << "        <coordinates>" << c.lon << "," << c.lat << ",0</coordinates>\n"
           << "      </Point>\n";
        ts << "      <LineString>\n"
           << "        <tessellate>1</tessellate>\n"
           << "        <coordinates>"
           << homeLon_ << "," << homeLat_ << ",0 "
           << c.lon    << "," << c.lat    << ",0"
           << "</coordinates>\n"
           << "      </LineString>\n";
        ts << "    </MultiGeometry>\n";
        ts << "  </Placemark>\n";
    }

    ts << "</Document>\n</kml>\n";
    qInfo() << "[KML] Written" << contacts.size() << "contacts to" << path_
            << "| LookAt:" << last.callsign
            << last.grid
            << QString("range %1 km").arg(lookAtRangeM_ / 1000.0, 0, 'f', 0);
}

QString KMLWriter::esc(const QString& s)
{
    QString r;
    r.reserve(s.size());
    for (QChar ch : s) {
        if      (ch == '&') r += "&amp;";
        else if (ch == '<') r += "&lt;";
        else if (ch == '>') r += "&gt;";
        else                r += ch;
    }
    return r;
}
