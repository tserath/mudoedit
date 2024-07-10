#include "CustomMdiSubWindow.h"
#include "MainWindow.h"
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QToolTip>
#include <QCursor>
#include <QWindow>

CustomMdiSubWindow::CustomMdiSubWindow(MainWindow* mainWindow, QWidget* parent)
    : QMdiSubWindow(parent), m_mainWindow(mainWindow)
{
}

void CustomMdiSubWindow::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);

    // Add standard MDI window actions
    QAction* restoreAction = menu.addAction(tr("&Restore"));
    QAction* moveAction = menu.addAction(tr("&Move"));
    QAction* sizeAction = menu.addAction(tr("&Size"));
    QAction* minimizeAction = menu.addAction(tr("Mi&nimize"));
    QAction* maximizeAction = menu.addAction(tr("Ma&ximize"));
    menu.addSeparator();
    QAction* closeAction = menu.addAction(tr("&Close"));

    // Set enabled state based on current window state
    restoreAction->setEnabled(isMaximized() || isMinimized());
    maximizeAction->setEnabled(!isMaximized());
    minimizeAction->setEnabled(!isMinimized());

    // Connect actions
    connect(restoreAction, &QAction::triggered, this, &QMdiSubWindow::showNormal);
    connect(moveAction, &QAction::triggered, this, &CustomMdiSubWindow::startSystemMove);
    connect(sizeAction, &QAction::triggered, this, &CustomMdiSubWindow::startSystemResize);
    connect(minimizeAction, &QAction::triggered, this, &QMdiSubWindow::showMinimized);
    connect(maximizeAction, &QAction::triggered, this, &QMdiSubWindow::showMaximized);
    connect(closeAction, &QAction::triggered, this, &QMdiSubWindow::close);

    // Add "Move to Tab" submenu
    menu.addSeparator();
    QMenu* moveToTabMenu = menu.addMenu(tr("Move to Tab"));
    int currentTabIndex = m_mainWindow->getCurrentTabIndex();
    int tabCount = m_mainWindow->getTabCount();
    
    for (int i = 0; i < tabCount; ++i) {
        if (i != currentTabIndex) {
            QString tabName = (i == 0) ? tr("Default") : tr("Tab %1").arg(i);
            QAction* action = moveToTabMenu->addAction(tabName);
            connect(action, &QAction::triggered, [this, i]() {
                m_mainWindow->moveWindowToTab(this, i);
            });
        }
    }
    
    menu.exec(event->globalPos());
}

void CustomMdiSubWindow::startSystemMove()
{
    QWidget* widget = parentWidget();
    if (widget) {
        widget->windowHandle()->startSystemMove();
    }
}

void CustomMdiSubWindow::startSystemResize()
{
    QWidget* widget = parentWidget();
    if (widget) {
        widget->windowHandle()->startSystemResize(Qt::Edges(Qt::BottomEdge | Qt::RightEdge));
    }
}