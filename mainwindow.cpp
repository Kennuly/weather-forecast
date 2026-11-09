#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMouseEvent>


MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(1000, 500, 800, 450);
    this->setFixedSize(800, 450);

    //设置窗口属性为没有边框的窗口
    setWindowFlag(Qt::FramelessWindowHint);


    widget = new QWidget(this);
    //设置名字
    widget->setObjectName("widget");
    widget->setGeometry(0, 0, 800, 450);

    //setStyleSheet(...)把整段外观规则应用到当前窗口及其子控件。
    setStyleSheet(R"(
        QWidget#widget {
            border-image: url(:/weather/backgrounds/sky.svg);
            color: rgb(255, 255, 255);
        }

        QLabel {
            font: 10pt "Microsoft YaHei";
            border-radius: 4px;
            background-color: rgba(60, 60, 60, 130);
            color: rgb(255, 255, 255);
        }

        Line {
            background-color: rgba(0, 85, 0, 255);
        }
    )");


    exitMenu = new QMenu(this);

    exitAct = new QAction(this);
    exitAct->setText(tr("退出"));
    exitAct->setIcon(QIcon(":/weather/actions/exit.svg"));

    exitMenu->addAction(exitAct);
    connect(exitAct, &QAction::triggered, this, &MainWindow::slot_exitapp);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//重写 Qt 的右键菜单事件
void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    /**
     * QCursor::pos()：获取鼠标当前的屏幕全局坐标。
     * exec(...)：在指定位置弹出菜单，并阻塞等待用户选择或关闭菜单。
     * event->accept()：告诉 Qt，此右键事件已经处理，不再继续传递。
    **/
    exitMenu->exec(QCursor::pos());
    event->accept();
}

//鼠标移动事件
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    move(event->globalPos() - mPos);
}
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    //globalPos()鼠标当前位置           pos()主窗口当前位置
    mPos = event->globalPos() - this->pos();
}

void MainWindow::slot_exitapp()
{
    //qApp->exit(0);
    this->close();
}