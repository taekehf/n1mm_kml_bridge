#pragma once

#include <QSettings>
#include <QString>
#include <QCoreApplication>
#include <QFile>
#include "config.h"
#include <QTextStream>


class Loadconfig
{
public:
    Loadconfig(Config &);

private:
    void loadSettings(Config &, const QSettings &);
    void saveSettings(QSettings &);
    static QString promptLine(const char*, bool);


    const QString m_sSettingsFile{QCoreApplication::applicationDirPath() + "/config.ini"};
    QSettings settings(QString, QSettings::Format);
    QString textIn{};
    double dblIn{};
    int intIn{};
    bool valid{false};

};

