#pragma once

#include "backend/common/ApiTypes.h"
#include "core/ConfigManager.h"
#include "network/rest/RestClient.h"
#include "ui/main/TitleBar.h"
#include "ui/settings/SettingsView.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QSplitter>
#include <QStackedWidget>
#include <QSystemTrayIcon>
#include <QTableWidget>
#include <QTabWidget>

namespace courierman::ui {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(core::ConfigManager* configManager,
               core::AppSettings settings,
               QWidget* parent = nullptr);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void sendRequest();
    void handleRequestStarted();
    void handleResponse(const backend::ResponseEnvelope& response);
    void handleRequestFailed(const QString& message);
    void showWorkspace();
    void showSettings();
    void showUpdateDialog();
    void openWebsite();
    void openReleaseNotes();
    void importRequest();
    void exportRequest();
    void forceQuit();

private:
    void buildUi();
    [[nodiscard]] QWidget* buildWorkspacePage();
    [[nodiscard]] QWidget* buildLeftSidebar();
    [[nodiscard]] QWidget* buildRequestEditor();
    [[nodiscard]] QWidget* buildResponseViewer();
    [[nodiscard]] QWidget* buildRightSidebar();
    [[nodiscard]] QWidget* buildBottomBar();
    [[nodiscard]] QWidget* buildRunnerPage();
    void initTray();
    void connectSignals();
    void appendConsole(const QString& line);
    void updateCodeSnippet();
    void updateDocumentation();
    void showProgress(const QString& title, const QString& message);
    void hideProgress();
    void setPanelVisible(QWidget* panel, bool visible);
    [[nodiscard]] backend::RequestDefinition collectRequest() const;

    core::ConfigManager* m_configManager;
    core::AppSettings m_settings;
    network::RestClient* m_restClient{nullptr};
    TitleBar* m_titleBar{nullptr};
    QStackedWidget* m_stack{nullptr};
    SettingsView* m_settingsView{nullptr};
    QWidget* m_leftPanel{nullptr};
    QWidget* m_rightPanel{nullptr};
    QWidget* m_bottomPanel{nullptr};
    QComboBox* m_method{nullptr};
    QLineEdit* m_url{nullptr};
    QTableWidget* m_params{nullptr};
    QTableWidget* m_headers{nullptr};
    QPlainTextEdit* m_body{nullptr};
    QPlainTextEdit* m_scriptEditor{nullptr};
    QPlainTextEdit* m_responsePretty{nullptr};
    QPlainTextEdit* m_responseRaw{nullptr};
    QPlainTextEdit* m_responseHeaders{nullptr};
    QPlainTextEdit* m_console{nullptr};
    QPlainTextEdit* m_codeSnippet{nullptr};
    QPlainTextEdit* m_documentation{nullptr};
    QComboBox* m_snippetLanguage{nullptr};
    QLabel* m_status{nullptr};
    QLabel* m_statusIcon{nullptr};
    QProgressDialog* m_progress{nullptr};
    QSystemTrayIcon* m_trayIcon{nullptr};
    bool m_forceQuit{false};
};

}  // namespace courierman::ui
