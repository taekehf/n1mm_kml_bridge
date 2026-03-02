#include "bridge.h"
#include "kmlwriter.h"
#include "qrzclient.h"
#include "maidenhead.h"

#include <QHostAddress>
#include <QMutexLocker>
#include <QDebug>

// ── XML helper ────────────────────────────────────────────────────────────────
QString Bridge::xmlExtract(const QString& xml, const QString& tag)
{
    const QString open  = "<" + tag + ">";
    const QString close = "</" + tag + ">";
    int s = xml.indexOf(open, 0, Qt::CaseInsensitive);
    if (s < 0) return {};
    s += open.size();
    int e = xml.indexOf(close, s, Qt::CaseInsensitive);
    if (e < 0) return {};
    return xml.mid(s, e - s).trimmed();
}

QString Bridge::n1mmField(const QString& xml, const QString& tag)
{
    return xmlExtract(xml, tag);
}

// ── Construction ──────────────────────────────────────────────────────────────
Bridge::Bridge(Config cfg, QObject* parent)
    : QObject(parent), cfg_(std::move(cfg))
{
    kml_ = new KMLWriter(cfg_.kml_path, cfg_.home_lat, cfg_.home_lon,
                         cfg_.home_call, cfg_.lookat_range_km, this);
}

Bridge::~Bridge()
{
    running_ = false;
    queue_.done();
    if (worker_.joinable()) worker_.join();
    socket_.close();
}

// ── Start ─────────────────────────────────────────────────────────────────────
bool Bridge::start()
{
    // Bind UDP socket
    if (!socket_.bind(QHostAddress::AnyIPv4, cfg_.udp_port,
                      QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint)) {
        qCritical() << "[UDP] Cannot bind port" << cfg_.udp_port
                    << ":" << socket_.errorString();
        return false;
    }
    connect(&socket_, &QUdpSocket::readyRead, this, &Bridge::onDatagramReady);
    qInfo() << "[Bridge] Listening for N1MM on UDP port" << cfg_.udp_port;
    qInfo() << "[Bridge] KML output:" << cfg_.kml_path;

    // Launch worker thread
    running_ = true;
    worker_ = std::thread(&Bridge::workerLoop, this);
    return true;
}

// ── UDP receive slot (runs on Qt main thread) ─────────────────────────────────
void Bridge::onDatagramReady()
{
    while (socket_.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(static_cast<int>(socket_.pendingDatagramSize()));
        socket_.readDatagram(data.data(), data.size());

        const QString xml = QString::fromUtf8(data);

        // N1MM sends several message types; we only care about <contactinfo>
        if (!xml.contains("<contactinfo", Qt::CaseInsensitive)) continue;

        QString call = n1mmField(xml, "call");
        if (call.isEmpty()) continue;
        call = call.toUpper();

        qInfo() << "[UDP] Contact received:" << call;
        queue_.push({call, xml});
    }
}

// ── Worker thread: QRZ lookups + KML rewrite ─────────────────────────────────
void Bridge::workerLoop()
{
    // QNetworkAccessManager must live in the thread that uses it,
    // so we construct QRZClient here inside the worker thread.
    QRZClient qrz(cfg_.qrz_user, cfg_.qrz_pass);

    while (running_) {
        try {
            auto [call, rawXml] = queue_.pop();

            Contact c;
            c.callsign  = call;
            c.band      = n1mmField(rawXml, "Band");
            c.mode      = n1mmField(rawXml, "Mode");
            c.timestamp = n1mmField(rawXml, "ContactTimestamp");

            // QRZ grid lookup
            auto grid = qrz.lookupGrid(call);
            if (!grid || grid->isEmpty()) {
                qWarning() << "[Bridge] No grid for" << call << "— skipping";
                continue;
            }
            c.grid = *grid;

            // Maidenhead → lat/lon
            auto ll = gridToLatLon(c.grid);
            if (!ll) {
                qWarning() << "[Bridge] Cannot parse grid" << c.grid << "for" << call;
                continue;
            }
            c.lat = ll->lat;
            c.lon = ll->lon;

            qInfo() << "[Bridge]" << call << "→" << c.grid
                    << QString("(%1, %2)").arg(c.lat, 0, 'f', 4).arg(c.lon, 0, 'f', 4);

            // Store and write KML
            {
                QMutexLocker lock(&contactsMu_);
                contacts_[call] = c;
            }

            // Read a snapshot for the writer (avoids holding the lock during I/O)
            QMap<QString, Contact> snapshot;
            {
                QMutexLocker lock(&contactsMu_);
                snapshot = contacts_;
            }
            kml_->write(snapshot, c);

        } catch (const std::exception&) {
            // queue_.done() was called — time to exit
            break;
        }
    }
}
