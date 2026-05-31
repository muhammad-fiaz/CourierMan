#pragma once

#include <QComboBox>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QWidget>

namespace courierman::ui {

class TitleBar : public QWidget {
    Q_OBJECT

public:
    explicit TitleBar(QWidget* hostWindow, QWidget* parent = nullptr);
    [[nodiscard]] QComboBox* environmentSelector() const;

signals:
    void workspaceRequested();
    void settingsRequested();
    void runnerRequested();
    void updateRequested();
    void websiteRequested();
    void releaseNotesRequested();
    void issueReportRequested();
    void importRequested();
    void exportRequested();
    void quitRequested();
    void toggleLeftRequested();
    void toggleRightRequested();
    void toggleBottomRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    void buildMenus();
    void toggleMaximized();

    QWidget* m_hostWindow;
    QMenuBar* m_menuBar{nullptr};
    QComboBox* m_environmentSelector{nullptr};
    QPoint m_dragStart;
};

}  // namespace courierman::ui
