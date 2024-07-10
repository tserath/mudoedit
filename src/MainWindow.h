#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <KXmlGuiWindow>
#include <QTabWidget>
#include <QMdiArea>
#include <QLoggingCategory>
#include <QAction>

// Forward declarations of classes used in this header
class FileIO;
class DocumentManager;
class AutoSaveManager;
class EditOperations;
class WindowManagement;
class SettingsManagement;
class MenuManager;
class ToolbarManager;
class ZoomManager;

// Declare a logging category for the main window
Q_DECLARE_LOGGING_CATEGORY(mainWindowLog)

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    // Constructor for the main window
    explicit MainWindow(QWidget *parent = nullptr);
    
    // Destructor to clean up resources
    virtual ~MainWindow();
    
    // Method to open a file (used for default text editor functionality)
    void openFile(const QString &filePath);
    void openFile();
    
    // Methods for zoom operations
    void zoomIn();
    void zoomOut();
    
    // Methods for toggling spell check and syntax highlighting
    void toggleSpellCheck(bool enabled);
    void toggleSyntaxHighlighting(bool enabled);
    
    // Method to add a new tab
    void addNewTab();
    
    // Method to close the current tab
    void closeCurrentTab();
    
    void openFileInCurrentTab(const QString &filePath);
    
    // Move windows between tabs
    int getCurrentTabIndex() const;
    int getTabCount() const;
    void moveWindowToTab(QMdiSubWindow* window, int tabIndex);

    // Helper methods to get the active MDI area (moved from private to public)
    QMdiArea* getActiveMdiArea() const;
    QMdiArea* getActiveMdiArea(int index) const;  // New overloaded version

protected:
    // Override the close event to handle unsaved changes
    void closeEvent(QCloseEvent *event) override;
    
    // Override drag enter event to accept file drops
    void dragEnterEvent(QDragEnterEvent *event) override;
    
    // Override drop event to open dropped files
    void dropEvent(QDropEvent *event) override;

private:
    // Helper methods
    void setupUi();
    void setupComponents();
    bool maybeSave();
    
    // UI components
    QTabWidget *m_tabWidget;
    
    // Core operations
    FileIO *m_fileIO;
    DocumentManager *m_documentManager;
    AutoSaveManager *m_autoSaveManager;
    EditOperations *m_editOps;
    WindowManagement *m_windowMgmt;
    SettingsManagement *m_settingsManagement;
    
    // Manager classes
    MenuManager *m_menuManager;
    ToolbarManager *m_toolbarManager;
    ZoomManager *m_zoomManager;
    
    // Menubar toggle
    void setupMenuBarToggle();
    void toggleMenuBar();
    
    // Methods for window geometry
    void saveWindowGeometry();
    void restoreWindowGeometry();
    
    QAction *m_toggleMenuBarAction;
    
    // New methods for tab management
    void setupTabContextMenu();
    void renameTab();
    void updateTabBarVisibility();
};

#endif // MAIN_WINDOW_H