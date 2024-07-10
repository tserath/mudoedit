#include "WindowManagement.h"
#include <QMdiSubWindow>
#include <QMdiArea>
#include <QSplitter>

WindowManagement::WindowManagement(QTabWidget *tabWidget, QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget)
{
}

void WindowManagement::tileWindows()
{
    // Tile windows in the active MDI area
    if (QMdiArea *mdiArea = getActiveMdiArea()) {
        mdiArea->tileSubWindows();
    }
}

void WindowManagement::cascadeWindows()
{
    // Cascade windows in the active MDI area
    if (QMdiArea *mdiArea = getActiveMdiArea()) {
        mdiArea->cascadeSubWindows();
    }
}

void WindowManagement::minimizeAllWindows()
{
    // Minimize all windows in the active MDI area
    if (QMdiArea *mdiArea = getActiveMdiArea()) {
        for (QMdiSubWindow *window : mdiArea->subWindowList()) {
            window->showMinimized();
        }
    }
}

void WindowManagement::closeAllWindows()
{
    // Close all windows in the active MDI area
    if (QMdiArea *mdiArea = getActiveMdiArea()) {
        mdiArea->closeAllSubWindows();
    }
}

void WindowManagement::tileWindowsSideBySide()
{
    // Tile windows side by side in the active MDI area
    if (QMdiArea *mdiArea = getActiveMdiArea()) {
        QList<QMdiSubWindow*> windows = mdiArea->subWindowList();
        if (windows.isEmpty())
            return;

        // Calculate the width for each window
        int width = mdiArea->width() / windows.size();
        int height = mdiArea->height();

        // Position each window side by side
        for (int i = 0; i < windows.size(); ++i) {
            QMdiSubWindow *window = windows.at(i);
            window->setGeometry(i * width, 0, width, height);
        }
    }
}

QMdiArea* WindowManagement::getActiveMdiArea() const
{
    // Get the current widget in the tab (which should be a QSplitter)
    QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->currentWidget());
    if (splitter) {
        // The MDI area should be the first widget in the splitter
        return qobject_cast<QMdiArea*>(splitter->widget(0));
    }
    return nullptr;
}