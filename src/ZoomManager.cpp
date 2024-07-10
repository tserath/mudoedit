#include "ZoomManager.h"
#include <QTabWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <KTextEdit>
#include <QSplitter>

ZoomManager::ZoomManager(QTabWidget* tabWidget, QObject* parent)
    : QObject(parent),
      m_tabWidget(tabWidget),
      m_zoomFactor(1.0)
{
}

void ZoomManager::zoomIn()
{
    m_zoomFactor *= 1.1;
    applyZoom();
}

void ZoomManager::zoomOut()
{
    m_zoomFactor *= 0.9;
    applyZoom();
}

void ZoomManager::applyZoom()
{
    QMdiArea* mdiArea = getActiveMdiArea();
    if (!mdiArea) return;

    for (QMdiSubWindow* window : mdiArea->subWindowList()) {
        if (KTextEdit* textEdit = qobject_cast<KTextEdit*>(window->widget())) {
            QFont font = textEdit->font();
            font.setPointSizeF(font.pointSizeF() * m_zoomFactor);
            textEdit->setFont(font);
        }
    }
}

QMdiArea* ZoomManager::getActiveMdiArea() const
{
    QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->currentWidget());
    if (splitter) {
        return qobject_cast<QMdiArea*>(splitter->widget(0));
    }
    return nullptr;
}