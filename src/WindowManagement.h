#ifndef WINDOWMANAGEMENT_H
#define WINDOWMANAGEMENT_H

#include <QObject>
#include <QTabWidget>

class QMdiArea;

// This class manages window operations for the application
class WindowManagement : public QObject
{
    Q_OBJECT

public:
    // Constructor takes a pointer to the tab widget
    explicit WindowManagement(QTabWidget *tabWidget, QObject *parent = nullptr);

public Q_SLOTS:
    // Slot to arrange windows in a tile pattern
    void tileWindows();
    // Slot to arrange windows in a cascade pattern
    void cascadeWindows();
    // Slot to minimize all windows
    void minimizeAllWindows();
    // Slot to close all windows
    void closeAllWindows();
    // Slot to tile windows side by side
    void tileWindowsSideBySide();

private:
    // Helper method to get the active MDI area
    QMdiArea* getActiveMdiArea() const;
    // Pointer to the tab widget
    QTabWidget *m_tabWidget;
};

#endif // WINDOWMANAGEMENT_H