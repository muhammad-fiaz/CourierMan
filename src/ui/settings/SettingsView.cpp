#include "ui/settings/SettingsView.h"

#include "backend/common/AutoStartManager.h"
#include "ui/shared_widgets/HoverButton.h"

#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

namespace courierman::ui {

SettingsView::SettingsView(core::ConfigManager* configManager, QWidget* parent)
    : QWidget(parent)
    , m_configManager(configManager) {
    buildUi();
}

void SettingsView::load() {
    auto loaded = m_configManager->load();
    if (!loaded) {
        QMessageBox::warning(this, QStringLiteral("Settings"), loaded.error());
        return;
    }
    m_currentSettings = *loaded;
    m_theme->setCurrentText(m_currentSettings.theme);
    m_aiProvider->setCurrentText(m_currentSettings.aiProvider);
    m_startupMode->setCurrentText(m_currentSettings.startupMode);
    m_accentColor->setText(m_currentSettings.accentColor);
    m_editorFontFamily->setText(m_currentSettings.editorFontFamily);
    m_editorFontSize->setValue(m_currentSettings.editorFontSize);
    m_proxyUrl->setText(m_currentSettings.proxyUrl);
    m_certificatePath->setText(m_currentSettings.certificatePath);
    m_networkTimeout->setValue(m_currentSettings.networkTimeoutMs);
    m_retryCount->setValue(m_currentSettings.retryCount);
    m_minimizeToTray->setChecked(m_currentSettings.minimizeToTray);
    m_closeToTray->setChecked(m_currentSettings.closeToTray);
    m_autoStart->setChecked(backend::AutoStartManager::isEnabled());
    m_autoCheckUpdates->setChecked(m_currentSettings.autoCheckUpdates);
    m_silentUpdates->setChecked(m_currentSettings.silentUpdates);
    m_verifyDownloads->setChecked(m_currentSettings.verifyDownloads);
    m_telemetry->setChecked(m_currentSettings.telemetryEnabled);
    m_logRetention->setValue(m_currentSettings.logRetentionDays);
    m_maxLogMb->setValue(m_currentSettings.maxLogFileMb);
}

void SettingsView::buildUi() {
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    auto* nav = new QWidget(this);
    nav->setObjectName(QStringLiteral("settingsNav"));
    nav->setMinimumWidth(190);
    nav->setMaximumWidth(260);
    auto* navLayout = new QVBoxLayout(nav);
    navLayout->setContentsMargins(14, 18, 14, 14);
    navLayout->setSpacing(8);

    auto* back = new HoverButton(QStringLiteral("Back To Workspace"), nav);
    connect(back, &QPushButton::clicked, this, &SettingsView::backRequested);
    navLayout->addWidget(back);

    const QStringList pages{QStringLiteral("General"),      QStringLiteral("Appearance"),
                            QStringLiteral("Editor"),       QStringLiteral("Network"),
                            QStringLiteral("Certificates"), QStringLiteral("Updates"),
                            QStringLiteral("Startup"),      QStringLiteral("Privacy"),
                            QStringLiteral("AI Providers"), QStringLiteral("Advanced")};
    for (int i = 0; i < pages.size(); ++i) {
        auto* button = new HoverButton(pages.at(i), nav);
        connect(button, &QPushButton::clicked, this, [this, i]() { selectPage(i); });
        navLayout->addWidget(button);
    }
    navLayout->addStretch(1);
    root->addWidget(nav);

    auto* content = new QWidget(this);
    content->setObjectName(QStringLiteral("settingsContent"));
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(24, 20, 24, 20);
    contentLayout->setSpacing(16);

    m_title = new QLabel(QStringLiteral("Settings"), content);
    m_title->setObjectName(QStringLiteral("settingsTitle"));
    contentLayout->addWidget(m_title);

    m_pages = new QStackedWidget(content);
    m_pages->addWidget(buildGeneralPage());
    m_pages->addWidget(buildAppearancePage());
    m_pages->addWidget(buildEditorPage());
    m_pages->addWidget(buildNetworkPage());
    m_pages->addWidget(buildCertificatesPage());
    m_pages->addWidget(buildUpdatesPage());
    m_pages->addWidget(buildStartupPage());
    m_pages->addWidget(buildPrivacyPage());
    m_pages->addWidget(buildAiPage());
    m_pages->addWidget(buildAdvancedPage());

    auto* scroll = new QScrollArea(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidget(m_pages);
    contentLayout->addWidget(scroll, 1);

    auto* actions = new QHBoxLayout();
    actions->addStretch(1);
    auto* saveButton = new HoverButton(QStringLiteral("Save Settings"), content);
    saveButton->setObjectName(QStringLiteral("primaryButton"));
    connect(saveButton, &QPushButton::clicked, this, &SettingsView::save);
    actions->addWidget(saveButton);
    contentLayout->addLayout(actions);
    root->addWidget(content, 1);
}

QWidget* SettingsView::buildGeneralPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    form->setLabelAlignment(Qt::AlignRight);

    m_minimizeToTray = new QCheckBox(QStringLiteral("Minimize window to tray"), page);
    form->addRow(QStringLiteral("Minimize"), m_minimizeToTray);

    m_closeToTray = new QCheckBox(QStringLiteral("Close button sends CourierMan to tray"), page);
    form->addRow(QStringLiteral("Close"), m_closeToTray);

    m_autoStart = new QCheckBox(QStringLiteral("Start CourierMan on system boot"), page);
    form->addRow(QStringLiteral("Autostart"), m_autoStart);
    return page;
}

QWidget* SettingsView::buildAppearancePage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_theme = new QComboBox(page);
    m_theme->addItems({QStringLiteral("system"), QStringLiteral("light"), QStringLiteral("dark"),
                       QStringLiteral("custom")});
    form->addRow(QStringLiteral("Theme"), m_theme);
    m_accentColor = new QLineEdit(page);
    m_accentColor->setPlaceholderText(QStringLiteral("#0f91c8"));
    form->addRow(QStringLiteral("Accent color"), m_accentColor);
    return page;
}

QWidget* SettingsView::buildEditorPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_editorFontFamily = new QLineEdit(page);
    form->addRow(QStringLiteral("Font family"), m_editorFontFamily);
    m_editorFontSize = new QSpinBox(page);
    m_editorFontSize->setRange(8, 32);
    form->addRow(QStringLiteral("Font size"), m_editorFontSize);
    return page;
}

QWidget* SettingsView::buildNetworkPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_networkTimeout = new QSpinBox(page);
    m_networkTimeout->setRange(1000, 300000);
    m_networkTimeout->setSuffix(QStringLiteral(" ms"));
    form->addRow(QStringLiteral("Timeout"), m_networkTimeout);
    m_retryCount = new QSpinBox(page);
    m_retryCount->setRange(0, 10);
    form->addRow(QStringLiteral("Retries"), m_retryCount);
    m_proxyUrl = new QLineEdit(page);
    m_proxyUrl->setPlaceholderText(QStringLiteral("http://user:pass@proxy.local:8080"));
    form->addRow(QStringLiteral("Proxy"), m_proxyUrl);
    return page;
}

QWidget* SettingsView::buildCertificatesPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_certificatePath = new QLineEdit(page);
    m_certificatePath->setPlaceholderText(QStringLiteral("Path to client certificate"));
    form->addRow(QStringLiteral("Client certificate"), m_certificatePath);
    auto* note = new QLabel(QStringLiteral("mTLS certificate material is referenced locally and never exported automatically."), page);
    note->setWordWrap(true);
    form->addRow(QStringLiteral("Storage"), note);
    return page;
}

QWidget* SettingsView::buildUpdatesPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_autoCheckUpdates = new QCheckBox(QStringLiteral("Check GitHub Releases on startup"), page);
    form->addRow(QStringLiteral("Check"), m_autoCheckUpdates);
    m_silentUpdates = new QCheckBox(QStringLiteral("Download updates silently after user approval"), page);
    form->addRow(QStringLiteral("Silent"), m_silentUpdates);
    m_verifyDownloads = new QCheckBox(QStringLiteral("Verify downloaded release assets"), page);
    form->addRow(QStringLiteral("Verify"), m_verifyDownloads);
    return page;
}

QWidget* SettingsView::buildStartupPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_startupMode = new QComboBox(page);
    m_startupMode->addItems({QStringLiteral("normal"), QStringLiteral("tray"), QStringLiteral("last-workspace")});
    form->addRow(QStringLiteral("Startup mode"), m_startupMode);
    return page;
}

QWidget* SettingsView::buildPrivacyPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_telemetry = new QCheckBox(QStringLiteral("Share anonymous diagnostics"), page);
    form->addRow(QStringLiteral("Diagnostics"), m_telemetry);
    auto* note = new QLabel(QStringLiteral("Default is off. Secrets and payloads are never sent."), page);
    note->setWordWrap(true);
    form->addRow(QStringLiteral("Policy"), note);
    return page;
}

QWidget* SettingsView::buildAiPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_aiProvider = new QComboBox(page);
    m_aiProvider->addItems({QStringLiteral("offline"), QStringLiteral("openai"), QStringLiteral("gemini"),
                            QStringLiteral("claude"), QStringLiteral("ollama"), QStringLiteral("llama.cpp")});
    form->addRow(QStringLiteral("Default provider"), m_aiProvider);
    auto* note = new QLabel(QStringLiteral("Provider credentials are stored locally and are not synced by default."), page);
    note->setWordWrap(true);
    form->addRow(QStringLiteral("Credentials"), note);
    return page;
}

QWidget* SettingsView::buildAdvancedPage() {
    auto* page = new QWidget(this);
    auto* form = new QFormLayout(page);
    m_logRetention = new QSpinBox(page);
    m_logRetention->setRange(1, 90);
    form->addRow(QStringLiteral("Log retention days"), m_logRetention);
    m_maxLogMb = new QSpinBox(page);
    m_maxLogMb->setRange(1, 256);
    form->addRow(QStringLiteral("Max log file MB"), m_maxLogMb);
    return page;
}

void SettingsView::selectPage(int index) {
    m_pages->setCurrentIndex(index);
    const QStringList titles{QStringLiteral("General Settings"),
                             QStringLiteral("Appearance Settings"),
                             QStringLiteral("Editor Settings"),
                             QStringLiteral("Network Settings"),
                             QStringLiteral("Certificate Settings"),
                             QStringLiteral("Update Settings"),
                             QStringLiteral("Startup Settings"),
                             QStringLiteral("Privacy Settings"),
                             QStringLiteral("AI Provider Settings"),
                             QStringLiteral("Advanced Settings")};
    m_title->setText(titles.value(index, QStringLiteral("Settings")));
}

void SettingsView::save() {
    m_currentSettings.theme = m_theme->currentText();
    m_currentSettings.aiProvider = m_aiProvider->currentText();
    m_currentSettings.startupMode = m_startupMode->currentText();
    m_currentSettings.accentColor = m_accentColor->text().trimmed();
    m_currentSettings.editorFontFamily = m_editorFontFamily->text().trimmed();
    m_currentSettings.editorFontSize = m_editorFontSize->value();
    m_currentSettings.proxyUrl = m_proxyUrl->text().trimmed();
    m_currentSettings.certificatePath = m_certificatePath->text().trimmed();
    m_currentSettings.networkTimeoutMs = m_networkTimeout->value();
    m_currentSettings.retryCount = m_retryCount->value();
    m_currentSettings.minimizeToTray = m_minimizeToTray->isChecked();
    m_currentSettings.closeToTray = m_closeToTray->isChecked();
    m_currentSettings.autoStart = m_autoStart->isChecked();
    m_currentSettings.autoCheckUpdates = m_autoCheckUpdates->isChecked();
    m_currentSettings.silentUpdates = m_silentUpdates->isChecked();
    m_currentSettings.verifyDownloads = m_verifyDownloads->isChecked();
    m_currentSettings.telemetryEnabled = m_telemetry->isChecked();
    m_currentSettings.logRetentionDays = m_logRetention->value();
    m_currentSettings.maxLogFileMb = m_maxLogMb->value();

    if (auto autostart = backend::AutoStartManager::setEnabled(m_currentSettings.autoStart); !autostart) {
        QMessageBox::warning(this, QStringLiteral("Autostart"), autostart.error());
        return;
    }
    if (auto saved = m_configManager->save(m_currentSettings); !saved) {
        QMessageBox::warning(this, QStringLiteral("Settings"), saved.error());
        return;
    }
    emit settingsSaved(m_currentSettings);
}

}  // namespace courierman::ui
