#pragma once

#include "backend/common/ApiTypes.h"

#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QObject>

namespace courierman::network {

class RestClient : public QObject {
    Q_OBJECT

public:
    explicit RestClient(QObject* parent = nullptr);

public slots:
    void send(const backend::RequestDefinition& request);

signals:
    void requestStarted();
    void responseReady(const backend::ResponseEnvelope& response);
    void requestFailed(const QString& message);

private:
    QNetworkAccessManager m_manager;
};

}  // namespace courierman::network
