#include "ui/main/TitleBar.h"

#include "ui/shared_widgets/HoverButton.h"
#include "ui/shared_widgets/IconFactory.h"

#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QSizePolicy>
#include <QStyle>
#include <QUrl>

namespace courierman::ui {

TitleBar::TitleBar(QWidget* hostWindow, QWidget* parent)
    : QWidget(parent)
    , m_hostWindow(hostWindow) {
    setObjectName(QStringLiteral("titleBar"));
    setMinimumHeight(44);
    setMaximumHeight(50);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 12, 0);
    layout->setSpacing(8);

    m_menuBar = new QMenuBar(this);
    m_menuBar->setNativeMenuBar(false);
    m_menuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    buildMenus();
    layout->addWidget(m_menuBar, 0);
    layout->addStretch(1);

    auto* rightCluster = new QWidget(this);
    rightCluster->setObjectName(QStringLiteral("titleRightCluster"));
    auto* rightLayout = new QHBoxLayout(rightCluster);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    auto* envLabel = new QLabel(QStringLiteral("Environment"), this);
    envLabel->setObjectName(QStringLiteral("mutedLabel"));
    rightLayout->addWidget(envLabel);

    m_environmentSelector = new QComboBox(this);
    m_environmentSelector->addItems({QStringLiteral("Local"), QStringLiteral("Staging"), QStringLiteral("Production")});
    m_environmentSelector->setMinimumWidth(160);
    rightLayout->addWidget(m_environmentSelector);

    auto* preview = new HoverButton(QString{}, this);
    preview->setObjectName(QStringLiteral("panelToggle"));
    preview->setIcon(IconFactory::icon(QStringLiteral("eye")));
    preview->setToolTip(QStringLiteral("Preview workspace"));
    connect(preview, &QPushButton::clicked, this, &TitleBar::workspaceRequested);
    rightLayout->addWidget(preview);

    auto* settings = new HoverButton(QString{}, this);
    settings->setObjectName(QStringLiteral("panelToggle"));
    settings->setIcon(IconFactory::icon(QStringLiteral("settings")));
    settings->setToolTip(QStringLiteral("Settings"));
    connect(settings, &QPushButton::clicked, this, &TitleBar::settingsRequested);
    rightLayout->addWidget(settings);

    auto* updates = new HoverButton(QString{}, this);
    updates->setObjectName(QStringLiteral("panelToggle"));
    updates->setIcon(IconFactory::icon(QStringLiteral("bell")));
    updates->setToolTip(QStringLiteral("Check for updates"));
    connect(updates, &QPushButton::clicked, this, &TitleBar::updateRequested);
    rightLayout->addWidget(updates);

    auto* leftToggle = new HoverButton(QString{}, this);
    leftToggle->setObjectName(QStringLiteral("panelToggle"));
    leftToggle->setIcon(IconFactory::icon(QStringLiteral("sidebar_left")));
    leftToggle->setToolTip(QStringLiteral("Toggle left sidebar"));
    connect(leftToggle, &QPushButton::clicked, this, &TitleBar::toggleLeftRequested);
    rightLayout->addWidget(leftToggle);

    auto* rightToggle = new HoverButton(QString{}, this);
    rightToggle->setObjectName(QStringLiteral("panelToggle"));
    rightToggle->setIcon(IconFactory::icon(QStringLiteral("sidebar_right")));
    rightToggle->setToolTip(QStringLiteral("Toggle right sidebar"));
    connect(rightToggle, &QPushButton::clicked, this, &TitleBar::toggleRightRequested);
    rightLayout->addWidget(rightToggle);

    auto* bottomToggle = new HoverButton(QString{}, this);
    bottomToggle->setObjectName(QStringLiteral("panelToggle"));
    bottomToggle->setIcon(IconFactory::icon(QStringLiteral("console")));
    bottomToggle->setToolTip(QStringLiteral("Toggle bottom console"));
    connect(bottomToggle, &QPushButton::clicked, this, &TitleBar::toggleBottomRequested);
    rightLayout->addWidget(bottomToggle);
    layout->addWidget(rightCluster, 1);
}

QComboBox* TitleBar::environmentSelector() const {
    return m_environmentSelector;
}

void TitleBar::buildMenus() {
    auto* file = m_menuBar->addMenu(QStringLiteral("File"));
    file->addAction(IconFactory::icon(QStringLiteral("add")), QStringLiteral("New Request"), this, &TitleBar::workspaceRequested);
    file->addAction(IconFactory::icon(QStringLiteral("import")), QStringLiteral("Import"), this, &TitleBar::importRequested);
    file->addAction(IconFactory::icon(QStringLiteral("export")), QStringLiteral("Export"), this, &TitleBar::exportRequested);
    file->addSeparator();
    file->addAction(QStringLiteral("Exit"), this, &TitleBar::quitRequested);

    auto* edit = m_menuBar->addMenu(QStringLiteral("Edit"));
    edit->addAction(QStringLiteral("Duplicate Request"));
    edit->addAction(QStringLiteral("Find In Workspace"));

    auto* view = m_menuBar->addMenu(QStringLiteral("View"));
    view->addAction(QStringLiteral("Workspace"), this, &TitleBar::workspaceRequested);
    view->addAction(QStringLiteral("Runner"), this, &TitleBar::runnerRequested);
    view->addSeparator();
    view->addAction(QStringLiteral("Toggle Left Sidebar"), this, &TitleBar::toggleLeftRequested);
    view->addAction(QStringLiteral("Toggle Right Sidebar"), this, &TitleBar::toggleRightRequested);
    view->addAction(QStringLiteral("Toggle Console"), this, &TitleBar::toggleBottomRequested);

    auto* settingsMenu = m_menuBar->addMenu(QStringLiteral("Settings"));
    settingsMenu->addAction(IconFactory::icon(QStringLiteral("settings")), QStringLiteral("Preferences"), this, &TitleBar::settingsRequested);
    settingsMenu->addAction(IconFactory::icon(QStringLiteral("environments")), QStringLiteral("Workspace Settings"), this, &TitleBar::settingsRequested);

    auto* help = m_menuBar->addMenu(QStringLiteral("Help"));
    help->addAction(QStringLiteral("Website"), this, &TitleBar::websiteRequested);
    help->addAction(QStringLiteral("Release Notes"), this, &TitleBar::releaseNotesRequested);
    help->addAction(QStringLiteral("Report Issue"), this, &TitleBar::issueReportRequested);
    help->addAction(QStringLiteral("Check For Updates"), this, &TitleBar::updateRequested);
}

}  // namespace courierman::ui
