// Qt includes
#include <QApplication>
#include <QFileDialog>
#include <QLoggingCategory>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>
#include <QSplitter>
#include <QStyle>
#include <QTabWidget>
#include <QMimeDatabase>
#include <QMimeType>
#include <QVBoxLayout>
//#include <QHBoxLayout>
#include <QPushButton>
#include <QDialog>
#include <KStandardGuiItem>

// KDE includes
#include <KFileWidget>
#include <KLocalizedString>
#include <KMessageBox>
#include <KTextEdit>
//#include <KIO/OpenFileManagerWindowJob>
#include <KFileFilter>

// Standard library includes
//#include <algorithm>

// Local includes
#include "DocumentManager.h"
#include "FileIO.h"
#include "MainWindow.h"
#include "SettingsManagement.h"
#include "CustomMdiSubWindow.h"

Q_LOGGING_CATEGORY(docManagerLog, "mudoedit.documentmanager")

DocumentManager::DocumentManager(MainWindow* mainWindow, QTabWidget *tabWidget, FileIO *fileIO, SettingsManagement *settingsManagement, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_tabWidget(tabWidget)
    , m_fileIO(fileIO)
    , m_settingsManagement(settingsManagement)
{
}

void DocumentManager::newDocument()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (!mdiArea) {
        qCCritical(docManagerLog) << QStringLiteral("No active MDI area. Cannot create new document.");
        return;
    }

    KTextEdit *textEdit = new KTextEdit;
    CustomMdiSubWindow *subWindow = new CustomMdiSubWindow(m_mainWindow, mdiArea);
    subWindow->setWidget(textEdit);
    mdiArea->addSubWindow(subWindow);
    setupTextEdit(textEdit);
    setupSubWindow(subWindow);
    subWindow->setWindowTitle(i18n("Untitled"));
    subWindow->resize(600, 400);
    subWindow->show();
    logDocumentState(textEdit, QStringLiteral("newDocument"));
    logAllDocumentStates(QStringLiteral("After newDocument"));
}

QMdiSubWindow* DocumentManager::openFile(const QString &filePath)
{
    qCDebug(docManagerLog) << "Opening file:" << filePath;

    if (!m_fileIO->isFileReadable(filePath)) {
        qCWarning(docManagerLog) << "File is not readable:" << filePath;
        KMessageBox::error(m_tabWidget, i18n("Could not open file %1", filePath));
        return nullptr;
    }

    QString content = m_fileIO->readFile(filePath);

    QMdiArea *mdiArea = getActiveMdiArea();
    if (!mdiArea) {
        qCCritical(docManagerLog) << "No active MDI area. Cannot open file.";
        return nullptr;
    }

    // Create a CustomMdiSubWindow instead of a regular QMdiSubWindow
    KTextEdit *textEdit = new KTextEdit;
    CustomMdiSubWindow *subWindow = new CustomMdiSubWindow(m_mainWindow, mdiArea);
    subWindow->setWidget(textEdit);
    mdiArea->addSubWindow(subWindow);

    textEdit->setPlainText(content);
    setupTextEdit(textEdit, filePath);
    subWindow->resize(600, 400);
    subWindow->show();
    textEdit->document()->setModified(false);
    subWindow->setProperty("fullFilePath", filePath);
    
    qCDebug(docManagerLog) << "File opened successfully:" << filePath;
    
    Q_EMIT fileOpened(filePath);
    
    return subWindow;
}
void DocumentManager::openFile()
{
    // Create a QFileDialog with the parent set to m_tabWidget
    QFileDialog dialog(m_tabWidget);
    
    // Set the dialog title
    dialog.setWindowTitle(i18n("Open File"));
    
    // Set the starting directory to the user's home folder
    dialog.setDirectory(QDir::homePath());
    
    // Set the file mode to allow selection of existing files
    dialog.setFileMode(QFileDialog::ExistingFile);
    
    // Create a QMimeDatabase to work with MIME types
    QMimeDatabase mimeDb;
    
    // Get all MIME types that represent text files
    QStringList mimeTypeFilters;
    for (const QMimeType &mimeType : mimeDb.allMimeTypes()) {
        if (mimeType.name().startsWith(QStringLiteral("text/"))) {
            mimeTypeFilters.append(mimeType.name());
        }
    }
    
    // Set the MIME type filters for the dialog
    dialog.setMimeTypeFilters(mimeTypeFilters);
    
    // Set "All Text Files" as the default filter
    dialog.selectMimeTypeFilter(QStringLiteral("text/plain"));
    
    // Add an "All Files" option
    dialog.setNameFilter(i18n("All Files (*)"));
    
    // Show the dialog and get the selected file
    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        if (!files.isEmpty()) {
            // Open the selected file
            openFile(files.first());
        }
    }
}


bool DocumentManager::saveFile()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (!mdiArea) {
        qCCritical(docManagerLog) << QStringLiteral("No active MDI area. Cannot save file.");
        return false;
    }

    KTextEdit *textEdit = qobject_cast<KTextEdit*>(mdiArea->activeSubWindow()->widget());
    if (!textEdit) return false;

    QString filePath = mdiArea->activeSubWindow()->property("fullFilePath").toString();
    
    if (filePath.isEmpty() || filePath == i18n("Untitled")) {
        filePath = QFileDialog::getSaveFileName(m_tabWidget, i18n("Save File"), 
                                                QDir::homePath(),
                                                i18n("Text Files (*.txt);;All Files (*)"));
        if (filePath.isEmpty()) return false;
    }

    if (m_fileIO->writeFile(filePath, textEdit->toPlainText())) {
        textEdit->document()->setModified(false);
        QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
        if (window) {
            window->setWindowTitle(QFileInfo(filePath).fileName());
            window->setProperty("fullFilePath", filePath);
        }
        logDocumentState(textEdit, QStringLiteral("saveFile"));
        logAllDocumentStates(QStringLiteral("After saveFile"));
        return true;
    }
    return false;
}

void DocumentManager::saveAllFiles()
{
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QMdiArea *mdiArea = getActiveMdiArea(i);
        if (!mdiArea) continue;

        for (QMdiSubWindow *window : mdiArea->subWindowList()) {
            KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget());
            if (textEdit) {
                mdiArea->setActiveSubWindow(window);
                saveFile();
            }
        }
    }
    logAllDocumentStates(QStringLiteral("After saveAllFiles"));
}

QList<QMdiSubWindow*> DocumentManager::getModifiedWindows()
{
    QList<QMdiSubWindow*> modifiedWindows;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QMdiArea *mdiArea = getActiveMdiArea(i);
        if (!mdiArea) continue;

        for (QMdiSubWindow *window : mdiArea->subWindowList()) {
            if (KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget())) {
                if (textEdit->document()->isModified()) {
                    modifiedWindows.append(window);
                    qCDebug(docManagerLog) << "Modified document found:" << window->windowTitle()
                                           << "Content:" << textEdit->toPlainText().left(20) + QStringLiteral("...");
                }
            }
        }
    }
    qCDebug(docManagerLog) << "Total modified documents:" << modifiedWindows.size();
    return modifiedWindows;
}

bool DocumentManager::saveAllModifiedDocuments(const QList<QMdiSubWindow*>& windows)
{
    for (QMdiSubWindow *window : windows) {
        QMdiArea *mdiArea = qobject_cast<QMdiArea*>(window->parentWidget());
        if (mdiArea) {
            mdiArea->setActiveSubWindow(window);
            if (!saveFile()) {
                return false;
            }
        }
    }
    logAllDocumentStates(QStringLiteral("After saveAllModifiedDocuments"));
    return true;
}

void DocumentManager::saveOpenDocuments()
{
    QStringList openFiles;
    QVariantList windowGeometries;
    QList<int> tabIndices;

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QMdiArea *mdiArea = getActiveMdiArea(i);
        if (!mdiArea) continue;

        for (QMdiSubWindow *window : mdiArea->subWindowList()) {
            KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget());
            if (textEdit) {
                QString filePath = window->property("fullFilePath").toString();
                if (!filePath.isEmpty()) {
                    openFiles << filePath;
                    windowGeometries << window->saveGeometry();
                    tabIndices << i;
                }
            }
        }
    }
    
    // Save these lists to QSettings
    QSettings settings(QStringLiteral("erateth"), QStringLiteral("mudoedit"));
    settings.setValue(QStringLiteral("openFiles"), openFiles);
    settings.setValue(QStringLiteral("windowGeometries"), windowGeometries);
    settings.setValue(QStringLiteral("tabIndices"), QVariant::fromValue(tabIndices));
    
    qCDebug(docManagerLog) << "Saved" << openFiles.size() << "open documents";
    logAllDocumentStates(QStringLiteral("After saveOpenDocuments"));
}

void DocumentManager::reopenDocuments()
{
    QSettings settings(QStringLiteral("erateth"), QStringLiteral("mudoedit"));
    QStringList openFiles = settings.value(QStringLiteral("openFiles")).toStringList();
    QVariantList windowGeometries = settings.value(QStringLiteral("windowGeometries")).toList();
    QList<int> tabIndices = settings.value(QStringLiteral("tabIndices")).value<QList<int>>();
    
    qCDebug(docManagerLog) << "Attempting to reopen" << openFiles.size() << "documents";
    
    // First, create the necessary tabs
    int maxTabIndex = tabIndices.isEmpty() ? 0 : *std::max_element(tabIndices.begin(), tabIndices.end());
    while (m_tabWidget->count() <= maxTabIndex) {
        m_mainWindow->addNewTab();
    }
    
    // Now reopen the documents in their respective tabs
    for (int i = 0; i < openFiles.size(); ++i) {
        const QString &filePath = openFiles[i];
        int tabIndex = (i < tabIndices.size()) ? tabIndices[i] : 0;
        
        qCDebug(docManagerLog) << "Reopening file:" << filePath << "in tab" << tabIndex;
        
        m_tabWidget->setCurrentIndex(tabIndex);
        QMdiSubWindow *window = openFile(filePath);
        if (window && i < windowGeometries.size()) {
            window->restoreGeometry(windowGeometries[i].toByteArray());
            qCDebug(docManagerLog) << "Restored geometry for" << filePath;
        } else if (!window) {
            qCWarning(docManagerLog) << "Failed to reopen file:" << filePath;
        }
    }
    
    // If no documents were reopened, ensure there's at least one empty tab
    if (m_tabWidget->count() == 0) {
        m_mainWindow->addNewTab();
    }
    
    logAllDocumentStates(QStringLiteral("After reopenDocuments"));
}

void DocumentManager::logDocumentState(KTextEdit* textEdit, const QString& action)
{
    QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
    QString title = window ? window->windowTitle() : QStringLiteral("Unknown");
    qCDebug(docManagerLog) << action << "- Document:" << title
                           << "Modified:" << textEdit->document()->isModified()
                           << "Content:" << textEdit->toPlainText().left(20) + QStringLiteral("...");
}

void DocumentManager::logAllDocumentStates(const QString& context)
{
    qCDebug(docManagerLog) << "Logging all document states -" << context;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        QMdiArea *mdiArea = getActiveMdiArea(i);
        if (!mdiArea) continue;

        for (QMdiSubWindow *window : mdiArea->subWindowList()) {
            if (KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget())) {
                qCDebug(docManagerLog) << "Document:" << window->windowTitle()
                                       << "Modified:" << textEdit->document()->isModified()
                                       << "Content:" << textEdit->toPlainText().left(20) + QStringLiteral("...");
            }
        }
    }
}

QStringList DocumentManager::getRecentFiles() const
{
    return m_recentFiles;
}

QMdiArea* DocumentManager::getActiveMdiArea(int index) const
{
    QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->widget(index));
    if (splitter) {
        return qobject_cast<QMdiArea*>(splitter->widget(0));
    }
    return nullptr;
}

QMdiArea* DocumentManager::getActiveMdiArea() const
{
    return getActiveMdiArea(m_tabWidget->currentIndex());
}

void DocumentManager::setupTextEdit(KTextEdit* textEdit, const QString& filePath)
{
    textEdit->setCheckSpellingEnabled(m_settingsManagement->isSpellCheckEnabled());

    textEdit->document()->setModified(false);

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            textEdit, [this, textEdit](bool changed) {
                QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
                if (window) {
                    QString title = window->windowTitle();
                    if (changed && !title.endsWith(QLatin1String(" *"))) {
                        window->setWindowTitle(title + QLatin1String(" *"));
                        qCDebug(docManagerLog) << "Document marked as modified:" << title;
                    } else if (!changed && title.endsWith(QLatin1String(" *"))) {
                        window->setWindowTitle(title.left(title.length() - 2));
                        qCDebug(docManagerLog) << "Document marked as unmodified:" << title;
                    }
                }
                logDocumentState(textEdit, QStringLiteral("modificationChanged"));
            });

    connect(textEdit, &KTextEdit::textChanged, this, [this, textEdit]() {
        if (textEdit->document()->isModified()) {
            logDocumentState(textEdit, QStringLiteral("textChanged"));
        }
    });

    if (!filePath.isEmpty()) {
        QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
        if (window) {
            window->setWindowTitle(QFileInfo(filePath).fileName());
        }
    }

    logDocumentState(textEdit, QStringLiteral("setupTextEdit"));
}

void DocumentManager::setupSubWindow(QMdiSubWindow *subWindow)
{
    subWindow->setWindowFlags(Qt::SubWindow | Qt::CustomizeWindowHint | Qt::WindowTitleHint | 
                              Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    subWindow->setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));
}
