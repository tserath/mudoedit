#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QObject>
#include <QMdiArea>
#include <QSettings>
#include <QTimer>
#include <QLoggingCategory>

class KTextEdit;
class QMdiSubWindow;
class SettingsManagement;

Q_DECLARE_LOGGING_CATEGORY(fileOpsLog)

class FileOperations : public QObject
{
    Q_OBJECT

public:
    explicit FileOperations(QMdiArea *mdiArea, QSettings *settings, SettingsManagement *settingsManagement, QObject *parent = nullptr);
    ~FileOperations();

    void newDocument();
    QMdiSubWindow* openFile(const QString &filePath);
    void openFile();
    bool saveFile();
    void saveAllFiles();
    void saveOpenDocuments();
    void reopenDocuments();
    QList<QMdiSubWindow*> getModifiedWindows();
    bool saveAllModifiedDocuments(const QList<QMdiSubWindow*>& windows);
    void logAllDocumentStates(const QString& context);

    void startAutoSave();
    void stopAutoSave();

    // Make getRecentFiles public
    QStringList getRecentFiles() const;
    void addToRecentFiles(const QString &filePath);

Q_SIGNALS:
    // Add the missing signal
    void recentFilesChanged();

private:
    void setupTextEdit(KTextEdit* textEdit, const QString& filePath = QString());
    void setupSubWindow(QMdiSubWindow* subWindow);
    void logDocumentState(KTextEdit* textEdit, const QString& action);
    void loadAutoSaveSettings();
    void saveAutoSaveSettings();
    void updateRecentFiles();

    QMdiArea *m_mdiArea;
    QSettings *m_settings;
    SettingsManagement *m_settingsManagement;
    QTimer *m_autoSaveTimer;
    int m_autoSaveInterval;

    QStringList m_recentFiles;
    static const int MAX_RECENT_FILES = 10;

private Q_SLOTS:
    void autoSave();
};

#endif // FILEOPERATIONS_H