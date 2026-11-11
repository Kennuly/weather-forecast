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
    //setObjectName() 是给 Qt 对象设置一个程序内部的名字，常用于样式表匹配、查找控件和调试。
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
            background-color: argb(60, 60, 60, 130);
            color: rgb(255, 255, 255);
        }

        QFrame {
            background-color: rgba(0, 85, 0, 255);
        }
    )");


    exitMenu = new QMenu(this);

    exitAct = new QAction(this);
    exitAct->setText(tr("退出"));
    exitAct->setIcon(QIcon(":/weather/actions/exit.svg"));

    exitMenu->addAction(exitAct);
    connect(exitAct, &QAction::triggered, this, &MainWindow::slot_exitapp);


    lineEdit = new QLineEdit(this);
    lineEdit->setObjectName("cityLineEdit");
    lineEdit->setGeometry(575, 10, 171, 31);

    textEdit = new QTextEdit("2026-11-10",this);
    textEdit->setObjectName("dateLb");
    textEdit->setGeometry(190, 3, 421, 41);
    textEdit->setAlignment(Qt::AlignHCenter);   //水平居中
    textEdit->setStyleSheet(R"(
        QTextEdit {
            background-color:rgba(255, 255, 255, 0);
            font: 20pt "Adobe 黑体 Std R";
            color: rgb(255, 255, 255);
        }
    )");

    line1 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("hLine_1");
    line1->setGeometry(10, 45, 781, 1);

    line2 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("hLine_10");
    line2->setGeometry(10, 183, 351, 1);

    line3 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("hLine_11");
    line3->setGeometry(10, 335, 351, 1);


    lab = new QLabel("天气预报", this);
    lab->setGeometry(80, 10, 171, 31);
    lab->setStyleSheet(R"(
        font: 20pt "华文琥珀";
        background-color:rgba(255,255,255,0);
        color: rgb(85,255,0);
    )");

    lab1 = new QLabel("湿度", this);
    lab1->setAlignment(Qt::AlignHCenter);
    lab2 = new QLabel("95%", this);
    lab2->setAlignment(Qt::AlignHCenter);
    lab3 = new QLabel("PM2.5", this);
    lab3->setAlignment(Qt::AlignHCenter);
    lab4 = new QLabel("0", this);
    lab4->setAlignment(Qt::AlignHCenter);
    lab5 = new QLabel("风向", this);
    lab5->setAlignment(Qt::AlignHCenter);
    lab6 = new QLabel("西北风", this);
    lab6->setAlignment(Qt::AlignHCenter);
    lab7 = new QLabel("风力", this);
    lab7->setAlignment(Qt::AlignHCenter);
    lab8 = new QLabel("<3", this);
    lab8->setAlignment(Qt::AlignHCenter);

    metricsPanel = new QWidget(this);
    metricsPanel->setGeometry(10, 185, 351, 152);
    layout = new QGridLayout(metricsPanel);
    layout->setContentsMargins(0, 3, 0, 5);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(6);

    layout->addWidget(lab1, 0, 0);
    layout->addWidget(lab2, 0, 1);
    layout->addWidget(lab3, 1, 0);
    layout->addWidget(lab4, 1, 1);
    layout->addWidget(lab5, 2, 0);
    layout->addWidget(lab6, 2, 1);
    layout->addWidget(lab7, 3, 0);
    layout->addWidget(lab8, 3, 1);





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
    mPos = event->globalPos() - this->pos();    //鼠标按下时，相对于窗口左上角的偏移量
}

void MainWindow::slot_exitapp()
{
    //qApp->exit(0);
    this->close();
}