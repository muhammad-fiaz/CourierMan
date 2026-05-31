#include "ui/main/TitleBar.h"

#include "ui/shared_widgets/HoverButton.h"

#include <QApplication>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QIcon>
#include <QMenu>
#include <QStyle>
#include <QUrl>

namespace courierman::ui {

TitleBar::TitleBar(QWidget* hostWindow, QWidget* parent)
    : QWidget(parent)
    , m_hostWindow(hostWindow) {
    setObjectName(QStringLiteral("titleBar"));
    setFixedHeight(50);
    setStyleSheet(QStringLiteral(
        "QWidget#titleBar { background: #ffffff; border-bottom: 1px solid #d7dee8; }"
        "QLabel#titleText { font-size: 18px; font-weight: 700; color: #0f172a; }"
        "QPushButton#windowButton { border: 0; border-radius: 5px; padding: 5px 9px; }"
        "QPushButton#windowButton:hover { background: #edf3f8; }"
        "QPushButton#closeButton { border: 0; border-radius: 5px; padding: 5px 9px; }"
        "QPushButton#closeButton:hover { background: #fee2e2; color: #991b1b; }"));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 10, 0);
    layout->setSpacing(10);

    auto* logo = new QLabel(this);
    logo->setPixmap(QIcon(QStringLiteral(":/courierman/icons/logo_rounded.png")).pixmap(30, 30));
    layout->addWidget(logo);

    auto* title = new QLabel(QStringLiteral("CourierMan"), this);
    title->setObjectName(QStringLiteral("titleText"));
    layout->addWidget(title);

    m_menuBar = new QMenuBar(this);
    m_menuBar->setNativeMenuBar(false);
    buildMenus();
    layout->addWidget(m_menuBar);

    layout->addStretch(1);

    auto* envLabel = new QLabel(QStringLiteral("Environment"), this);
    envLabel->setStyleSheet(QStringLiteral("color: #64748b; font-size: 12px;"));
    layout->addWidget(envLabel);

    m_environmentSelector = new QComboBox(this);
    m_environmentSelector->addItems({QStringLiteral("Local"), QStringLiteral("Staging"), QStringLiteral("Production")});
    m_environmentSelector->setMinimumWidth(160);
    layout->addWidget(m_environmentSelector);

    auto* leftToggle = new HoverButton(QStringLiteral("Left"), this);
    leftToggle->setToolTip(QStringLiteral("Toggle left sidebar"));
    connect(leftToggle, &QPushButton::clicked, this, &TitleBar::toggleLeftRequested);
    layout->addWidget(leftToggle);

    auto* rightToggle = new HoverButton(QStringLiteral("Right"), this);
    rightToggle->setToolTip(QStringLiteral("Toggle right sidebar"));
    connect(rightToggle, &QPushButton::clicked, this, &TitleBar::toggleRightRequested);
    layout->addWidget(rightToggle);

    auto* bottomToggle = new HoverButton(QStringLiteral("Console"), this);
    bottomToggle->setToolTip(QStringLiteral("Toggle bottom console"));
    connect(bottomToggle, &QPushButton::clicked, this, &TitleBar::toggleBottomRequested);
    layout->addWidget(bottomToggle);

    auto* minimize = new QPushButton(QStringLiteral("-"), this);
    minimize->setObjectName(QStringLiteral("windowButton"));
    connect(minimize, &QPushButton::clicked, m_hostWindow, &QWidget::showMinimized);
    layout->addWidget(minimize);

    auto* maximize = new QPushButton(QStringLiteral("[]"), this);
    maximize->setObjectName(QStringLiteral("windowButton"));
    connect(maximize, &QPushButton::clicked, this, &TitleBar::toggleMaximized);
    layout->addWidget(maximize);

    auto* close = new QPushButton(QStringLiteral("X"), this);
    close->setObjectName(QStringLiteral("closeButton"));
    connect(close, &QPushButton::clicked, m_hostWindow, &QWidget::close);
    layout->addWidget(close);
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

void TitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragStart = event->globalPosition().toPoint() - m_hostWindow->frameGeometry().topLeft();
        event->accept();
    }
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
    if ((event->buttons() & Qt::LeftButton) != 0 && !m_dragStart.isNull() && !m_hostWindow->isMaximized()) {
        m_hostWindow->move(event->globalPosition().toPoint() - m_dragStart);
        event->accept();
    }
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        toggleMaximized();
        event->accept();
    }
}

void TitleBar::toggleMaximized() {
    if (m_hostWindow->isMaximized()) {
        m_hostWindow->showNormal();
    } else {
        m_hostWindow->showMaximized();
    }
}

}  // namespace courierman::ui
