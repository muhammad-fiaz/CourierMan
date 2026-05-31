#pragma once

#include <QByteArray>
#include <QList>
#include <QString>

namespace courierman::backend {

struct KeyValue {
    QString key;
    QString value;
    bool enabled{true};
};

struct RequestDefinition {
    QString id;
    QString name;
    QString method{"GET"};
    QString url{"https://httpbin.org/get"};
    QList<KeyValue> headers;
    QList<KeyValue> queryParameters;
    QByteArray body;
    QString bodyType{"json"};
    QString authType{"none"};
};

struct ResponseEnvelope {
    int statusCode{0};
    QString reason;
    QByteArray body;
    QList<KeyValue> headers;
    qint64 elapsedMs{0};
    QString error;
};

}  // namespace courierman::backend
