#ifndef AUTOSAVEMANAGER_H
#define AUTOSAVEMANAGER_H

#include <QObject>
#include <QTimer>

class QTabWidget;
class DocumentManager;
class QSettings;

// This class manages the auto-save functionality
class AutoSaveManager : public QObject
{
    Q_OBJECT

public:
    explicit AutoSaveManager(QTabWidget *tabWidget, DocumentManager *documentManager, QSettings *settings, QObject *parent = nullptr);
    ~AutoSaveManager();

    // Start the auto-save timer
    void startAutoSave();

    // Stop the auto-save timer
    void stopAutoSave();

    // Load auto-save settings from QSettings
    void loadAutoSaveSettings();

    // Save auto-save settings to QSettings
    void saveAutoSaveSettings();

public Q_SLOTS:
    // Perform auto-save operation
    void autoSave();

private:
    QTabWidget *m_tabWidget;
    DocumentManager *m_documentManager;
    QSettings *m_settings;
    QTimer *m_autoSaveTimer;
    int m_autoSaveInterval;

    // Default auto-save interval in milliseconds (5 minutes)
    static const int DEFAULT_AUTOSAVE_INTERVAL = 300000;
};

#endif // AUTOSAVEMANAGER_H