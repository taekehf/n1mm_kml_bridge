#pragma once
#include "contact.h"
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QString>

class KMLWriter : public QObject {
    Q_OBJECT
public:
    /// lookAtRangeKm: camera altitude above the DX location in kilometres.
    /// 200 km gives a regional view; 50 km zooms to city level.
    /// Override at runtime with the LOOKAT_RANGE_KM environment variable.
    explicit KMLWriter(const QString& path,
                       double homeLat, double homeLon,
                       const QString& homeCall,
                       double lookAtRangeKm = 200.0,
                       QObject* parent = nullptr);

    /// Thread-safe rewrite. lastContact drives the camera position.
    void write(const QMap<QString, Contact>& contacts, const Contact& lastContact);

private:
    static QString esc(const QString& s);

    QString path_{};
    double  homeLat_{}, homeLon_{};
    QString homeCall_{};
    double  lookAtRangeM_{};   ///< pre-converted to metres
    QMutex  mu_{};
};
