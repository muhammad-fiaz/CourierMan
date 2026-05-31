#include "ui/dialogs/UpdateDialog.h"

#include <nlohmann/json.hpp>

#include <QCryptographicHash>
#include <QDesktopServices>
#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QVBoxLayout>

#include <fstream>

namespace courierman::ui {
namespace {

[[nodiscard]] QString currentVersion() {
#ifdef COURIERMAN_VERSION
    return QString::fromLatin1(COURIERMAN_VERSION);
#else
    return QStringLiteral("1.0.0");
#endif
}

[[nodiscard]] QString normalizeTag(QString tag) {
    if (tag.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        tag.remove(0, 1);
    }
    return tag;
}

}  // namespace

UpdateDialog::UpdateDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("CourierMan Update"));
    setModal(true);
    resize(480, 180);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    m_message = new QLabel(QStringLiteral("Checking for updates..."), this);
    m_message->setWordWrap(true);
    layout->addWidget(m_message);

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 0);
    layout->addWidget(m_progress);

    auto* actions = new QHBoxLayout();
    actions->addStretch(1);
    m_primaryAction = new QPushButton(QStringLiteral("Download"), this);
    m_primaryAction->setObjectName(QStringLiteral("primaryButton"));
    m_primaryAction->setEnabled(false);
    m_closeAction = new QPushButton(QStringLiteral("Close"), this);
    connect(m_closeAction, &QPushButton::clicked, this, &QDialog::accept);
    actions->addWidget(m_primaryAction);
    actions->addWidget(m_closeAction);
    layout->addLayout(actions);
}

void UpdateDialog::startCheck() {
    setBusy(QStringLiteral("Checking GitHub Releases for the latest CourierMan build..."));
    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/muhammad-fiaz/CourierMan/releases/latest")));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("CourierMan-Updater"));
    auto* reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray payload = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            setMessage(QStringLiteral("Update check failed: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        try {
            const auto json = nlohmann::json::parse(payload.toStdString());
            const QString latestTag = QString::fromStdString(json.value("tag_name", "v1.0.0"));
            const QString htmlUrl = QString::fromStdString(json.value("html_url", ""));
            if (normalizeTag(latestTag) <= normalizeTag(currentVersion())) {
                setMessage(QStringLiteral("CourierMan %1 is up to date.").arg(currentVersion()));
                reply->deleteLater();
                return;
            }

            if (json.contains("assets") && json["assets"].is_array() && !json["assets"].empty()) {
                const auto& asset = json["assets"].front();
                m_downloadUrl = QUrl(QString::fromStdString(asset.value("browser_download_url", "")));
            } else {
                m_downloadUrl = QUrl(htmlUrl);
            }

            setMessage(QStringLiteral("CourierMan %1 is available.").arg(latestTag));
            m_primaryAction->setEnabled(m_downloadUrl.isValid());
            disconnect(m_primaryAction, nullptr, nullptr, nullptr);
            connect(m_primaryAction, &QPushButton::clicked, this, [this]() {
                if (m_downloadUrl.fileName().isEmpty()) {
                    QDesktopServices::openUrl(m_downloadUrl);
                } else {
                    beginDownload(m_downloadUrl);
                }
            });
        } catch (const std::exception& error) {
            setMessage(QStringLiteral("Unable to parse release metadata: %1").arg(QString::fromUtf8(error.what())));
        }
        reply->deleteLater();
    });
}

void UpdateDialog::beginDownload(const QUrl& url) {
    setBusy(QStringLiteral("Downloading %1...").arg(url.fileName()));
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
    m_primaryAction->setEnabled(false);

    auto* reply = m_network.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            m_progress->setValue(static_cast<int>((received * 100) / total));
        }
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        if (reply->error() != QNetworkReply::NoError) {
            setMessage(QStringLiteral("Download failed: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        const QString downloads = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        const QString destination = QDir(downloads).filePath(url.fileName());
        std::ofstream output(destination.toStdString(), std::ios::binary | std::ios::trunc);
        const QByteArray bytes = reply->readAll();
        output.write(bytes.constData(), bytes.size());
        output.close();
        const QString sha256 =
            QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
        setMessage(QStringLiteral("Downloaded update to %1\nSHA-256: %2").arg(destination, sha256));
        m_progress->setValue(100);
        reply->deleteLater();
    });
}

void UpdateDialog::setBusy(const QString& message) {
    m_message->setText(message);
    m_progress->setRange(0, 0);
    m_primaryAction->setEnabled(false);
}

void UpdateDialog::setMessage(const QString& message) {
    m_message->setText(message);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);
}

}  // namespace courierman::ui
