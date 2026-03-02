#pragma once
#include "config.h"
#include "contact.h"
#include "blockingqueue.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QUdpSocket>
#include <QString>
#include <atomic>
#include <thread>
#include <utility>   // std::pair

class KMLWriter;

class Bridge : public QObject
{
    Q_OBJECT
public:
    explicit Bridge(Config cfg, QObject* parent = nullptr);
    ~Bridge() override;

    bool start();

private slots:
    void onDatagramReady();

private:
    void workerLoop();
    static QString xmlExtract(const QString& xml, const QString& tag);
    static QString n1mmField (const QString& xml, const QString& tag);

    Config   cfg_;
    QUdpSocket socket_;

    QMap<QString, Contact> contacts_;
    QMutex                 contactsMu_;

    KMLWriter*  kml_    = nullptr;

    using WorkItem = std::pair<QString /*call*/, QString /*raw_xml*/>;
    BlockingQueue<WorkItem> queue_;

    std::thread         worker_;
    std::atomic<bool>   running_{false};
};
