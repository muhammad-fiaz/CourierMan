#include "network/rest/RestClient.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

namespace courierman::network {

RestClient::RestClient(QObject* parent)
    : QObject(parent) {}

void RestClient::send(const backend::RequestDefinition& request) {
    QUrl url(request.url);
    QUrlQuery query(url);
    for (const auto& pair : request.queryParameters) {
        if (pair.enabled && !pair.key.trimmed().isEmpty()) {
            query.addQueryItem(pair.key, pair.value);
        }
    }
    url.setQuery(query);

    if (!url.isValid() || url.scheme().isEmpty()) {
        emit requestFailed(QStringLiteral("Invalid URL: %1").arg(request.url));
        return;
    }

    QNetworkRequest networkRequest(url);
    for (const auto& header : request.headers) {
        if (header.enabled && !header.key.trimmed().isEmpty()) {
            networkRequest.setRawHeader(header.key.toUtf8(), header.value.toUtf8());
        }
    }

    auto* timer = new QElapsedTimer();
    timer->start();
    emit requestStarted();

    const QByteArray verb = request.method.toUpper().toUtf8();
    QNetworkReply* reply = nullptr;
    if (verb == "GET") {
        reply = m_manager.get(networkRequest);
    } else if (verb == "HEAD") {
        reply = m_manager.head(networkRequest);
    } else if (verb == "DELETE") {
        reply = m_manager.deleteResource(networkRequest);
    } else {
        reply = m_manager.sendCustomRequest(networkRequest, verb, request.body);
    }

    connect(reply, &QNetworkReply::finished, this, [this, reply, timer]() {
        backend::ResponseEnvelope envelope;
        envelope.elapsedMs = timer->elapsed();
        envelope.statusCode =
            reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        envelope.reason =
            reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        envelope.body = reply->readAll();
        for (const auto& header : reply->rawHeaderPairs()) {
            envelope.headers.append(
                backend::KeyValue{QString::fromUtf8(header.first), QString::fromUtf8(header.second), true});
        }
        if (reply->error() != QNetworkReply::NoError) {
            envelope.error = reply->errorString();
            emit requestFailed(envelope.error);
        }
        emit responseReady(envelope);
        reply->deleteLater();
        delete timer;
    });
}

}  // namespace courierman::network
