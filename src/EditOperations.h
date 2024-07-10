#ifndef EDITOPERATIONS_H
#define EDITOPERATIONS_H

#include <QObject>

class QTabWidget;
class QMdiArea;

// This class handles editing operations like undo, redo, cut, copy, and paste
class EditOperations : public QObject
{
    Q_OBJECT

public:
    // Constructor takes a pointer to the tab widget and an optional parent object
    explicit EditOperations(QTabWidget *tabWidget, QObject *parent = nullptr);

public Q_SLOTS:
    // Functions to perform various editing operations
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

private:
    // Helper function to get the active MDI area
    QMdiArea* getActiveMdiArea() const;

    QTabWidget *m_tabWidget;
};

#endif // EDITOPERATIONS_H