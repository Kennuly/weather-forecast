#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
private:
    QWidget* widget;        //窗口
    QMenu* exitMenu;        //右键退出的菜单
    QAction* exitAct;       //退出行为
    QPoint mPos;            //窗口移动需要用的点
private:
    ////重写 Qt 的右键菜单事件
    Ui::MainWindow *ui;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
private slots:
    void slot_exitapp();
};
#endif // MAINWINDOW_H
