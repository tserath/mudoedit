#ifndef TOOLBARMANAGER_H
#define TOOLBARMANAGER_H

#include <KActionCollection>

class KToolBar;
class MainWindow;

// This class manages the creation and organization of toolbars in the application
class ToolbarManager
{
public:
    // Constructor takes pointers to necessary components
    explicit ToolbarManager(MainWindow* mainWindow, KActionCollection* actionCollection);

    // Set up the main toolbar
    void setupMainToolbar();

private:
    // Pointers to necessary components
    MainWindow* m_mainWindow;
    KActionCollection* m_actionCollection;
};

#endif // TOOLBARMANAGER_H