// This file implements the FileOperations class, which handles all file-related tasks in the editor

#include "FileOperations.h"
#include <KTextEdit>
#include <QMdiSubWindow>
#include <QFileDialog>
#include <QFile>
#include <KMessageBox>
#include <KLocalizedString>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include "SettingsManagement.h"
#include <QApplication>
#include <QStyle>
#include <QMimeDatabase>
#include <QStringList>

// Set up a logging category for file operations
Q_LOGGING_CATEGORY(fileOpsLog, "mudoedit.fileoperations")

// Constructor for FileOperations
FileOperations::FileOperations(QMdiArea *mdiArea, QSettings *settings, SettingsManagement *settingsManagement, QObject *parent)
    : QObject(parent), m_mdiArea(mdiArea), m_settings(settings), m_settingsManagement(settingsManagement),
      m_autoSaveTimer(new QTimer(this)), m_autoSaveInterval(300000) // Default to 5 minutes (300,000 ms)
{
    loadAutoSaveSettings();
    connect(m_autoSaveTimer, &QTimer::timeout, this, &FileOperations::autoSave);

    // Load recent files
    m_recentFiles = m_settings->value("recentFiles").toStringList();
}


// Create a new document
void FileOperations::newDocument()
{
    KTextEdit *textEdit = new KTextEdit;
    QMdiSubWindow *subWindow = m_mdiArea->addSubWindow(textEdit);
    setupTextEdit(textEdit);
    setupSubWindow(subWindow);
    subWindow->setWindowTitle(i18n("Untitled"));
    subWindow->resize(600, 400);
    subWindow->show();
    logDocumentState(textEdit, QStringLiteral("newDocument"));
    logAllDocumentStates(QStringLiteral("After newDocument"));
}

// Open a file using a file dialog
void FileOperations::openFile()
{
    // Create a QFileDialog with the parent set to m_mdiArea
    QFileDialog dialog(m_mdiArea);
    
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

// Open a specific file
QMdiSubWindow* FileOperations::openFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString absoluteFilePath = fileInfo.absoluteFilePath();

    KTextEdit *textEdit = new KTextEdit;
    QMdiSubWindow *subWindow = m_mdiArea->addSubWindow(textEdit);
    setupSubWindow(subWindow);

    QFile file(absoluteFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        textEdit->setPlainText(QString::fromUtf8(file.readAll()));
        file.close();
        setupTextEdit(textEdit, absoluteFilePath);
        subWindow->resize(600, 400);  // Default size if no saved geometry
        subWindow->show();
        textEdit->document()->setModified(false);
        subWindow->setProperty("fullFilePath", absoluteFilePath);
        logDocumentState(textEdit, QStringLiteral("openFile"));
        logAllDocumentStates(QStringLiteral("After openFile"));
         // Add the file to recent files
        addToRecentFiles(absoluteFilePath);       
        return subWindow;
    } else {
        KMessageBox::error(m_mdiArea, i18n("Could not open file %1", absoluteFilePath));
        m_mdiArea->removeSubWindow(subWindow);
        delete subWindow;
        return nullptr;
    }
}

// Set up the sub-window with proper flags and icons
void FileOperations::setupSubWindow(QMdiSubWindow *subWindow)
{
    // Set window flags for the sub-window
    subWindow->setWindowFlags(Qt::SubWindow | Qt::CustomizeWindowHint | Qt::WindowTitleHint | 
                              Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    // Get the application's style
    QStyle *style = QApplication::style();
    
    // Set the window icon using the file icon from the current style
    subWindow->setWindowIcon(style->standardIcon(QStyle::SP_FileIcon));
}

// Save the current file
bool FileOperations::saveFile()
{
    // Get the active text edit widget
    KTextEdit *textEdit = qobject_cast<KTextEdit*>(m_mdiArea->activeSubWindow()->widget());
    if (!textEdit) return false;

    // Get the current file path
    QString filePath = m_mdiArea->activeSubWindow()->property("fullFilePath").toString();
    
    // If it's a new file (Untitled), ask for a save location
    if (filePath.isEmpty() || filePath == i18n("Untitled")) {
        filePath = QFileDialog::getSaveFileName(m_mdiArea, i18n("Save File"), 
                                                QDir::homePath(), // Start in the home directory
                                                i18n("Text Files (*.txt);;All Files (*)"));
        if (filePath.isEmpty()) return false; // User cancelled the save dialog

        // Add the new file to recent files
        addToRecentFiles(filePath);
    }

    // Ensure we have the absolute file path
    QFileInfo fileInfo(filePath);
    QString absoluteFilePath = fileInfo.absoluteFilePath();

    // Open the file for writing
    QFile file(absoluteFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Write the content to the file
        QTextStream out(&file);
        out << textEdit->toPlainText();
        file.close();

        // Update the document and window properties
        textEdit->document()->setModified(false);
        QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
        if (window) {
            window->setWindowTitle(fileInfo.fileName());
            window->setProperty("fullFilePath", absoluteFilePath);
        }

        // Log the save operation
        logDocumentState(textEdit, QStringLiteral("saveFile"));
        logAllDocumentStates(QStringLiteral("After saveFile"));
        return true;
    } else {
        // Show an error message if the file couldn't be saved
        KMessageBox::error(m_mdiArea, i18n("Could not save file %1", absoluteFilePath));
        return false;
    }
}

// Save all open files
void FileOperations::saveAllFiles()
{
    for (QMdiSubWindow *window : m_mdiArea->subWindowList()) {
        KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget());
        if (textEdit) {
            m_mdiArea->setActiveSubWindow(window);
            saveFile();
        }
    }
    logAllDocumentStates(QStringLiteral("After saveAllFiles"));
}

// Save the list of open documents for later reopening
void FileOperations::saveOpenDocuments()
{
    QStringList openFiles;
    QVariantList windowGeometries;
    for (QMdiSubWindow *window : m_mdiArea->subWindowList()) {
        KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget());
        if (textEdit) {
            QString filePath = window->property("fullFilePath").toString();
            if (!filePath.isEmpty()) {
                openFiles << filePath;
                windowGeometries << window->saveGeometry();
            }
        }
    }
    m_settings->setValue(QStringLiteral("openFiles"), openFiles);
    m_settings->setValue(QStringLiteral("windowGeometries"), windowGeometries);
    logAllDocumentStates(QStringLiteral("After saveOpenDocuments"));
}

// Reopen documents from the last session
void FileOperations::reopenDocuments()
{
    QStringList openFiles = m_settings->value(QStringLiteral("openFiles")).toStringList();
    QVariantList windowGeometries = m_settings->value(QStringLiteral("windowGeometries")).toList();
    qCDebug(fileOpsLog) << "Attempting to reopen" << openFiles.size() << "files";

    for (int i = 0; i < openFiles.size(); ++i) {
        const QString &filePath = openFiles[i];
        QFileInfo fileInfo(filePath);
        if (fileInfo.exists() && fileInfo.isReadable()) {
            QMdiSubWindow *window = openFile(filePath);
            if (window && i < windowGeometries.size()) {
                window->restoreGeometry(windowGeometries[i].toByteArray());
            }
        } else {
            qCDebug(fileOpsLog) << "Failed to reopen file:" << filePath << "(File doesn't exist or is not readable)";
        }
    }
    logAllDocumentStates(QStringLiteral("After reopenDocuments"));
}

// Get a list of windows with modified documents
QList<QMdiSubWindow*> FileOperations::getModifiedWindows()
{
    QList<QMdiSubWindow*> modifiedWindows;
    for (QMdiSubWindow *window : m_mdiArea->subWindowList()) {
        if (KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget())) {
            if (textEdit->document()->isModified()) {
                modifiedWindows.append(window);
                qCDebug(fileOpsLog) << "Modified document found:" << window->windowTitle()
                                    << "Content:" << textEdit->toPlainText().left(20) + QStringLiteral("...");
            }
        }
    }
    qCDebug(fileOpsLog) << "Total modified documents:" << modifiedWindows.size();
    return modifiedWindows;
}

// Save all modified documents
bool FileOperations::saveAllModifiedDocuments(const QList<QMdiSubWindow*>& windows)
{
    for (QMdiSubWindow *window : windows) {
        m_mdiArea->setActiveSubWindow(window);
        if (!saveFile()) {
            return false;  // If saving fails, return false
        }
    }
    logAllDocumentStates(QStringLiteral("After saveAllModifiedDocuments"));
    return true;
}

// Set up the text edit widget
void FileOperations::setupTextEdit(KTextEdit* textEdit, const QString& filePath)
{
    textEdit->setCheckSpellingEnabled(true);

    textEdit->document()->setModified(false);

    // Apply the current spell check setting
    textEdit->setCheckSpellingEnabled(m_settingsManagement->isSpellCheckEnabled());

    // Connect signals for document modification
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            textEdit, [this, textEdit](bool changed) {
                QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
                if (window) {
                    QString title = window->windowTitle();
                    if (changed && !title.endsWith(QLatin1String(" *"))) {
                        window->setWindowTitle(title + QLatin1String(" *"));
                        qCDebug(fileOpsLog) << "Document marked as modified:" << title;
                    } else if (!changed && title.endsWith(QLatin1String(" *"))) {
                        window->setWindowTitle(title.left(title.length() - 2));
                        qCDebug(fileOpsLog) << "Document marked as unmodified:" << title;
                    }
                }
                logDocumentState(textEdit, QStringLiteral("modificationChanged"));
            });

    connect(textEdit, &KTextEdit::textChanged, this, [this, textEdit]() {
        // Only log if the document is actually modified
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

// Log the state of a document
void FileOperations::logDocumentState(KTextEdit* textEdit, const QString& action)
{
    QMdiSubWindow* window = qobject_cast<QMdiSubWindow*>(textEdit->parent());
    QString title = window ? window->windowTitle() : QStringLiteral("Unknown");
    qCDebug(fileOpsLog) << action << "- Document:" << title
                        << "Modified:" << textEdit->document()->isModified()
                        << "Content:" << textEdit->toPlainText().left(20) + QStringLiteral("...");
}

// Log the state of all documents
void FileOperations::logAllDocumentStates(const QString& context)
{
    qCDebug(fileOpsLog) << "Logging all document states -" << context;
    for (QMdiSubWindow *window : m_mdiArea->subWindowList()) {
        if (KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget())) {
            qCDebug(fileOpsLog) << "Document:" << window->windowTitle()
                                << "Modified:" << textEdit->document()->isModified()
                                << "Content:" << textEdit->toPlainText().left(20) + QStringLiteral("...");
        }
    }
}

void FileOperations::startAutoSave()
{
    m_autoSaveTimer->start(m_autoSaveInterval);
    qCDebug(fileOpsLog) << "Autosave started with interval:" << m_autoSaveInterval << "ms";
}

void FileOperations::stopAutoSave()
{
    m_autoSaveTimer->stop();
    qCDebug(fileOpsLog) << "Autosave stopped";
}

void FileOperations::autoSave()
{
    qCDebug(fileOpsLog) << "Performing autosave";
    for (QMdiSubWindow *window : m_mdiArea->subWindowList())
    {
        KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget());
        if (textEdit && textEdit->document()->isModified())
        {
            QString filePath = window->property("fullFilePath").toString();
            if (!filePath.isEmpty() && filePath != i18n("Untitled"))
            {
                QFile file(filePath);
                if (file.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream out(&file);
                    out << textEdit->toPlainText();
                    file.close();
                    textEdit->document()->setModified(false);
                    qCDebug(fileOpsLog) << "Autosaved file:" << filePath;
                }
                else
                {
                    qCWarning(fileOpsLog) << "Failed to autosave file:" << filePath;
                }
            }
        }
    }
}

void FileOperations::loadAutoSaveSettings()
{
    m_autoSaveInterval = m_settings->value(QStringLiteral("autoSaveInterval"), 300000).toInt();
    bool autoSaveEnabled = m_settings->value(QStringLiteral("autoSaveEnabled"), true).toBool();
    if (autoSaveEnabled)
    {
        startAutoSave();
    }
}

void FileOperations::saveAutoSaveSettings()
{
    m_settings->setValue(QStringLiteral("autoSaveInterval"), m_autoSaveInterval);
    m_settings->setValue(QStringLiteral("autoSaveEnabled"), m_autoSaveTimer->isActive());
}

// Modify the destructor to save autosave settings
FileOperations::~FileOperations()
{
    saveAutoSaveSettings();
}

void FileOperations::addToRecentFiles(const QString &filePath)
{
    // Get the absolute file path
    QFileInfo fileInfo(filePath);
    QString absoluteFilePath = fileInfo.absoluteFilePath();

    // Remove the file path if it already exists in the list
    m_recentFiles.removeAll(absoluteFilePath);

    // Add the file path to the beginning of the list
    m_recentFiles.prepend(absoluteFilePath);

    // Ensure the list doesn't exceed the maximum size
    while (m_recentFiles.size() > MAX_RECENT_FILES) {
        m_recentFiles.removeLast();
    }

    // Save the updated list to settings
    updateRecentFiles();
}

// Update the updateRecentFiles method
void FileOperations::updateRecentFiles()
{
    m_settings->setValue(QStringLiteral("recentFiles"), m_recentFiles);
    m_settings->sync();
    Q_EMIT recentFilesChanged();  // Emit the signal when recent files are updated
}

QStringList FileOperations::getRecentFiles() const
{
    return m_recentFiles;
}