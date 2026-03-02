#include "qrzclient.h"

#include <QEventLoop>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>

// ── Tiny XML helper ───────────────────────────────────────────────────────────
static std::optional<QString> xmlExtract(const QString& xml, const QString& tag)
{
    const QString open  = "<" + tag + ">";
    const QString close = "</" + tag + ">";
    int s = xml.indexOf(open);
    if (s < 0) return std::nullopt;
    s += open.size();
    int e = xml.indexOf(close, s);
    if (e < 0) return std::nullopt;
    return xml.mid(s, e - s);
}

// ─────────────────────────────────────────────────────────────────────────────
QRZClient::QRZClient(const QString& user, const QString& pass, QObject* parent)
    : QObject(parent), user_(user), pass_(pass)
{}

// ── Blocking HTTP GET using a local event loop ────────────────────────────────
QString QRZClient::httpGet(const QString& url)
{
    QNetworkRequest req{QUrl(url)};
    req.setTransferTimeout(10'000);   // 10 s

    QNetworkReply* reply = nam_.get(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // Safety timeout in case the signal never fires
    QTimer::singleShot(12'000, &loop, &QEventLoop::quit);
    loop.exec();

    QString body;
    if (reply->error() == QNetworkReply::NoError) {
        body = QString::fromUtf8(reply->readAll());
    } else {
        qWarning() << "[QRZ] HTTP error:" << reply->errorString();
    }
    reply->deleteLater();
    return body;
}

// ── QRZ session management ────────────────────────────────────────────────────
bool QRZClient::ensureSession()
{
    if (!sessionKey_.isEmpty()) return true;

    const QString url = QString(
        "https://xmldata.qrz.com/xml/current/?username=%1;password=%2;agent=n1mm_kml_bridge"
    ).arg(user_, pass_);

    const QString body = httpGet(url);
    if (body.isEmpty()) return false;

    auto key = xmlExtract(body, "Key");
    if (!key) {
        auto err = xmlExtract(body, "Error");
        qWarning() << "[QRZ] Login failed:" << err.value_or("unknown error");
        return false;
    }
    sessionKey_ = *key;
    qInfo() << "[QRZ] Session established.";
    return true;
}

// ── Public lookup ─────────────────────────────────────────────────────────────
std::optional<QString> QRZClient::lookupGrid(const QString& callsign)
{
    if (!ensureSession()) return std::nullopt;

    auto doLookup = [&]() -> QString {
        const QString url = QString(
            "https://xmldata.qrz.com/xml/current/?s=%1;callsign=%2"
        ).arg(sessionKey_, callsign);
        return httpGet(url);
    };

    QString body = doLookup();
    if (body.isEmpty()) return std::nullopt;

    // Handle session expiry
    if (auto err = xmlExtract(body, "Error"); err) {
        if (err->contains("Session Timeout") || err->contains("Invalid session")) {
            sessionKey_.clear();
            if (!ensureSession()) return std::nullopt;
            body = doLookup();
        } else {
            qWarning() << "[QRZ] Lookup error for" << callsign << ":" << *err;
            return std::nullopt;
        }
    }

    auto grid = xmlExtract(body, "grid");
    if (!grid || grid->isEmpty()) {
        qWarning() << "[QRZ] No grid in response for" << callsign;
        return std::nullopt;
    }
    return grid;
}
