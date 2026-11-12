#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "weatherdata.h"

#include <QAction>
#include <QColor>
#include <QContextMenuEvent>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPoint>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>

class WeatherService;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    // 使用自定义信号槽的 QObject 子类必须包含 Q_OBJECT。
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    // private slots 可以通过 connect() 连接按钮点击和 WeatherService 信号。
    void slot_searchCity();
    void slot_refreshWeather();
    void slot_exitapp();
    void handleWeatherData(const WeatherData& data);
    void handleWeatherError(const QString& message);
    void handleLoadingChanged(bool loading);

private:
    // setupXXX() 方法只负责创建和摆放界面控件。
    void setupWindow();
    void setupSearchControls();
    void setupCurrentWeatherInfo();
    void setupColdIndexPanel();
    void setupForecastTable();
    void populateForecastTable(const QVector<ForecastDayData>& forecast);

    QString weatherIconPath(const QString& weatherCode) const;
    QColor airQualityColor(const QString& airQuality) const;
    QString airQualityShortText(const QString& airQuality) const;
    void updateColdIndex(const WeatherData& data);
    void showQueryError(const QString& message);

    // uic 根据 mainwindow.ui 自动生成 Ui::MainWindow。
    Ui::MainWindow* ui;

    // 网络请求和 JSON 解析由服务层负责，主窗口只消费 WeatherData。
    WeatherService* weatherService;

    // 基础窗口和拖动相关成员。
    QWidget* widget;
    QMenu* exitMenu;
    QAction* exitAct;
    QPoint mPos;

    // 顶部城市搜索框和日期显示。
    QLineEdit* lineEdit;
    QTextEdit* textEdit;

    // 左侧分隔线。
    QFrame* line1;
    QFrame* line2;
    QFrame* line3;

    // 左侧湿度、PM2.5、风向和风力指标。
    QLabel* lab;
    QLabel* lab1;
    QLabel* lab2;
    QLabel* lab3;
    QLabel* lab4;
    QLabel* lab5;
    QLabel* lab6;
    QLabel* lab7;
    QLabel* lab8;
    QGridLayout* layout;
    QWidget* metricsPanel;

    // 城市和顶部操作按钮。
    QLabel* cityTit;
    QLabel* city;
    QToolButton* searchButton;
    QToolButton* refreshButton;

    // 当前天气摘要。
    QLabel* airQualityIcon;
    QLabel* airQualityLabel;
    QLabel* currentTemperature;
    QLabel* currentWeatherIcon;
    QLabel* currentCondition;
    QLabel* weatherTip;

    // 底部感冒指数与右侧六日预报表格。
    QLabel* coldIndexTitle;
    QLabel* coldIndexPanel;
    QTableWidget* forecastTable;
};

#endif // MAINWINDOW_H
