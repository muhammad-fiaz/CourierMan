#pragma once

#include "core/ConfigManager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QStackedWidget>
#include <QWidget>

namespace courierman::ui {

class SettingsView : public QWidget {
    Q_OBJECT

public:
    explicit SettingsView(core::ConfigManager* configManager, QWidget* parent = nullptr);
    void load();

signals:
    void backRequested();
    void settingsSaved(const core::AppSettings& settings);

private slots:
    void save();

private:
    void buildUi();
    [[nodiscard]] QWidget* buildGeneralPage();
    [[nodiscard]] QWidget* buildAppearancePage();
    [[nodiscard]] QWidget* buildEditorPage();
    [[nodiscard]] QWidget* buildNetworkPage();
    [[nodiscard]] QWidget* buildCertificatesPage();
    [[nodiscard]] QWidget* buildUpdatesPage();
    [[nodiscard]] QWidget* buildStartupPage();
    [[nodiscard]] QWidget* buildPrivacyPage();
    [[nodiscard]] QWidget* buildAiPage();
    [[nodiscard]] QWidget* buildAdvancedPage();
    void selectPage(int index);

    core::ConfigManager* m_configManager;
    core::AppSettings m_currentSettings;
    QStackedWidget* m_pages{nullptr};
    QLabel* m_title{nullptr};
    QComboBox* m_theme{nullptr};
    QComboBox* m_aiProvider{nullptr};
    QComboBox* m_startupMode{nullptr};
    QLineEdit* m_accentColor{nullptr};
    QLineEdit* m_editorFontFamily{nullptr};
    QLineEdit* m_proxyUrl{nullptr};
    QLineEdit* m_certificatePath{nullptr};
    QCheckBox* m_minimizeToTray{nullptr};
    QCheckBox* m_closeToTray{nullptr};
    QCheckBox* m_autoStart{nullptr};
    QCheckBox* m_autoCheckUpdates{nullptr};
    QCheckBox* m_silentUpdates{nullptr};
    QCheckBox* m_verifyDownloads{nullptr};
    QCheckBox* m_telemetry{nullptr};
    QSpinBox* m_editorFontSize{nullptr};
    QSpinBox* m_networkTimeout{nullptr};
    QSpinBox* m_retryCount{nullptr};
    QSpinBox* m_logRetention{nullptr};
    QSpinBox* m_maxLogMb{nullptr};
};

}  // namespace courierman::ui
