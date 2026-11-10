#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QFrame>
#include <QLabel>
#include <QGridLayout>

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
    QLineEdit* lineEdit;    //文本编辑框
    QTextEdit* textEdit;    //日期标签

    //水平线
    QFrame* line1;
    QFrame* line2;
    QFrame* line3;

    QLabel* lab;        //天气预报标签

    QLabel* lab1;       //湿度title
    QLabel* lab2;       //湿度
    QLabel* lab3;       //PM2.5 title
    QLabel* lab4;       //PM2.5
    QLabel* lab5;       //风向title
    QLabel* lab6;       //风向
    QLabel* lab7;       //风力title
    QLabel* lab8;       //风力
    QGridLayout* layout;
    QWidget* metricsPanel;


private:
    ////重写 Qt 的右键菜单事件
    Ui::MainWindow *ui;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
private slots:
    void slot_exitapp();
};
#endif // MAINWINDOW_H
