#pragma once
#include <optional>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

/**
 * Synchronous (blocking) QRZ XML API client.
 *
 * Must be used from a non-GUI thread.  Creates its own
 * QNetworkAccessManager and uses a local QEventLoop to block
 * until each HTTP response arrives — no callbacks required.
 */
class QRZClient : public QObject
{
    Q_OBJECT
public:
    explicit QRZClient(const QString& user, const QString& pass,
                       QObject* parent = nullptr);

    /// Returns the Maidenhead grid locator for a callsign, or nullopt.
    std::optional<QString> lookupGrid(const QString& callsign);

private:
    bool ensureSession();
    QString httpGet(const QString& url);

    QString user_{}, pass_{}, sessionKey_{};
    QNetworkAccessManager nam_{};
};
