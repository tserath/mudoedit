#include "MenuManager.h"
#include "MainWindow.h"
#include "DocumentManager.h"
#include "EditOperations.h"
#include "WindowManagement.h"
#include "SettingsManagement.h"
#include <KLocalizedString>
#include <KStandardAction>
#include <KToggleAction>
#include <QMenu>
#include <QMenuBar>
#include <KSharedConfig>
#include <KConfigGroup>

MenuManager::MenuManager(MainWindow* mainWindow, KActionCollection* actionCollection, 
                         DocumentManager* documentManager, EditOperations* editOps, 
                         WindowManagement* windowMgmt, SettingsManagement* settingsManagement)
    : m_mainWindow(mainWindow), m_actionCollection(actionCollection), 
      m_documentManager(documentManager), m_editOps(editOps), 
      m_windowMgmt(windowMgmt), m_settingsManagement(settingsManagement)
{
    setupMenus();
}

void MenuManager::setupMenus()
{
    setupFileMenu();
    setupEditMenu();
    setupViewMenu();
    setupWindowMenu();

    // Add Settings menu
    QMenu *settingsMenu = m_mainWindow->menuBar()->addMenu(i18n("&Settings"));
    settingsMenu->setObjectName(QStringLiteral("settings"));

    QAction *preferencesAction = new QAction(i18n("&Preferences"), m_mainWindow);
    preferencesAction->setObjectName(QStringLiteral("options_preferences"));
    m_actionCollection->addAction(QStringLiteral("options_preferences"), preferencesAction);
    settingsMenu->addAction(preferencesAction);

    // Connect the settings action to open the settings dialog
    connect(preferencesAction, &QAction::triggered, m_settingsManagement, &SettingsManagement::showSettingsDialog);
}

void MenuManager::setupFileMenu()
{
    // Create standard file actions
    KStandardAction::openNew(m_documentManager, &DocumentManager::newDocument, m_actionCollection);
    KStandardAction::open(m_mainWindow, [this]() { m_documentManager->openFile(); }, m_actionCollection);
    KStandardAction::save(m_documentManager, &DocumentManager::saveFile, m_actionCollection);
    KStandardAction::saveAs(m_documentManager, &DocumentManager::saveFile, m_actionCollection);
    KStandardAction::quit(m_mainWindow, &MainWindow::close, m_actionCollection);

    // Create custom "Save All" action
    QAction* saveAllAction = new QAction(QIcon::fromTheme(QStringLiteral("document-save-all")), i18n("Save &All"), this);
    m_actionCollection->addAction(QStringLiteral("file_save_all"), saveAllAction);
    connect(saveAllAction, &QAction::triggered, m_documentManager, &DocumentManager::saveAllFiles);

    // Set up recent files menu
    m_recentFilesMenu = new KRecentFilesAction(i18n("Recent Files"), this);
    m_actionCollection->addAction(QStringLiteral("recent_files"), m_recentFilesMenu);
    connect(m_recentFilesMenu, &KRecentFilesAction::urlSelected, this, [this](const QUrl &url) {
        if (m_documentManager) {
            m_documentManager->openFile(url.toLocalFile());
        }
    });

    // Load recent files from settings
    m_recentFilesMenu->loadEntries(KSharedConfig::openConfig()->group(QStringLiteral("RecentFiles")));
}

void MenuManager::setupEditMenu()
{
    // Create standard edit actions
    KStandardAction::undo(m_editOps, &EditOperations::undo, m_actionCollection);
    KStandardAction::redo(m_editOps, &EditOperations::redo, m_actionCollection);
    KStandardAction::cut(m_editOps, &EditOperations::cut, m_actionCollection);
    KStandardAction::copy(m_editOps, &EditOperations::copy, m_actionCollection);
    KStandardAction::paste(m_editOps, &EditOperations::paste, m_actionCollection);
}

void MenuManager::setupViewMenu()
{
    // Create View menu if it doesn't exist
    QMenu* viewMenu = m_mainWindow->menuBar()->findChild<QMenu*>(QStringLiteral("view"));
    if (!viewMenu) {
        viewMenu = m_mainWindow->menuBar()->addMenu(i18n("&View"));
        viewMenu->setObjectName(QStringLiteral("view"));
    }

    // Add zoom actions
    KStandardAction::zoomIn(m_mainWindow, &MainWindow::zoomIn, m_actionCollection);
    KStandardAction::zoomOut(m_mainWindow, &MainWindow::zoomOut, m_actionCollection);

    // Add spell check toggle action
    KToggleAction* spellCheckAction = new KToggleAction(QIcon::fromTheme(QStringLiteral("tools-check-spelling")), i18n("Enable &Spell Checking"), this);
    m_actionCollection->addAction(QStringLiteral("toggle_spell_check"), spellCheckAction);
    spellCheckAction->setChecked(m_settingsManagement->isSpellCheckEnabled());
    connect(spellCheckAction, &KToggleAction::triggered, m_mainWindow, &MainWindow::toggleSpellCheck);
    viewMenu->addAction(spellCheckAction);

    // Add syntax highlighting toggle action
    QAction* syntaxHighlightAction = new QAction(QIcon::fromTheme(QStringLiteral("code-context")), i18n("Enable &Syntax Highlighting"), this);
    m_actionCollection->addAction(QStringLiteral("toggle_syntax_highlight"), syntaxHighlightAction);
    syntaxHighlightAction->setCheckable(true);
    syntaxHighlightAction->setChecked(m_settingsManagement->isSyntaxHighlightingEnabled());
    connect(syntaxHighlightAction, &QAction::toggled, m_mainWindow, &MainWindow::toggleSyntaxHighlighting);
    viewMenu->addAction(syntaxHighlightAction);
}

void MenuManager::setupWindowMenu()
{
    // Create Window menu
    QMenu* windowMenu = m_mainWindow->menuBar()->addMenu(i18n("&Window"));

    // Add window management actions
    QAction* tileAction = new QAction(QIcon::fromTheme(QStringLiteral("view-split-left-right")), i18n("&Tile"), this);
    m_actionCollection->addAction(QStringLiteral("window_tile"), tileAction);
    connect(tileAction, &QAction::triggered, m_windowMgmt, &WindowManagement::tileWindows);
    windowMenu->addAction(tileAction);

    QAction* cascadeAction = new QAction(QIcon::fromTheme(QStringLiteral("view-list-details")), i18n("&Cascade"), this);
    m_actionCollection->addAction(QStringLiteral("window_cascade"), cascadeAction);
    connect(cascadeAction, &QAction::triggered, m_windowMgmt, &WindowManagement::cascadeWindows);
    windowMenu->addAction(cascadeAction);

    QAction* minimizeAllAction = new QAction(QIcon::fromTheme(QStringLiteral("window-minimize")), i18n("&Minimize All"), this);
    m_actionCollection->addAction(QStringLiteral("window_minimize_all"), minimizeAllAction);
    connect(minimizeAllAction, &QAction::triggered, m_windowMgmt, &WindowManagement::minimizeAllWindows);
    windowMenu->addAction(minimizeAllAction);

    QAction* closeAllAction = new QAction(QIcon::fromTheme(QStringLiteral("window-close")), i18n("Close &All"), this);
    m_actionCollection->addAction(QStringLiteral("window_close_all"), closeAllAction);
    connect(closeAllAction, &QAction::triggered, m_windowMgmt, &WindowManagement::closeAllWindows);
    windowMenu->addAction(closeAllAction);
    
    // Add tab management actions
    windowMenu->addSeparator();
    addNewTabAction();
    closeCurrentTabAction();
    windowMenu->addAction(m_actionCollection->action(QStringLiteral("new_tab")));
    windowMenu->addAction(m_actionCollection->action(QStringLiteral("close_tab")));
}



void MenuManager::addNewTabAction()
{
    QAction* newTabAction = new QAction(QIcon::fromTheme(QStringLiteral("tab-new")), i18n("New Tab"), this);
    m_actionCollection->addAction(QStringLiteral("new_tab"), newTabAction);
    connect(newTabAction, &QAction::triggered, m_mainWindow, &MainWindow::addNewTab);
}

void MenuManager::closeCurrentTabAction()
{
    QAction* closeTabAction = new QAction(QIcon::fromTheme(QStringLiteral("tab-close")), i18n("Close Tab"), this);
    m_actionCollection->addAction(QStringLiteral("close_tab"), closeTabAction);
    connect(closeTabAction, &QAction::triggered, m_mainWindow, &MainWindow::closeCurrentTab);
}

void MenuManager::updateRecentFilesMenu()
{
    // Check if both documentManager and recentFilesMenu are available
    if (m_documentManager && m_recentFilesMenu) {
        // Clear the existing menu items
        m_recentFilesMenu->clear();
        // Get the list of recent files from the document manager
        QStringList recentFiles = m_documentManager->getRecentFiles();
        // Add each recent file to the menu
        for (const QString &file : recentFiles) {
            m_recentFilesMenu->addUrl(QUrl::fromLocalFile(file), file);
        }
    }
}

