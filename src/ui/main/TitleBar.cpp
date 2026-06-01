#include "ui/main/TitleBar.h"

#include "ui/shared_widgets/HoverButton.h"

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

    auto* leftCluster = new QWidget(this);
    leftCluster->setObjectName(QStringLiteral("titleLeftCluster"));
    auto* leftLayout = new QHBoxLayout(leftCluster);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(8);

    auto* logo = new QLabel(this);
    logo->setObjectName(QStringLiteral("titleLogo"));
    logo->setPixmap(QIcon(QStringLiteral(":/courierman/icons/logo_rounded.png")).pixmap(24, 24));
    leftLayout->addWidget(logo);

    m_menuBar = new QMenuBar(this);
    m_menuBar->setNativeMenuBar(false);
    m_menuBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    buildMenus();
    leftLayout->addWidget(m_menuBar);
    layout->addWidget(leftCluster, 1);

    auto* title = new QLabel(QStringLiteral("CourierMan"), this);
    title->setObjectName(QStringLiteral("centerTitleText"));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title, 1);

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

    auto* leftToggle = new HoverButton(QStringLiteral("L"), this);
    leftToggle->setObjectName(QStringLiteral("panelToggle"));
    leftToggle->setToolTip(QStringLiteral("Toggle left sidebar"));
    connect(leftToggle, &QPushButton::clicked, this, &TitleBar::toggleLeftRequested);
    rightLayout->addWidget(leftToggle);

    auto* rightToggle = new HoverButton(QStringLiteral("R"), this);
    rightToggle->setObjectName(QStringLiteral("panelToggle"));
    rightToggle->setToolTip(QStringLiteral("Toggle right sidebar"));
    connect(rightToggle, &QPushButton::clicked, this, &TitleBar::toggleRightRequested);
    rightLayout->addWidget(rightToggle);

    auto* bottomToggle = new HoverButton(QStringLiteral("C"), this);
    bottomToggle->setObjectName(QStringLiteral("panelToggle"));
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
    file->addAction(QStringLiteral("New Request"), this, &TitleBar::workspaceRequested);
    file->addAction(QStringLiteral("Import"), this, &TitleBar::importRequested);
    file->addAction(QStringLiteral("Export"), this, &TitleBar::exportRequested);
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

    auto* settings = m_menuBar->addMenu(QStringLiteral("Settings"));
    settings->addAction(QStringLiteral("Preferences"), this, &TitleBar::settingsRequested);
    settings->addAction(QStringLiteral("Workspace Settings"), this, &TitleBar::settingsRequested);

    auto* help = m_menuBar->addMenu(QStringLiteral("Help"));
    help->addAction(QStringLiteral("Website"), this, &TitleBar::websiteRequested);
    help->addAction(QStringLiteral("Release Notes"), this, &TitleBar::releaseNotesRequested);
    help->addAction(QStringLiteral("Report Issue"), this, &TitleBar::issueReportRequested);
    help->addAction(QStringLiteral("Check For Updates"), this, &TitleBar::updateRequested);
}

}  // namespace courierman::ui
