#ifndef ZOOMMANAGER_H
#define ZOOMMANAGER_H

#include <QObject>

class QTabWidget;
class KTextEdit;
class QMdiArea;  // Add this forward declaration

// This class manages zoom functionality for the text editor
class ZoomManager : public QObject
{
    Q_OBJECT

public:
    // Constructor takes a pointer to the tab widget
    explicit ZoomManager(QTabWidget* tabWidget, QObject* parent = nullptr);

public Q_SLOTS:
    // Slot to handle zooming in
    void zoomIn();
    // Slot to handle zooming out
    void zoomOut();

private:
    // Helper function to apply zoom to all open documents
    void applyZoom();

    // Helper function to get the active MDI area
    QMdiArea* getActiveMdiArea() const;

    QTabWidget* m_tabWidget;
    qreal m_zoomFactor;
};

#endif // ZOOMMANAGER_H