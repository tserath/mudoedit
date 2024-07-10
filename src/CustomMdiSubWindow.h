#ifndef CUSTOMMDISUBWINDOW_H
#define CUSTOMMDISUBWINDOW_H

#include <QMdiSubWindow>

class MainWindow;
class QContextMenuEvent;

class CustomMdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    explicit CustomMdiSubWindow(MainWindow* mainWindow, QWidget* parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private:
    void startSystemMove();
    void startSystemResize();

    MainWindow* m_mainWindow;
};

#endif // CUSTOMMDISUBWINDOW_H