#include "AutoSaveManager.h"
#include "DocumentManager.h"
#include <QTabWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <KTextEdit>
#include <QSettings>
#include <QLoggingCategory>
#include <KLocalizedString>
#include <QSplitter>

Q_LOGGING_CATEGORY(autoSaveLog, "mudoedit.autosavemanager")

AutoSaveManager::AutoSaveManager(QTabWidget *tabWidget, DocumentManager *documentManager, QSettings *settings, QObject *parent)
    : QObject(parent),
      m_tabWidget(tabWidget),
      m_documentManager(documentManager),
      m_settings(settings),
      m_autoSaveTimer(new QTimer(this)),
      m_autoSaveInterval(DEFAULT_AUTOSAVE_INTERVAL)
{
    connect(m_autoSaveTimer, &QTimer::timeout, this, &AutoSaveManager::autoSave);
    loadAutoSaveSettings();
}

AutoSaveManager::~AutoSaveManager()
{
    saveAutoSaveSettings();
}

void AutoSaveManager::startAutoSave()
{
    m_autoSaveTimer->start(m_autoSaveInterval);
    qCDebug(autoSaveLog) << "Autosave started with interval:" << m_autoSaveInterval << "ms";
}

void AutoSaveManager::stopAutoSave()
{
    m_autoSaveTimer->stop();
    qCDebug(autoSaveLog) << "Autosave stopped";
}

void AutoSaveManager::loadAutoSaveSettings()
{
    m_autoSaveInterval = m_settings->value(QStringLiteral("autoSaveInterval"), DEFAULT_AUTOSAVE_INTERVAL).toInt();
    bool autoSaveEnabled = m_settings->value(QStringLiteral("autoSaveEnabled"), true).toBool();
    if (autoSaveEnabled)
    {
        startAutoSave();
    }
}

void AutoSaveManager::saveAutoSaveSettings()
{
    m_settings->setValue(QStringLiteral("autoSaveInterval"), m_autoSaveInterval);
    m_settings->setValue(QStringLiteral("autoSaveEnabled"), m_autoSaveTimer->isActive());
}

void AutoSaveManager::autoSave()
{
    qCDebug(autoSaveLog) << "Performing autosave";
    for (int i = 0; i < m_tabWidget->count(); ++i)
    {
        QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->widget(i));
        if (!splitter) continue;

        QMdiArea *mdiArea = qobject_cast<QMdiArea*>(splitter->widget(0));
        if (!mdiArea) continue;

        for (QMdiSubWindow *window : mdiArea->subWindowList())
        {
            KTextEdit *textEdit = qobject_cast<KTextEdit*>(window->widget());
            if (textEdit && textEdit->document()->isModified())
            {
                QString filePath = window->property("fullFilePath").toString();
                if (!filePath.isEmpty() && filePath != i18n("Untitled"))
                {
                    if (m_documentManager->saveFile())
                    {
                        qCDebug(autoSaveLog) << "Autosaved file:" << filePath;
                    }
                    else
                    {
                        qCWarning(autoSaveLog) << "Failed to autosave file:" << filePath;
                    }
                }
            }
        }
    }
}