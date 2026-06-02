#include "ui/main/MainWindow.h"

#include "backend/common/FeatureCatalog.h"
#include "backend/common/IssueReporter.h"
#include "backend/common/ScriptRunner.h"
#include "backend/common/SnippetGenerator.h"
#include "ui/dialogs/UpdateDialog.h"
#include "ui/shared_widgets/HoverButton.h"
#include "ui/shared_widgets/IconFactory.h"
#include "ui/shared_widgets/ThemeManager.h"

#include <QApplication>
#include <QAbstractItemView>
#include <QDateTime>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
    label->setObjectName(QStringLiteral("sectionTitle"));
    return label;
}

void prepareKeyValueTable(QTableWidget* table) {
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({QStringLiteral("Enabled"), QStringLiteral("Key"), QStringLiteral("Value")});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setShowGrid(false);
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

[[nodiscard]] QJsonArray keyValuesToJson(const QList<backend::KeyValue>& values) {
    QJsonArray array;
    for (const auto& value : values) {
        array.append(QJsonObject{{QStringLiteral("key"), value.key},
                                 {QStringLiteral("value"), value.value},
                                 {QStringLiteral("enabled"), value.enabled}});
    }
    return array;
}

void populateKeyValueTable(QTableWidget* table, const QJsonArray& values) {
    table->setRowCount(qMax(4, values.size()));
    for (int row = 0; row < table->rowCount(); ++row) {
        const QJsonObject object = row < values.size() ? values.at(row).toObject() : QJsonObject{};
        auto* enabled = new QTableWidgetItem();
        enabled->setCheckState(object.value(QStringLiteral("enabled")).toBool(row == 0) ? Qt::Checked : Qt::Unchecked);
        table->setItem(row, 0, enabled);
        table->setItem(row, 1, new QTableWidgetItem(object.value(QStringLiteral("key")).toString()));
        table->setItem(row, 2, new QTableWidgetItem(object.value(QStringLiteral("value")).toString()));
    }
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
    resize(1440, 900);

    auto* root = new QWidget(this);
    root->setObjectName(QStringLiteral("appRoot"));
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
    m_statusIcon = new QLabel(this);
    m_statusIcon->setPixmap(IconFactory::icon(QStringLiteral("online"), QColor(QStringLiteral("#22c55e"))).pixmap(14, 14));
    m_statusIcon->setToolTip(QStringLiteral("Online"));
    statusBar()->addWidget(m_status, 1);
    statusBar()->addPermanentWidget(m_statusIcon);
}

QWidget* MainWindow::buildWorkspacePage() {
    auto* workspace = new QWidget(this);
    workspace->setObjectName(QStringLiteral("workspacePage"));
    auto* layout = new QVBoxLayout(workspace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* horizontal = new QSplitter(Qt::Horizontal, workspace);
    horizontal->setChildrenCollapsible(true);
    m_leftPanel = buildLeftSidebar();
    m_rightPanel = buildRightSidebar();

    auto* center = new QSplitter(Qt::Vertical, workspace);
    center->setObjectName(QStringLiteral("centerSurface"));
    center->setChildrenCollapsible(true);
    center->addWidget(buildRequestEditor());
    center->addWidget(buildResponseViewer());
    m_bottomPanel = buildBottomBar();
    center->addWidget(m_bottomPanel);
    center->setSizes({360, 330, 150});

    horizontal->addWidget(m_leftPanel);
    horizontal->addWidget(center);
    horizontal->addWidget(m_rightPanel);
    horizontal->setSizes({300, 800, 360});

    layout->addWidget(horizontal, 1);
    return workspace;
}

QWidget* MainWindow::buildLeftSidebar() {
    auto* panel = new QWidget(this);
    panel->setObjectName(QStringLiteral("leftPanel"));
    panel->setMinimumWidth(260);
    panel->setMaximumWidth(420);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* top = new QHBoxLayout();
    top->addWidget(sectionTitle(QStringLiteral("Workspace"), panel));
    top->addStretch(1);
    auto* add = new HoverButton(QStringLiteral("New"), panel);
    add->setIcon(IconFactory::icon(QStringLiteral("add")));
    connect(add, &QPushButton::clicked, this, [this]() { appendConsole(QStringLiteral("Created a new local request draft.")); });
    top->addWidget(add);
    layout->addLayout(top);

    auto* searchRow = new QHBoxLayout();
    auto* search = new QLineEdit(panel);
    search->setPlaceholderText(QStringLiteral("Search collections"));
    search->addAction(IconFactory::icon(QStringLiteral("search")), QLineEdit::LeadingPosition);
    searchRow->addWidget(search, 1);
    auto* filter = new HoverButton(QString{}, panel);
    filter->setObjectName(QStringLiteral("panelToggle"));
    filter->setIcon(IconFactory::icon(QStringLiteral("filter")));
    filter->setToolTip(QStringLiteral("Filter collections"));
    searchRow->addWidget(filter);
    layout->addLayout(searchRow);

    auto* tabs = new QTabWidget(panel);
    tabs->setUsesScrollButtons(false);
    tabs->setDocumentMode(true);
    tabs->setIconSize(QSize(16, 16));
    auto* collections = new QTreeWidget(tabs);
    collections->setHeaderHidden(true);
    auto* root = new QTreeWidgetItem(collections, {QStringLiteral("CourierMan Demo")});
    new QTreeWidgetItem(root, {QStringLiteral("GET Health Check")});
    new QTreeWidgetItem(root, {QStringLiteral("POST JSON Payload")});
    root->setExpanded(true);
    tabs->addTab(collections, IconFactory::icon(QStringLiteral("collections")), QStringLiteral("Collections"));

    auto* apis = new QListWidget(tabs);
    apis->addItems(backend::FeatureCatalog::protocolNames());
    tabs->addTab(apis, IconFactory::icon(QStringLiteral("api")), QStringLiteral("APIs"));

    auto* history = new QListWidget(tabs);
    history->addItem(QStringLiteral("No sent requests yet"));
    tabs->addTab(history, IconFactory::icon(QStringLiteral("history")), QStringLiteral("History"));

    layout->addWidget(tabs, 1);
    return panel;
}

QWidget* MainWindow::buildRequestEditor() {
    auto* editor = new QWidget(this);
    editor->setObjectName(QStringLiteral("centerSurface"));
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
    send->setIcon(IconFactory::icon(QStringLiteral("send"), QColor(QStringLiteral("#ffffff"))));
    connect(send, &QPushButton::clicked, this, &MainWindow::sendRequest);
    requestLine->addWidget(send);
    layout->addLayout(requestLine);

    auto* tabs = new QTabWidget(editor);
    tabs->setUsesScrollButtons(false);
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
    viewer->setObjectName(QStringLiteral("centerSurface"));
    auto* layout = new QVBoxLayout(viewer);
    layout->setContentsMargins(12, 4, 12, 8);
    layout->addWidget(sectionTitle(QStringLiteral("Response"), viewer));

    auto* tabs = new QTabWidget(viewer);
    tabs->setUsesScrollButtons(false);
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
    panel->setObjectName(QStringLiteral("rightPanel"));
    panel->setMinimumWidth(320);
    panel->setMaximumWidth(420);
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* tabs = new QTabWidget(panel);
    tabs->setUsesScrollButtons(false);
    tabs->setDocumentMode(true);
    tabs->setIconSize(QSize(16, 16));
    m_documentation = new QPlainTextEdit(tabs);
    m_documentation->setReadOnly(true);
    tabs->addTab(m_documentation, IconFactory::icon(QStringLiteral("docs")), QStringLiteral("Docs"));

    auto* code = new QWidget(tabs);
    auto* codeLayout = new QVBoxLayout(code);
    m_snippetLanguage = new QComboBox(code);
    m_snippetLanguage->addItems(backend::FeatureCatalog::codeGenerationLanguages());
    codeLayout->addWidget(m_snippetLanguage);
    m_codeSnippet = new QPlainTextEdit(code);
    m_codeSnippet->setReadOnly(true);
    codeLayout->addWidget(m_codeSnippet, 1);
    connect(m_snippetLanguage, &QComboBox::currentTextChanged, this, &MainWindow::updateCodeSnippet);
    tabs->addTab(code, IconFactory::icon(QStringLiteral("code")), QStringLiteral("Code Gen"));

    auto* ai = new QListWidget(tabs);
    ai->addItems({QStringLiteral("Generate tests"), QStringLiteral("Mock payloads"),
                  QStringLiteral("Infer schema"), QStringLiteral("Write documentation"),
                  QStringLiteral("Explain response"), QStringLiteral("Local offline assistant")});
    tabs->addTab(ai, IconFactory::icon(QStringLiteral("ai")), QStringLiteral("AI Tools"));

    layout->addWidget(tabs, 1);
    return panel;
}

QWidget* MainWindow::buildBottomBar() {
    auto* tabs = new QTabWidget(this);
    tabs->setUsesScrollButtons(false);
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
    connect(m_titleBar, &TitleBar::importRequested, this, &MainWindow::importRequest);
    connect(m_titleBar, &TitleBar::exportRequested, this, &MainWindow::exportRequest);
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
    connect(m_method, &QComboBox::currentTextChanged, this, &MainWindow::updateCodeSnippet);
    connect(m_method, &QComboBox::currentTextChanged, this, &MainWindow::updateDocumentation);
    connect(m_url, &QLineEdit::textChanged, this, &MainWindow::updateCodeSnippet);
    connect(m_url, &QLineEdit::textChanged, this, &MainWindow::updateDocumentation);
    connect(m_body, &QPlainTextEdit::textChanged, this, &MainWindow::updateCodeSnippet);
    connect(m_body, &QPlainTextEdit::textChanged, this, &MainWindow::updateDocumentation);
    connect(m_params, &QTableWidget::itemChanged, this, &MainWindow::updateDocumentation);
    connect(m_headers, &QTableWidget::itemChanged, this, &MainWindow::updateDocumentation);
    updateCodeSnippet();
    updateDocumentation();
}

void MainWindow::sendRequest() {
    const auto request = collectRequest();
    appendConsole(QStringLiteral("[%1] Sending %2 %3")
                      .arg(QDateTime::currentDateTime().toString(Qt::ISODate), request.method, request.url));
    m_restClient->send(request);
}

void MainWindow::handleRequestStarted() {
    showProgress(QStringLiteral("Sending request"), QStringLiteral("Waiting for the API response..."));
    m_status->setText(QStringLiteral("Sending request..."));
    m_responsePretty->setPlainText(QStringLiteral("Loading..."));
    m_responseRaw->clear();
    m_responseHeaders->clear();
}

void MainWindow::handleResponse(const backend::ResponseEnvelope& response) {
    hideProgress();
    m_status->setText(QStringLiteral("HTTP %1 %2").arg(response.statusCode).arg(response.reason));
    m_statusIcon->setToolTip(QStringLiteral("%1 ms, %2 bytes").arg(response.elapsedMs).arg(response.body.size()));
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
    hideProgress();
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

void MainWindow::importRequest() {
    const QString path = QFileDialog::getOpenFileName(this,
                                                      QStringLiteral("Import Request"),
                                                      QString{},
                                                      QStringLiteral("CourierMan Request (*.json);;JSON Files (*.json)"));
    if (path.isEmpty()) {
        return;
    }
    showProgress(QStringLiteral("Importing request"), QStringLiteral("Reading request JSON..."));

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        hideProgress();
        QMessageBox::warning(this, QStringLiteral("Import Request"), file.errorString());
        return;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        hideProgress();
        QMessageBox::warning(this, QStringLiteral("Import Request"), error.errorString());
        return;
    }

    const QJsonObject object = document.object();
    m_method->setCurrentText(object.value(QStringLiteral("method")).toString(QStringLiteral("GET")));
    m_url->setText(object.value(QStringLiteral("url")).toString());
    m_body->setPlainText(object.value(QStringLiteral("body")).toString());
    populateKeyValueTable(m_headers, object.value(QStringLiteral("headers")).toArray());
    populateKeyValueTable(m_params, object.value(QStringLiteral("queryParameters")).toArray());
    updateCodeSnippet();
    appendConsole(QStringLiteral("Imported request from %1").arg(path));
    hideProgress();
}

void MainWindow::exportRequest() {
    const QString path = QFileDialog::getSaveFileName(this,
                                                      QStringLiteral("Export Request"),
                                                      QStringLiteral("courierman-request.json"),
                                                      QStringLiteral("CourierMan Request (*.json);;JSON Files (*.json)"));
    if (path.isEmpty()) {
        return;
    }
    showProgress(QStringLiteral("Exporting request"), QStringLiteral("Writing request JSON..."));

    const auto request = collectRequest();
    const QJsonObject object{{QStringLiteral("schema"), QStringLiteral("courierman.request.v1")},
                             {QStringLiteral("method"), request.method},
                             {QStringLiteral("url"), request.url},
                             {QStringLiteral("headers"), keyValuesToJson(request.headers)},
                             {QStringLiteral("queryParameters"), keyValuesToJson(request.queryParameters)},
                             {QStringLiteral("body"), QString::fromUtf8(request.body)}};

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        hideProgress();
        QMessageBox::warning(this, QStringLiteral("Export Request"), file.errorString());
        return;
    }
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    appendConsole(QStringLiteral("Exported request to %1").arg(path));
    hideProgress();
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
    const QString language = m_snippetLanguage ? m_snippetLanguage->currentText() : QStringLiteral("cURL");
    const auto snippet = backend::SnippetGenerator::generate(request, language);
    m_codeSnippet->setPlainText(snippet ? *snippet : snippet.error());
}

void MainWindow::updateDocumentation() {
    if (m_documentation == nullptr || m_method == nullptr || m_url == nullptr) {
        return;
    }

    const auto request = collectRequest();
    QString text;
    QTextStream stream(&text);
    stream << "Request\n";
    stream << "=======\n\n";
    stream << request.method << " " << request.url << "\n\n";

    stream << "Query Parameters\n";
    stream << "----------------\n";
    if (request.queryParameters.isEmpty()) {
        stream << "No query parameters.\n";
    } else {
        for (const auto& item : request.queryParameters) {
            stream << (item.enabled ? "[x] " : "[ ] ") << item.key << " = " << item.value << "\n";
        }
    }

    stream << "\nHeaders\n";
    stream << "-------\n";
    if (request.headers.isEmpty()) {
        stream << "No headers.\n";
    } else {
        for (const auto& item : request.headers) {
            stream << (item.enabled ? "[x] " : "[ ] ") << item.key << ": " << item.value << "\n";
        }
    }

    stream << "\nBody\n";
    stream << "----\n";
    stream << (request.body.isEmpty() ? QStringLiteral("No request body.") : QString::fromUtf8(request.body));
    m_documentation->setPlainText(text);
}

void MainWindow::showProgress(const QString& title, const QString& message) {
    if (m_progress == nullptr) {
        m_progress = new QProgressDialog(this);
        m_progress->setWindowModality(Qt::WindowModal);
        m_progress->setCancelButton(nullptr);
        m_progress->setMinimumDuration(0);
        m_progress->setAutoClose(false);
        m_progress->setAutoReset(false);
    }
    m_progress->setWindowTitle(title);
    m_progress->setLabelText(message);
    m_progress->setRange(0, 0);
    m_progress->show();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void MainWindow::hideProgress() {
    if (m_progress != nullptr) {
        m_progress->hide();
        m_progress->reset();
    }
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
