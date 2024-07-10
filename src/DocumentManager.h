#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include <QObject>
#include <QList>
#include <QString>

class QTabWidget;
class QMdiArea;
class QMdiSubWindow;
class KTextEdit;
class FileIO;
class SettingsManagement;
class MainWindow;

class DocumentManager : public QObject
{
    Q_OBJECT

public:
    explicit DocumentManager(MainWindow* mainWindow, QTabWidget *tabWidget, FileIO *fileIO, SettingsManagement *settingsManagement, QObject *parent = nullptr);    // Open a new document
    void newDocument();

    // Open an existing file
    QMdiSubWindow* openFile(const QString &filePath);

    // Save the current document
    bool saveFile();

    // Save all open documents
    void saveAllFiles();

    // Get a list of windows with modified documents
    QList<QMdiSubWindow*> getModifiedWindows();

    // Save all modified documents
    bool saveAllModifiedDocuments(const QList<QMdiSubWindow*>& windows);

    // Save the list of open documents for later reopening
    void saveOpenDocuments();

    // Reopen documents from the last session
    void reopenDocuments();

    // Log the state of all documents
    void logAllDocumentStates(const QString& context);

    // Get the list of recent files
    QStringList getRecentFiles() const;

public Q_SLOTS:
    // Slot to handle file open action
    void openFile();

Q_SIGNALS:
    // Signal emitted when a file is successfully opened
    void fileOpened(const QString &filePath);

private:
    void setupTextEdit(KTextEdit* textEdit, const QString& filePath = QString());
    void setupSubWindow(QMdiSubWindow* subWindow);
    void logDocumentState(KTextEdit* textEdit, const QString& action);
    QMdiArea* getActiveMdiArea() const;
    QMdiArea* getActiveMdiArea(int index) const;

    MainWindow* m_mainWindow;
    QTabWidget *m_tabWidget;
    FileIO *m_fileIO;
    SettingsManagement *m_settingsManagement;
    QStringList m_recentFiles;
};

#endif // DOCUMENTMANAGER_H