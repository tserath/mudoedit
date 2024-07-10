#include "EditOperations.h"
#include <QTabWidget>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <KTextEdit>
#include <QSplitter>

EditOperations::EditOperations(QTabWidget *tabWidget, QObject *parent)
    : QObject(parent), m_tabWidget(tabWidget)
{
}

QMdiArea* EditOperations::getActiveMdiArea() const
{
    QSplitter *splitter = qobject_cast<QSplitter*>(m_tabWidget->currentWidget());
    if (splitter) {
        return qobject_cast<QMdiArea*>(splitter->widget(0));
    }
    return nullptr;
}

void EditOperations::undo()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (mdiArea && mdiArea->activeSubWindow()) {
        if (KTextEdit* editor = qobject_cast<KTextEdit*>(mdiArea->activeSubWindow()->widget())) {
            editor->undo();
        }
    }
}

void EditOperations::redo()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (mdiArea && mdiArea->activeSubWindow()) {
        if (KTextEdit* editor = qobject_cast<KTextEdit*>(mdiArea->activeSubWindow()->widget())) {
            editor->redo();
        }
    }
}

void EditOperations::cut()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (mdiArea && mdiArea->activeSubWindow()) {
        if (KTextEdit* editor = qobject_cast<KTextEdit*>(mdiArea->activeSubWindow()->widget())) {
            editor->cut();
        }
    }
}

void EditOperations::copy()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (mdiArea && mdiArea->activeSubWindow()) {
        if (KTextEdit* editor = qobject_cast<KTextEdit*>(mdiArea->activeSubWindow()->widget())) {
            editor->copy();
        }
    }
}

void EditOperations::paste()
{
    QMdiArea *mdiArea = getActiveMdiArea();
    if (mdiArea && mdiArea->activeSubWindow()) {
        if (KTextEdit* editor = qobject_cast<KTextEdit*>(mdiArea->activeSubWindow()->widget())) {
            editor->paste();
        }
    }
}