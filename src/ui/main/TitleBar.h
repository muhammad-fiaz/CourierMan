#pragma once

#include <QComboBox>
#include <QLabel>
#include <QMenuBar>
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

private:
    void buildMenus();

    QWidget* m_hostWindow;
    QMenuBar* m_menuBar{nullptr};
    QComboBox* m_environmentSelector{nullptr};
};

}  // namespace courierman::ui
