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
#include <QTableWidget>
#include <QVector>
#include <QColor>
#include <QToolButton>

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

    QLabel* cityTit;
    QLabel* city;

    // 顶部搜索和刷新图标按钮
    QToolButton* searchButton;
    QToolButton* refreshButton;

    // 左侧当前天气摘要
    QLabel* airQualityIcon;
    QLabel* airQualityLabel;
    QLabel* currentTemperature;
    QLabel* currentWeatherIcon;
    QLabel* currentCondition;
    QLabel* weatherTip;

    // 底部感冒指数区域
    QLabel* coldIndexTitle;
    QLabel* coldIndexPanel;

    // 初始化界面中新增的区域
    void setupSearchControls();
    void setupCurrentWeatherInfo();
    void setupColdIndexPanel();

    // 保存右侧预报表格中每一天的数据
    struct ForecastItem {
        QString weekday;          // 星期，如“今天”“明天”
        QString date;             // 日期，如“02/02”
        QString airQuality;       // 空气质量
        QString weather;          // 天气文字，如“阴”
        QString weatherCode;      // 天气代码，用于匹配资源文件中的图标
        QString highTemperature;  // 最高温度
        QString lowTemperature;   // 最低温度
    };

    QTableWidget* forecastTable;  // 右侧天气预报表格

    // 初始化表格外观
    void setupForecastTable();

    // 把预报数据填入表格
    void populateForecastTable(const QVector<ForecastItem>& forecast);

    // 根据天气代码查找资源文件中的图标
    QString weatherIconPath(const QString& weatherCode) const;

    // 根据空气质量返回对应的显示颜色
    QColor airQualityColor(const QString& airQuality) const;


private:
    ////重写 Qt 的右键菜单事件
    Ui::MainWindow *ui;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
private slots:
    void slot_exitapp();
    void slot_searchCity();
};
#endif // MAINWINDOW_H
