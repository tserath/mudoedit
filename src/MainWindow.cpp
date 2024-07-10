#include "MainWindow.h"
#include "FileIO.h"
#include "DocumentManager.h"
#include "AutoSaveManager.h"
#include "EditOperations.h"
#include "WindowManagement.h"
#include "SettingsManagement.h"
#include "MenuManager.h"
#include "ToolbarManager.h"
#include "ZoomManager.h"
#include <KLocalizedString>
#include <KMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSettings>
#include <QMdiSubWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QStandardPaths>
#include <QPushButton>
#include <QSplitter>
#include <QLoggingCategory>
#include <QToolBar>
#include <QMenu>
#include <QMenuBar>
#include <KActionCollection>
#include <QScreen>
#include <QInputDialog>
#include <QTabBar> 

Q_LOGGING_CATEGORY(mainWindowLog, "mudoedit.mainwindow")


MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent),
      m_tabWidget(nullptr),
      m_fileIO(nullptr),
      m_documentManager(nullptr),
      m_autoSaveManager(nullptr),
      m_editOps(nullptr),
      m_windowMgmt(nullptr),
      m_settingsManagement(nullptr),
      m_menuManager(nullptr),
      m_toolbarManager(nullptr),
      m_zoomManager(nullptr),
      m_toggleMenuBarAction(nullptr)
{
    qCDebug(mainWindowLog) << QStringLiteral("Starting MainWindow constructor");

    setupUi();
    setupComponents();

    // Find the UI description file
    QString rcFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, 
                                            QStringLiteral("kxmlgui5/mudoedit/mudoeditui.rc"));
    if (rcFile.isEmpty()) {
        qCWarning(mainWindowLog) << QStringLiteral("Could not find mudoeditui.rc");
    } else {
        qCDebug(mainWindowLog) << QStringLiteral("Found mudoeditui.rc at:") << rcFile;
    }

    // Set up the GUI from the XML file
    setXMLFile(rcFile);
    setupGUI(Default, rcFile);

    // Add initial tab
    addNewTab();

    // Reopen documents from the last session and apply settings
    if (m_documentManager && m_settingsManagement && m_autoSaveManager) {
        qCDebug(mainWindowLog) << QStringLiteral("Attempting to reopen documents from last session");
        m_documentManager->reopenDocuments();
        m_settingsManagement->applySettings();
        m_autoSaveManager->startAutoSave();
        
        // Update tab bar visibility based on settings
        updateTabBarVisibility();
    } else {
        qCCritical(mainWindowLog) << QStringLiteral("Failed to initialize documentManager, settingsManagement, or autoSaveManager");
    }

    setupMenuBarToggle();
    restoreWindowGeometry();
    connect(qApp, &QApplication::aboutToQuit, this, &MainWindow::saveWindowGeometry);
    setWindowIcon(QIcon(QStringLiteral(":/icons/mudoedit.svg")));
    
    qCDebug(mainWindowLog) << QStringLiteral("Finished MainWindow constructor");
}

MainWindow::~MainWindow()
{
    // Log the start of the destructor
    qCDebug(mainWindowLog) << QStringLiteral("MainWindow destructor called");

    // Save open documents before closing
    if (m_documentManager) {
        qCDebug(mainWindowLog) << QStringLiteral("Saving open documents");
        m_documentManager->saveOpenDocuments();
    } else {
        qCWarning(mainWindowLog) << QStringLiteral("documentManager is null, unable to save open documents");
    }

    // Clean up dynamically allocated objects
    delete m_fileIO;
    delete m_documentManager;
    delete m_autoSaveManager;
    delete m_editOps;
    delete m_windowMgmt;
    delete m_settingsManagement;
    delete m_menuManager;
    delete m_toolbarManager;
    delete m_zoomManager;

    saveWindowGeometry();

    qCDebug(mainWindowLog) << QStringLiteral("MainWindow destructor finished");
}

void MainWindow::setupUi()
{
    // Create a main vertical layout for the central widget
    QVBoxLayout *mainLayout = new QVBoxLayout();
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Create a tab widget
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    // Create "Add New Tab" button
    QPushButton *addTabButton = new QPushButton(QIcon::fromTheme(QStringLiteral("tab-new")), QString(), this);
    addTabButton->setToolTip(i18n("Add New Tab"));
    connect(addTabButton, &QPushButton::clicked, this, &MainWindow::addNewTab);
    m_tabWidget->setCornerWidget(addTabButton, Qt::TopLeftCorner);

    // Add the default tab
    addNewTab();
    m_tabWidget->setTabText(0, QLatin1String("Default"));

    // Set up the tab context menu
    setupTabContextMenu();

    // Enable drag and drop for the main window
    setAcceptDrops(true);

    qCDebug(mainWindowLog) << QStringLiteral("UI setup completed");
}

void MainWindow::addNewTab()
{
    // Create a splitter to allow resizable sections
    QSplitter *splitter = new QSplitter(Qt::Horizontal);

    // Create and set up the MDI Area
    QMdiArea *mdiArea = new QMdiArea(splitter);
    if (!mdiArea) {
        qCCritical(mainWindowLog) << QStringLiteral("Failed to create QMdiArea");
        return;
    }

    // Set scrollbar policies for the MDI Area
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Add the MDI Area to the splitter
    splitter->addWidget(mdiArea);

    // Set initial sizes for the splitter
    splitter->setSizes(QList<int>() << width());
    splitter->setStretchFactor(0, 1);

    // Add the splitter to a new tab
    int tabIndex = m_tabWidget->count();
    QString tabName = (tabIndex == 0) ? QStringLiteral("Default") : QStringLiteral("Tab %1").arg(tabIndex);
    m_tabWidget->addTab(splitter, tabName);

    // Set the new tab as active
    m_tabWidget->setCurrentIndex(tabIndex);

    qCDebug(mainWindowLog) << QStringLiteral("Added new tab with index:") << tabIndex;
}

void MainWindow::closeCurrentTab()
{
    // Get the index of the current tab
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex != -1) {
        // Remove the tab at the current index
        m_tabWidget->removeTab(currentIndex);
        qCDebug(mainWindowLog) << QStringLiteral("Closed tab with index:") << currentIndex;
    }
}

QMdiArea* MainWindow::getActiveMdiArea() const
{
    // Existing implementation
    QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->currentWidget());
    if (splitter) {
        return qobject_cast<QMdiArea*>(splitter->widget(0));
    }
    return nullptr;
}

QMdiArea* MainWindow::getActiveMdiArea(int index) const
{
    QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->widget(index));
    if (splitter) {
        return qobject_cast<QMdiArea*>(splitter->widget(0));
    }
    return nullptr;
}

void MainWindow::moveWindowToTab(QMdiSubWindow* window, int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= m_tabWidget->count()) {
        return;
    }

    QMdiArea* sourceMdiArea = qobject_cast<QMdiArea*>(window->mdiArea());
    QMdiArea* targetMdiArea = getActiveMdiArea(tabIndex);

    if (sourceMdiArea && targetMdiArea && sourceMdiArea != targetMdiArea) {
        sourceMdiArea->removeSubWindow(window);
        targetMdiArea->addSubWindow(window);
        window->show();
        m_tabWidget->setCurrentIndex(tabIndex);
        targetMdiArea->setActiveSubWindow(window);
    }
}


void MainWindow::setupComponents()
{
    // Create settings object for the application
    QSettings *settings = new QSettings(QStringLiteral("erateth"), QStringLiteral("mudoedit"), this);

    // Initialize various operation classes
    m_settingsManagement = new SettingsManagement(m_tabWidget, settings, this);
    m_fileIO = new FileIO(this);
    m_documentManager = new DocumentManager(this, m_tabWidget, m_fileIO, m_settingsManagement, this);
    m_autoSaveManager = new AutoSaveManager(m_tabWidget, m_documentManager, settings, this);
    m_editOps = new EditOperations(m_tabWidget, this);
    m_windowMgmt = new WindowManagement(m_tabWidget, this);

     // Initialize manager classes
    m_menuManager = new MenuManager(this, actionCollection(), m_documentManager, m_editOps, m_windowMgmt, m_settingsManagement);
    m_toolbarManager = new ToolbarManager(this, actionCollection());
    m_zoomManager = new ZoomManager(m_tabWidget, this);

    // Set up menus and toolbar
    m_menuManager->setupMenus();
    m_toolbarManager->setupMainToolbar();

// Check if all components were initialized successfully
    if (!m_fileIO || !m_documentManager || !m_autoSaveManager || !m_editOps || !m_windowMgmt || !m_settingsManagement || 
        !m_menuManager || !m_toolbarManager || !m_zoomManager) {
        qCCritical(mainWindowLog) << QStringLiteral("Failed to initialize one or more components");
    } else {
        qCDebug(mainWindowLog) << QStringLiteral("All components initialized successfully");
    }

    // Connect signals and slots
    connect(m_documentManager, &DocumentManager::fileOpened, m_menuManager, &MenuManager::updateRecentFilesMenu);
    
    // Connect the settingsChanged signal to updateTabBarVisibility
    connect(m_settingsManagement, &SettingsManagement::settingsChanged, this, &MainWindow::updateTabBarVisibility);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qCDebug(mainWindowLog) << "closeEvent called";
    if (maybeSave()) {
        if (m_documentManager) {
            qCDebug(mainWindowLog) << "Saving open documents";
            m_documentManager->saveOpenDocuments();
        }
        
        event->accept();
        qCDebug(mainWindowLog) << "closeEvent accepted";
    } else {
        event->ignore();
        qCDebug(mainWindowLog) << "closeEvent ignored";
    }

    saveWindowGeometry();
}

bool MainWindow::maybeSave()
{
    qCDebug(mainWindowLog) << QStringLiteral("maybeSave called");
    if (!m_documentManager) {
        qCCritical(mainWindowLog) << QStringLiteral("documentManager is null in maybeSave");
        return true;  // Return true to allow closing if we can't check for unsaved changes
    }

    m_documentManager->logAllDocumentStates(QStringLiteral("Before maybeSave"));

    QList<QMdiSubWindow*> modifiedWindows = m_documentManager->getModifiedWindows();

    qCDebug(mainWindowLog) << QStringLiteral("Modified windows:") << modifiedWindows.size();

    if (modifiedWindows.isEmpty()) {
        qCDebug(mainWindowLog) << QStringLiteral("No modified documents, returning true");
        return true;
    }

    QString message = i18np("There is %1 document with unsaved changes. Do you want to save the changes before closing?",
                            "There are %1 documents with unsaved changes. Do you want to save the changes before closing?",
                            modifiedWindows.size());

    int ret = KMessageBox::questionTwoActionsCancel(
        this,
        message,
        i18n("Save Changes"),
        KStandardGuiItem::save(),
        KStandardGuiItem::discard(),
        KStandardGuiItem::cancel(),
        QStringLiteral("AskSaveOnExit")
    );

    switch (ret) {
        case KMessageBox::PrimaryAction:
            qCDebug(mainWindowLog) << QStringLiteral("User chose to save changes");
            return m_documentManager->saveAllModifiedDocuments(modifiedWindows);
        case KMessageBox::SecondaryAction:
            qCDebug(mainWindowLog) << QStringLiteral("User chose to discard changes");
            return true;
        case KMessageBox::Cancel:
        default:
            qCDebug(mainWindowLog) << QStringLiteral("User cancelled the close operation");
            return false;
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (const QUrl &url : urlList) {
            if (url.isLocalFile()) {
                QString filePath = url.toLocalFile();
                openFileInCurrentTab(filePath);
            }
        }
    }
}


void MainWindow::openFileInCurrentTab(const QString &filePath)
{
    qCDebug(mainWindowLog) << "Attempting to open file in current tab:" << filePath;

    if (!m_documentManager) {
        qCCritical(mainWindowLog) << QStringLiteral("DocumentManager not initialized. Cannot open file.");
        return;
    }

    QMdiArea *activeMdiArea = getActiveMdiArea();
    if (!activeMdiArea) {
        qCCritical(mainWindowLog) << QStringLiteral("No active MDI area. Cannot open file.");
        return;
    }

    QMdiSubWindow *subWindow = m_documentManager->openFile(filePath);
    
    if (subWindow) {
        activeMdiArea->setActiveSubWindow(subWindow);
        qCDebug(mainWindowLog) << QStringLiteral("File opened successfully in current tab:") << filePath;
    } else {
        qCWarning(mainWindowLog) << QStringLiteral("Failed to open file:") << filePath;
    }
}

void MainWindow::toggleSpellCheck(bool enabled)
{
    m_settingsManagement->setSpellCheckEnabled(enabled);
    m_settingsManagement->applySettings();
    m_settingsManagement->saveSettings();
}

void MainWindow::toggleSyntaxHighlighting(bool enabled)
{
    m_settingsManagement->setSyntaxHighlightingEnabled(enabled);
    m_settingsManagement->applySettings();
    m_settingsManagement->saveSettings();
}

void MainWindow::zoomIn()
{
    m_zoomManager->zoomIn();
}

void MainWindow::zoomOut()
{
    m_zoomManager->zoomOut();
}

void MainWindow::openFile(const QString &filePath)
{
    qCDebug(mainWindowLog) << "Attempting to open file:" << filePath;

    // Check if documentManager is initialized
    if (!m_documentManager) {
        qCCritical(mainWindowLog) << QStringLiteral("DocumentManager not initialized. Cannot open file.");
        return;
    }

    // Ensure we're using the default tab (index 0)
    if (m_tabWidget->currentIndex() != 0) {
        m_tabWidget->setCurrentIndex(0);
    }

    QMdiArea *activeMdiArea = getActiveMdiArea();
    if (!activeMdiArea) {
        qCCritical(mainWindowLog) << QStringLiteral("No active MDI area. Cannot open file.");
        return;
    }

    // Use DocumentManager to open the file
    QMdiSubWindow *subWindow = m_documentManager->openFile(filePath);
    
    if (subWindow) {
        // If the file was opened successfully, activate the subwindow
        activeMdiArea->setActiveSubWindow(subWindow);
        qCDebug(mainWindowLog) << QStringLiteral("File opened successfully:") << filePath;
    } else {
        qCWarning(mainWindowLog) << QStringLiteral("Failed to open file:") << filePath;
    }
}

// Implementation for opening a file using a file dialog
void MainWindow::openFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, i18n("Open File"), QString(), i18n("All Files (*)"));
    if (!filePath.isEmpty()) {
        openFile(filePath);
    }
}

void MainWindow::setupMenuBarToggle()
{
    // Create the toggle action
    m_toggleMenuBarAction = new QAction(this);
    
    // Set up the keyboard shortcut (Ctrl+M is standard for KDE)
    m_toggleMenuBarAction->setShortcut(Qt::CTRL | Qt::Key_M);
    
    // Connect the action to the toggle function
    connect(m_toggleMenuBarAction, &QAction::triggered, this, &MainWindow::toggleMenuBar);
    
    // Add the action to the action collection
    actionCollection()->addAction(QStringLiteral("toggle_menubar"), m_toggleMenuBarAction);
}

void MainWindow::toggleMenuBar()
{
    // Toggle the menubar visibility
    menuBar()->setVisible(!menuBar()->isVisible());
}

void MainWindow::saveWindowGeometry()
{
    // Save the current window geometry to settings
    QSettings settings(QStringLiteral("erateth"), QStringLiteral("mudoedit"));
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    settings.setValue(QStringLiteral("windowState"), saveState());
    
    qDebug() << "Saving window geometry:" << size() << "at position:" << pos();
    settings.sync(); // Ensure settings are written to disk immediately
}

void MainWindow::restoreWindowGeometry()
{
    // Restore the window geometry from settings
    QSettings settings(QStringLiteral("erateth"), QStringLiteral("mudoedit"));
    
    qDebug() << "Attempting to restore window geometry";
    
    // Restore geometry
    if (settings.contains(QStringLiteral("geometry"))) {
        restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
        qDebug() << "Restored geometry from settings. New size:" << size() << "at position:" << pos();
    } else {
        // Set a default size if no saved geometry exists
        const QRect availableGeometry = QApplication::primaryScreen()->availableGeometry();
        resize(availableGeometry.width() * 2 / 3, availableGeometry.height() * 2 / 3);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
        qDebug() << "No saved geometry found. Set default size:" << size() << "at position:" << pos();
    }
    
    // Restore window state
    if (settings.contains(QStringLiteral("windowState"))) {
        restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
        qDebug() << "Restored window state from settings";
    } else {
        qDebug() << "No saved window state found";
    }
}

void MainWindow::setupTabContextMenu()
{
    // Create a context menu for the tab bar
    m_tabWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tabWidget, &QWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        int tabIndex = m_tabWidget->tabBar()->tabAt(pos);
        if (tabIndex != -1 && tabIndex != 0) {  // Ignore the default tab
            QMenu contextMenu(this);
            QAction *renameAction = contextMenu.addAction(QLatin1String("Rename Tab"));
            connect(renameAction, &QAction::triggered, this, &MainWindow::renameTab);
            contextMenu.exec(m_tabWidget->tabBar()->mapToGlobal(pos));
        }
    });
}

void MainWindow::renameTab()
{
    int currentIndex = m_tabWidget->currentIndex();
    if (currentIndex != 0) {  // Don't rename the default tab
        bool ok;
        QString newName = QInputDialog::getText(this, QLatin1String("Rename Tab"),
                                                QLatin1String("Enter new tab name:"),
                                                QLineEdit::Normal,
                                                m_tabWidget->tabText(currentIndex), &ok);
        if (ok && !newName.isEmpty()) {
            m_tabWidget->setTabText(currentIndex, newName);
        }
    }
}

void MainWindow::updateTabBarVisibility()
{
    bool showTabBar = m_settingsManagement->isTabBarVisible();
    m_tabWidget->tabBar()->setVisible(showTabBar);
    
    // Update the text of the default tab
    if (!showTabBar) {
        m_tabWidget->setTabText(0, QLatin1String("mudoedit"));
    } else {
        m_tabWidget->setTabText(0, QLatin1String("Default"));
    }
}

int MainWindow::getCurrentTabIndex() const
{
    return m_tabWidget->currentIndex();
}

int MainWindow::getTabCount() const
{
    return m_tabWidget->count();
}

