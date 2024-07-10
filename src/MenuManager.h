#ifndef MENUMANAGER_H
#define MENUMANAGER_H

#include <QObject>
#include <KActionCollection>
#include <KRecentFilesAction>

class QMenu;
class MainWindow;
class DocumentManager;
class EditOperations;
class WindowManagement;
class SettingsManagement;

// This class manages the creation and organization of menus in the application
class MenuManager : public QObject
{
    Q_OBJECT

public:
    // Constructor takes pointers to necessary components
    MenuManager(MainWindow* mainWindow, KActionCollection* actionCollection, 
                DocumentManager* documentManager, EditOperations* editOps, 
                WindowManagement* windowMgmt, SettingsManagement* settingsManagement);

    // Set up all menus
    void setupMenus();
    
    // Update the recent files menu
    void updateRecentFilesMenu();

private:
    // Set up the File menu
    void setupFileMenu();
    // Set up the Edit menu
    void setupEditMenu();
    // Set up the View menu
    void setupViewMenu();
    // Set up the Window menu
    void setupWindowMenu();

    // Add new tab action
    void addNewTabAction();
    // Close current tab action
    void closeCurrentTabAction();

    // Pointers to necessary components
    MainWindow* m_mainWindow;
    KActionCollection* m_actionCollection;
    DocumentManager* m_documentManager;
    EditOperations* m_editOps;
    WindowManagement* m_windowMgmt;
    SettingsManagement* m_settingsManagement;

    // Recent files menu action
    KRecentFilesAction* m_recentFilesMenu;

    // Maximum number of recent files to display
    static constexpr int MAX_RECENT_FILES = 10;
};

#endif // MENUMANAGER_H