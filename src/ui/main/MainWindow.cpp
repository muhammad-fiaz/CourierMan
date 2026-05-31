#include "ui/main/MainWindow.h"

#include "backend/common/FeatureCatalog.h"
#include "backend/common/IssueReporter.h"
#include "backend/common/ScriptRunner.h"
#include "backend/common/SnippetGenerator.h"
#include "ui/dialogs/UpdateDialog.h"
#include "ui/shared_widgets/HoverButton.h"
#include "ui/shared_widgets/ThemeManager.h"

#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>
#include <QSysInfo>
#include <QTextStream>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include <utility>

namespace courierman::ui {
namespace {

[[nodiscard]] QLabel* sectionTitle(const QString& text, QWidget* parent) {
    auto* label = new QLabel(text, parent);
    label->setStyleSheet(QStringLiteral("font-size: 14px; font-weight: 700; color: #0f172a;"));
    return label;
}

void prepareKeyValueTable(QTableWidget* table) {
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({QStringLiteral("Enabled"), QStringLiteral("Key"), QStringLiteral("Value")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setRowCount(4);
    for (int row = 0; row < table->rowCount(); ++row) {
        auto* enabled = new QTableWidgetItem();
        enabled->setCheckState(row == 0 ? Qt::Checked : Qt::Unchecked);
        table->setItem(row, 0, enabled);
        table->setItem(row, 1, new QTableWidgetItem(row == 0 ? QStringLiteral("Accept") : QString{}));
        table->setItem(row, 2, new QTableWidgetItem(row == 0 ? QStringLiteral("application/json") : QString{}));
    }
}

[[nodiscard]] QString prettyJsonOrRaw(const QByteArray& bytes) {
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &error);
    if (error.error == QJsonParseError::NoError && !document.isNull()) {
        return QString::fromUtf8(document.toJson(QJsonDocument::Indented));
    }
    return QString::fromUtf8(bytes);
}

}  // namespace

MainWindow::MainWindow(core::ConfigManager* configManager,
                       core::AppSettings settings,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_configManager(configManager)
    , m_settings(std::move(settings))
    , m_restClient(new network::RestClient(this)) {
    buildUi();
    initTray();
    connectSignals();
}

void MainWindow::buildUi() {
    setWindowTitle(QStringLiteral("CourierMan"));
    setWindowIcon(QIcon(QStringLiteral(":/courierman/icons/logo_rounded.png")));
    setWindowFlag(Qt::FramelessWindowHint, true);
    resize(1440, 900);

    auto* root = new QWidget(this);
    auto* rootLayout = new QVBoxLayout(root);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_titleBar = new TitleBar(this, root);
    rootLayout->addWidget(m_titleBar);

    m_stack = new QStackedWidget(root);
    m_stack->addWidget(buildWorkspacePage());
    m_stack->addWidget(buildRunnerPage());
    m_settingsView = new SettingsView(m_configManager, root);
    m_stack->addWidget(m_settingsView);
    rootLayout->addWidget(m_stack, 1);

    setCentralWidget(root);
    m_status = new QLabel(QStringLiteral("Ready"), this);
    m_stats = new QLabel(QStringLiteral("Offline-first database active"), this);
    statusBar()->addWidget(m_status, 1);
    statusBar()->addPermanentWidget(m_stats);
}

QWidget* MainWindow::buildWorkspacePage() {
    auto* workspace = new QWidget(this);
    auto* layout = new QVBoxLayout(workspace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* horizontal = new QSplitter(Qt::Horizontal, workspace);
    m_leftPanel = buildLeftSidebar();
    m_rightPanel = buildRightSidebar();

    auto* center = new QSplitter(Qt::Vertical, workspace);
    center->addWidget(buildRequestEditor());
    center->addWidget(buildResponseViewer());
    m_bottomPanel = buildBottomBar();
    center->addWidget(m_bottomPanel);
    center->setSizes({360, 330, 150});

    horizontal->addWidget(m_leftPanel);
    horizontal->addWidget(center);
    horizontal->addWidget(m_rightPanel);
    horizontal->setSizes({260, 880, 300});

    layout->addWidget(horizontal, 1);
    return workspace;
}

QWidget* MainWindow::buildLeftSidebar() {
    auto* panel = new QWidget(this);
    panel->setMinimumWidth(220);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* top = new QHBoxLayout();
    top->addWidget(sectionTitle(QStringLiteral("Workspace"), panel));
    top->addStretch(1);
    auto* add = new HoverButton(QStringLiteral("New"), panel);
    connect(add, &QPushButton::clicked, this, [this]() { appendConsole(QStringLiteral("Created a new local request draft.")); });
    top->addWidget(add);
    layout->addLayout(top);

    auto* tabs = new QTabWidget(panel);
    auto* collections = new QTreeWidget(tabs);
    collections->setHeaderHidden(true);
    auto* root = new QTreeWidgetItem(collections, {QStringLiteral("CourierMan Demo")});
    new QTreeWidgetItem(root, {QStringLiteral("GET Health Check")});
    new QTreeWidgetItem(root, {QStringLiteral("POST JSON Payload")});
    root->setExpanded(true);
    tabs->addTab(collections, QStringLiteral("Collections"));

    auto* apis = new QListWidget(tabs);
    apis->addItems(backend::FeatureCatalog::protocolNames());
    tabs->addTab(apis, QStringLiteral("APIs"));

    auto* history = new QListWidget(tabs);
    history->addItem(QStringLiteral("No sent requests yet"));
    tabs->addTab(history, QStringLiteral("History"));

    auto* env = new QListWidget(tabs);
    env->addItems({QStringLiteral("Local"), QStringLiteral("Staging"), QStringLiteral("Production")});
    tabs->addTab(env, QStringLiteral("Environments"));
    layout->addWidget(tabs, 1);
    return panel;
}

QWidget* MainWindow::buildRequestEditor() {
    auto* editor = new QWidget(this);
    auto* layout = new QVBoxLayout(editor);
    layout->setContentsMargins(12, 12, 12, 8);
    layout->setSpacing(10);

    auto* requestLine = new QHBoxLayout();
    m_method = new QComboBox(editor);
    m_method->addItems({QStringLiteral("GET"), QStringLiteral("POST"), QStringLiteral("PUT"),
                        QStringLiteral("PATCH"), QStringLiteral("DELETE"), QStringLiteral("HEAD"),
                        QStringLiteral("OPTIONS"), QStringLiteral("TRACE"), QStringLiteral("CONNECT")});
    requestLine->addWidget(m_method);

    m_url = new QLineEdit(QStringLiteral("https://httpbin.org/get"), editor);
    requestLine->addWidget(m_url, 1);

    auto* send = new HoverButton(QStringLiteral("Send"), editor);
    send->setObjectName(QStringLiteral("primaryButton"));
    connect(send, &QPushButton::clicked, this, &MainWindow::sendRequest);
    requestLine->addWidget(send);
    layout->addLayout(requestLine);

    auto* tabs = new QTabWidget(editor);
    m_params = new QTableWidget(tabs);
    prepareKeyValueTable(m_params);
    tabs->addTab(m_params, QStringLiteral("Params"));

    m_headers = new QTableWidget(tabs);
    prepareKeyValueTable(m_headers);
    tabs->addTab(m_headers, QStringLiteral("Headers"));

    auto* auth = new QWidget(tabs);
    auto* authForm = new QFormLayout(auth);
    auto* authType = new QComboBox(auth);
    authType->addItems({QStringLiteral("None"), QStringLiteral("API Key"), QStringLiteral("Bearer"),
                        QStringLiteral("Basic"), QStringLiteral("OAuth 1"), QStringLiteral("OAuth 2"),
                        QStringLiteral("AWS"), QStringLiteral("JWT"), QStringLiteral("mTLS")});
    authForm->addRow(QStringLiteral("Type"), authType);
    authForm->addRow(QStringLiteral("Status"), new QLabel(QStringLiteral("Credentials are stored locally."), auth));
    tabs->addTab(auth, QStringLiteral("Auth"));

    m_body = new QPlainTextEdit(tabs);
    m_body->setPlaceholderText(QStringLiteral("{\n  \"hello\": \"world\"\n}"));
    tabs->addTab(m_body, QStringLiteral("Body"));

    m_scriptEditor = new QPlainTextEdit(tabs);
    m_scriptEditor->setPlainText(QStringLiteral("pm.test(\"status is 200\", () => pm.response.to.have.status(200));"));
    tabs->addTab(m_scriptEditor, QStringLiteral("Scripts"));

    layout->addWidget(tabs, 1);
    return editor;
}

QWidget* MainWindow::buildResponseViewer() {
    auto* viewer = new QWidget(this);
    auto* layout = new QVBoxLayout(viewer);
    layout->setContentsMargins(12, 4, 12, 8);
    layout->addWidget(sectionTitle(QStringLiteral("Response"), viewer));

    auto* tabs = new QTabWidget(viewer);
    m_responsePretty = new QPlainTextEdit(tabs);
    m_responsePretty->setReadOnly(true);
    tabs->addTab(m_responsePretty, QStringLiteral("Pretty"));
    m_responseRaw = new QPlainTextEdit(tabs);
    m_responseRaw->setReadOnly(true);
    tabs->addTab(m_responseRaw, QStringLiteral("Raw"));
    m_responseHeaders = new QPlainTextEdit(tabs);
    m_responseHeaders->setReadOnly(true);
    tabs->addTab(m_responseHeaders, QStringLiteral("Headers"));
    layout->addWidget(tabs, 1);
    return viewer;
}

QWidget* MainWindow::buildRightSidebar() {
    auto* panel = new QWidget(this);
    panel->setMinimumWidth(260);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* tabs = new QTabWidget(panel);
    auto* docs = new QListWidget(tabs);
    for (const auto& feature : backend::FeatureCatalog::allFeatures()) {
        docs->addItem(QStringLiteral("%1: %2").arg(feature.category, feature.name));
    }
    tabs->addTab(docs, QStringLiteral("Docs"));

    auto* code = new QWidget(tabs);
    auto* codeLayout = new QVBoxLayout(code);
    auto* lang = new QComboBox(code);
    lang->addItems(backend::FeatureCatalog::codeGenerationLanguages());
    codeLayout->addWidget(lang);
    m_codeSnippet = new QPlainTextEdit(code);
    m_codeSnippet->setReadOnly(true);
    codeLayout->addWidget(m_codeSnippet, 1);
    connect(lang, &QComboBox::currentTextChanged, this, &MainWindow::updateCodeSnippet);
    tabs->addTab(code, QStringLiteral("Code Gen"));

    auto* ai = new QListWidget(tabs);
    ai->addItems({QStringLiteral("Generate tests"), QStringLiteral("Mock payloads"),
                  QStringLiteral("Infer schema"), QStringLiteral("Write documentation"),
                  QStringLiteral("Explain response"), QStringLiteral("Local offline assistant")});
    tabs->addTab(ai, QStringLiteral("AI Tools"));

    layout->addWidget(tabs, 1);
    return panel;
}

QWidget* MainWindow::buildBottomBar() {
    auto* tabs = new QTabWidget(this);
    m_console = new QPlainTextEdit(tabs);
    m_console->setReadOnly(true);
    tabs->addTab(m_console, QStringLiteral("Console"));
    auto* stats = new QListWidget(tabs);
    stats->addItems({QStringLiteral("UI thread: active"), QStringLiteral("SQLite: WAL enabled"),
                     QStringLiteral("Logs: rotating async"), QStringLiteral("Network: idle")});
    tabs->addTab(stats, QStringLiteral("System Stats"));
    return tabs;
}

QWidget* MainWindow::buildRunnerPage() {
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 22, 28, 22);
    layout->addWidget(sectionTitle(QStringLiteral("Collection Runner"), page));
    auto* body = new QListWidget(page);
    body->addItems({QStringLiteral("Select collection"), QStringLiteral("Choose environment"),
                    QStringLiteral("Set iterations"), QStringLiteral("Run scripts and tests"),
                    QStringLiteral("Export report")});
    layout->addWidget(body, 1);
    return page;
}

void MainWindow::initTray() {
    m_trayIcon = new QSystemTrayIcon(QIcon(QStringLiteral(":/courierman/icons/logo_rounded.png")), this);
    auto* menu = new QMenu(this);
    menu->addAction(QStringLiteral("Open"), this, [this]() {
        showNormal();
        raise();
        activateWindow();
    });
    menu->addAction(QStringLiteral("Settings"), this, &MainWindow::showSettings);
    menu->addSeparator();
    menu->addAction(QStringLiteral("Exit"), this, &MainWindow::forceQuit);
    m_trayIcon->setContextMenu(menu);
    m_trayIcon->setToolTip(QStringLiteral("CourierMan"));
    m_trayIcon->show();
}

void MainWindow::connectSignals() {
    connect(m_restClient, &network::RestClient::requestStarted, this, &MainWindow::handleRequestStarted);
    connect(m_restClient, &network::RestClient::responseReady, this, &MainWindow::handleResponse);
    connect(m_restClient, &network::RestClient::requestFailed, this, &MainWindow::handleRequestFailed);
    connect(m_titleBar, &TitleBar::workspaceRequested, this, &MainWindow::showWorkspace);
    connect(m_titleBar, &TitleBar::runnerRequested, this, [this]() { m_stack->setCurrentIndex(1); });
    connect(m_titleBar, &TitleBar::settingsRequested, this, &MainWindow::showSettings);
    connect(m_titleBar, &TitleBar::updateRequested, this, &MainWindow::showUpdateDialog);
    connect(m_titleBar, &TitleBar::websiteRequested, this, &MainWindow::openWebsite);
    connect(m_titleBar, &TitleBar::releaseNotesRequested, this, &MainWindow::openReleaseNotes);
    connect(m_titleBar, &TitleBar::issueReportRequested, this, [this]() {
        const QUrl url = backend::IssueReporter::buildGitHubIssueUrl(
            {QStringLiteral("User-reviewed diagnostic report"),
             QApplication::applicationVersion(),
             QSysInfo::prettyProductName(),
             core::ConfigManager::logDirectory(),
             m_console ? m_console->toPlainText().right(4000) : QString{}});
        QDesktopServices::openUrl(url);
    });
    connect(m_titleBar, &TitleBar::quitRequested, this, &MainWindow::forceQuit);
    connect(m_titleBar, &TitleBar::toggleLeftRequested, this, [this]() {
        setPanelVisible(m_leftPanel, !m_leftPanel->isVisible());
    });
    connect(m_titleBar, &TitleBar::toggleRightRequested, this, [this]() {
        setPanelVisible(m_rightPanel, !m_rightPanel->isVisible());
    });
    connect(m_titleBar, &TitleBar::toggleBottomRequested, this, [this]() {
        setPanelVisible(m_bottomPanel, !m_bottomPanel->isVisible());
    });
    connect(m_settingsView, &SettingsView::backRequested, this, &MainWindow::showWorkspace);
    connect(m_settingsView, &SettingsView::settingsSaved, this, [this](const core::AppSettings& settings) {
        m_settings = settings;
        qApp->setStyleSheet(ThemeManager::styleSheetFor(m_settings));
        appendConsole(QStringLiteral("Settings saved to TOML."));
    });
    updateCodeSnippet();
}

void MainWindow::sendRequest() {
    const auto request = collectRequest();
    appendConsole(QStringLiteral("[%1] Sending %2 %3")
                      .arg(QDateTime::currentDateTime().toString(Qt::ISODate), request.method, request.url));
    m_restClient->send(request);
}

void MainWindow::handleRequestStarted() {
    m_status->setText(QStringLiteral("Sending request..."));
    m_responsePretty->setPlainText(QStringLiteral("Loading..."));
    m_responseRaw->clear();
    m_responseHeaders->clear();
}

void MainWindow::handleResponse(const backend::ResponseEnvelope& response) {
    m_status->setText(QStringLiteral("HTTP %1 %2").arg(response.statusCode).arg(response.reason));
    m_stats->setText(QStringLiteral("%1 ms, %2 bytes").arg(response.elapsedMs).arg(response.body.size()));
    m_responsePretty->setPlainText(prettyJsonOrRaw(response.body));
    m_responseRaw->setPlainText(QString::fromUtf8(response.body));

    QString headerText;
    QTextStream stream(&headerText);
    for (const auto& header : response.headers) {
        stream << header.key << ": " << header.value << '\n';
    }
    m_responseHeaders->setPlainText(headerText);
    const auto scriptResults =
        backend::ScriptRunner::runPostResponseTests(m_scriptEditor->toPlainText(), response);
    for (const auto& result : scriptResults) {
        appendConsole(QStringLiteral("Test %1: %2 (%3)")
                          .arg(result.passed ? QStringLiteral("passed") : QStringLiteral("failed"),
                               result.name,
                               result.message));
    }
    appendConsole(QStringLiteral("Received %1 in %2 ms").arg(response.statusCode).arg(response.elapsedMs));
}

void MainWindow::handleRequestFailed(const QString& message) {
    m_status->setText(QStringLiteral("Request failed"));
    appendConsole(QStringLiteral("Request failed: %1").arg(message));
}

void MainWindow::showWorkspace() {
    m_stack->setCurrentIndex(0);
    m_status->setText(QStringLiteral("Workspace"));
}

void MainWindow::showSettings() {
    m_settingsView->load();
    m_stack->setCurrentWidget(m_settingsView);
    m_status->setText(QStringLiteral("Settings"));
}

void MainWindow::showUpdateDialog() {
    auto* dialog = new UpdateDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->startCheck();
}

void MainWindow::openWebsite() {
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://muhammad-fiaz.github.io/CourierMan")));
}

void MainWindow::openReleaseNotes() {
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/muhammad-fiaz/CourierMan/releases")));
}

void MainWindow::forceQuit() {
    m_forceQuit = true;
    QApplication::quit();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (!m_forceQuit && (m_settings.closeToTray || m_settings.minimizeToTray) && m_trayIcon->isVisible()) {
        hide();
        m_trayIcon->showMessage(QStringLiteral("CourierMan"),
                                QStringLiteral("CourierMan is still running in the system tray."),
                                QSystemTrayIcon::Information,
                                1800);
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::appendConsole(const QString& line) {
    m_console->appendPlainText(line);
}

void MainWindow::updateCodeSnippet() {
    if (!m_codeSnippet) {
        return;
    }
    const QString method = m_method ? m_method->currentText() : QStringLiteral("GET");
    const QString url = m_url ? m_url->text() : QStringLiteral("https://httpbin.org/get");
    backend::RequestDefinition request;
    request.method = method;
    request.url = url;
    request.body = m_body ? m_body->toPlainText().toUtf8() : QByteArray{};
    request.headers = m_headers ? collectRequest().headers : QList<backend::KeyValue>{};
    const auto snippet = backend::SnippetGenerator::generate(request, QStringLiteral("cURL"));
    m_codeSnippet->setPlainText(snippet ? *snippet : snippet.error());
}

void MainWindow::setPanelVisible(QWidget* panel, bool visible) {
    panel->setVisible(visible);
    appendConsole(QStringLiteral("%1 panel %2")
                      .arg(panel == m_leftPanel ? QStringLiteral("Left")
                                                : panel == m_rightPanel ? QStringLiteral("Right")
                                                                        : QStringLiteral("Bottom"),
                           visible ? QStringLiteral("shown") : QStringLiteral("hidden")));
}

backend::RequestDefinition MainWindow::collectRequest() const {
    backend::RequestDefinition request;
    request.method = m_method->currentText();
    request.url = m_url->text().trimmed();
    request.body = m_body->toPlainText().toUtf8();

    const auto readTable = [](QTableWidget* table) {
        QList<backend::KeyValue> result;
        for (int row = 0; row < table->rowCount(); ++row) {
            const auto* enabled = table->item(row, 0);
            const auto* key = table->item(row, 1);
            const auto* value = table->item(row, 2);
            if (key != nullptr && !key->text().trimmed().isEmpty()) {
                result.append(backend::KeyValue{key->text().trimmed(),
                                                value != nullptr ? value->text() : QString{},
                                                enabled == nullptr || enabled->checkState() == Qt::Checked});
            }
        }
        return result;
    };

    request.headers = readTable(m_headers);
    request.queryParameters = readTable(m_params);
    return request;
}

}  // namespace courierman::ui
