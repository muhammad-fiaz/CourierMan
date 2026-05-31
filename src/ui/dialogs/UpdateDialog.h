#pragma once

#include <QDialog>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QProgressBar>
#include <QPushButton>
#include <QUrl>

namespace courierman::ui {

class UpdateDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget* parent = nullptr);
    void startCheck();

private:
    void beginDownload(const QUrl& url);
    void setBusy(const QString& message);
    void setMessage(const QString& message);

    QNetworkAccessManager m_network;
    QLabel* m_message{nullptr};
    QProgressBar* m_progress{nullptr};
    QPushButton* m_primaryAction{nullptr};
    QPushButton* m_closeAction{nullptr};
    QUrl m_downloadUrl;
};

}  // namespace courierman::ui
