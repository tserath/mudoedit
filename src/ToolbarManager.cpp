#include "ToolbarManager.h"
#include "MainWindow.h"
#include <KToolBar>
#include <KXmlGuiWindow>
#include <QAction>
#include <KActionCollection>

// Constructor for the ToolbarManager class
ToolbarManager::ToolbarManager(MainWindow* mainWindow, KActionCollection* actionCollection)
    : m_mainWindow(mainWindow),
      m_actionCollection(actionCollection)
{
}

// Set up the main toolbar with customization options
void ToolbarManager::setupMainToolbar()
{
    // Get the main toolbar or create a new one if it doesn't exist
    KToolBar* mainToolBar = m_mainWindow->toolBar(QStringLiteral("mainToolBar"));
    if (!mainToolBar) {
        // Create a new KToolBar instead of QToolBar for KDE integration
        mainToolBar = new KToolBar(QStringLiteral("mainToolBar"), m_mainWindow, Qt::TopToolBarArea);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        
        // Set toolbar to be movable and floatable
        mainToolBar->setMovable(true);
        mainToolBar->setFloatable(true);
        
        // Allow the toolbar to be hidden
        mainToolBar->setAllowedAreas(Qt::AllToolBarAreas);
        
        // Add the toolbar to the main window
        m_mainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
    }

    // Enable toolbar context menu for customization
    mainToolBar->setContextMenuPolicy(Qt::DefaultContextMenu);

    // Add actions to the toolbar
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("file_new")));
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("file_open")));
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("file_save")));
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("edit_cut")));
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("edit_copy")));
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("edit_paste")));
    mainToolBar->addSeparator();
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("toggle_spell_check")));
    mainToolBar->addAction(m_actionCollection->action(QStringLiteral("toggle_syntax_highlight")));

    // Make sure the toolbar is visible
    mainToolBar->show();

    // Set up XML GUI for toolbar customization
    m_mainWindow->setupGUI(KXmlGuiWindow::Default, QStringLiteral("mudoeditui.rc"));
}