#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "weatherservice.h"

#include <QBrush>
#include <QDate>
#include <QFileInfo>
#include <QHeaderView>
#include <QMouseEvent>
#include <QTableWidgetItem>
#include <QtMath>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
    , weatherService(new WeatherService(this))
{
    // setupUi() 创建 mainwindow.ui 中定义的控件。
    ui->setupUi(this);
    setupWindow();

    lineEdit = new QLineEdit(this);
    lineEdit->setObjectName("cityLineEdit");
    lineEdit->setGeometry(575, 10, 132, 31);
    lineEdit->setPlaceholderText("搜索城市");
    lineEdit->setStyleSheet(R"(
        QLineEdit {
            background-color: rgba(250, 250, 250, 220);
            border: 1px solid rgba(255, 255, 255, 180);
            border-radius: 4px;
            color: rgb(35, 45, 55);
            padding: 2px 8px;
            font: 10pt "Microsoft YaHei";
        }
    )");
    setupSearchControls();

    textEdit = new QTextEdit(QDate::currentDate().toString("yyyy-MM-dd"), this);
    textEdit->setObjectName("dateLb");
    textEdit->setGeometry(190, 3, 421, 41);
    textEdit->setAlignment(Qt::AlignHCenter);
    textEdit->setStyleSheet(R"(
        QTextEdit {
            background-color: rgba(255, 255, 255, 0);
            font: 20pt "Adobe 黑体 Std R";
            color: rgb(255, 255, 255);
        }
    )");

    line1 = new QFrame(this);
    line1->setFrameShape(QFrame::HLine);
    line1->setObjectName("hLine_1");
    line1->setGeometry(10, 45, 781, 1);

    line2 = new QFrame(this);
    line2->setFrameShape(QFrame::HLine);
    line2->setObjectName("hLine_10");
    line2->setGeometry(10, 183, 351, 1);

    line3 = new QFrame(this);
    line3->setFrameShape(QFrame::HLine);
    line3->setObjectName("hLine_11");
    line3->setGeometry(10, 335, 351, 1);

    lab = new QLabel("天气预报", this);
    lab->setGeometry(80, 10, 171, 31);
    lab->setStyleSheet(R"(
        font: 20pt "华文琥珀";
        background-color: rgba(255, 255, 255, 0);
        color: rgb(85, 255, 0);
    )");

    lab1 = new QLabel("湿度", this);
    lab1->setAlignment(Qt::AlignHCenter);
    lab2 = new QLabel("--", this);
    lab2->setAlignment(Qt::AlignHCenter);
    lab3 = new QLabel("PM2.5", this);
    lab3->setAlignment(Qt::AlignHCenter);
    lab4 = new QLabel("--", this);
    lab4->setAlignment(Qt::AlignHCenter);
    lab5 = new QLabel("风向", this);
    lab5->setAlignment(Qt::AlignHCenter);
    lab6 = new QLabel("--", this);
    lab6->setAlignment(Qt::AlignHCenter);
    lab7 = new QLabel("风力", this);
    lab7->setAlignment(Qt::AlignHCenter);
    lab8 = new QLabel("--", this);
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

    setupCurrentWeatherInfo();
    setupColdIndexPanel();
    setupForecastTable();

    // 信号槽：WeatherService 发出数据，MainWindow 接收后更新控件。
    // 这里使用函数指针写法，连接错误会在编译期被发现。
    connect(weatherService, &WeatherService::weatherReady,
            this, &MainWindow::handleWeatherData);
    connect(weatherService, &WeatherService::errorOccurred,
            this, &MainWindow::handleWeatherError);
    connect(weatherService, &WeatherService::loadingChanged,
            this, &MainWindow::handleLoadingChanged);

    // 窗口显示前先发起一次默认城市查询。请求是异步的，不会阻塞界面。
    weatherService->queryCity("长沙");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupWindow()
{
    // 固定窗口尺寸并隐藏系统边框，背景和拖动效果由自定义代码实现。
    setGeometry(1000, 500, 800, 450);
    setFixedSize(800, 450);
    setWindowFlag(Qt::FramelessWindowHint);

    // 把 this 作为父对象后，Qt 会在 MainWindow 析构时自动释放 widget。
    widget = new QWidget(this);
    widget->setObjectName("widget");
    widget->setGeometry(0, 0, 800, 450);

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
}

void MainWindow::setupSearchControls()
{
    const QString buttonStyle = R"(
        QToolButton {
            background-color: rgba(35, 82, 112, 225);
            border: 1px solid rgba(255, 255, 255, 180);
            border-radius: 4px;
        }
        QToolButton:hover {
            background-color: rgba(48, 112, 145, 245);
        }
        QToolButton:pressed {
            background-color: rgba(24, 65, 90, 255);
        }
        QToolButton:disabled {
            background-color: rgba(35, 82, 112, 120);
        }
    )";

    searchButton = new QToolButton(this);
    searchButton->setObjectName("searchButton");
    searchButton->setGeometry(711, 10, 34, 31);
    searchButton->setIcon(QIcon(":/weather/actions/search.svg"));
    searchButton->setIconSize(QSize(18, 18));
    searchButton->setCursor(Qt::PointingHandCursor);
    searchButton->setStyleSheet(buttonStyle);
    searchButton->setToolTip("搜索城市");

    refreshButton = new QToolButton(this);
    refreshButton->setObjectName("refreshButton");
    refreshButton->setGeometry(750, 10, 40, 31);
    refreshButton->setIcon(QIcon(":/weather/actions/refresh.svg"));
    refreshButton->setIconSize(QSize(20, 20));
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet(R"(
        QToolButton {
            background-color: rgba(73, 190, 72, 220);
            border: 1px solid rgba(220, 255, 220, 180);
            border-radius: 4px;
        }
        QToolButton:hover {
            background-color: rgba(88, 215, 83, 245);
        }
        QToolButton:pressed {
            background-color: rgba(43, 145, 55, 255);
        }
        QToolButton:disabled {
            background-color: rgba(73, 190, 72, 110);
        }
    )");
    refreshButton->setToolTip("刷新天气");

    connect(searchButton, &QToolButton::clicked,
            this, &MainWindow::slot_searchCity);
    connect(lineEdit, &QLineEdit::returnPressed,
            this, &MainWindow::slot_searchCity);
    connect(refreshButton, &QToolButton::clicked,
            this, &MainWindow::slot_refreshWeather);
}

void MainWindow::setupCurrentWeatherInfo()
{
    const QString transparentStyle = R"(
        background-color: rgba(0, 0, 0, 0);
        border: none;
    )";

    airQualityIcon = new QLabel(this);
    airQualityIcon->setGeometry(18, 40, 24, 24);
    airQualityIcon->setStyleSheet(transparentStyle);
    airQualityIcon->setPixmap(
        QIcon(":/weather/actions/leaf.svg").pixmap(QSize(24, 24)));

    airQualityLabel = new QLabel("--", this);
    airQualityLabel->setGeometry(42, 36, 44, 30);
    airQualityLabel->setAlignment(Qt::AlignCenter);
    airQualityLabel->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(85, 255, 0);
        font: 12pt "Microsoft YaHei";
    )");

    currentTemperature = new QLabel("--°", this);
    currentTemperature->setGeometry(72, 47, 84, 58);
    currentTemperature->setAlignment(Qt::AlignCenter);
    currentTemperature->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 29pt "Microsoft YaHei";
    )");

    cityTit = new QLabel(this);
    cityTit->setGeometry(153, 49, 30, 30);
    cityTit->setStyleSheet(transparentStyle);
    cityTit->setPixmap(
        QIcon(":/weather/actions/location.svg").pixmap(QSize(30, 30)));

    city = new QLabel("长沙", this);
    city->setGeometry(184, 44, 72, 36);
    city->setAlignment(Qt::AlignCenter);
    city->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 16pt "Microsoft YaHei";
    )");

    currentWeatherIcon = new QLabel(this);
    currentWeatherIcon->setGeometry(254, 43, 92, 92);
    currentWeatherIcon->setAlignment(Qt::AlignCenter);
    currentWeatherIcon->setStyleSheet(transparentStyle);
    const QString iconPath = weatherIconPath("clear-day");
    if (!iconPath.isEmpty()) {
        currentWeatherIcon->setPixmap(
            QIcon(iconPath).pixmap(QSize(88, 88)));
    }

    currentCondition = new QLabel("--", this);
    currentCondition->setGeometry(179, 91, 54, 28);
    currentCondition->setAlignment(Qt::AlignCenter);
    currentCondition->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 11pt "Microsoft YaHei";
    )");

    weatherTip = new QLabel("正在加载天气...", this);
    weatherTip->setGeometry(10, 148, 351, 30);
    weatherTip->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    weatherTip->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 11pt "Microsoft YaHei";
    )");
}

void MainWindow::setupColdIndexPanel()
{
    coldIndexTitle = new QLabel("感\n冒\n指\n数", this);
    coldIndexTitle->setGeometry(10, 344, 32, 96);
    coldIndexTitle->setAlignment(Qt::AlignCenter);
    coldIndexTitle->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 10pt "Microsoft YaHei";
    )");

    coldIndexPanel = new QLabel("数据加载中...", this);
    coldIndexPanel->setGeometry(44, 348, 158, 88);
    coldIndexPanel->setAlignment(Qt::AlignCenter);
    coldIndexPanel->setWordWrap(true);
    coldIndexPanel->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 245);
        border: 2px solid rgba(255, 255, 255, 255);
        border-radius: 0px;
        color: rgb(55, 71, 79);
    )");
}

void MainWindow::setupForecastTable()
{
    // 7 行依次表示星期、日期、AQI、天气、图标、最高温和最低温。
    // 6 列表示截图中的六天预报。
    forecastTable = new QTableWidget(7, 6, this);
    forecastTable->setObjectName("forecastTable");
    forecastTable->setGeometry(370, 60, 420, 382);
    forecastTable->setShowGrid(true);
    forecastTable->setWordWrap(false);
    forecastTable->setSelectionMode(QAbstractItemView::NoSelection);
    forecastTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    forecastTable->setFocusPolicy(Qt::NoFocus);
    forecastTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    forecastTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    forecastTable->setIconSize(QSize(46, 46));
    forecastTable->horizontalHeader()->hide();
    forecastTable->verticalHeader()->hide();
    forecastTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    forecastTable->horizontalHeader()->setMinimumSectionSize(0);
    forecastTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    const QList<int> rowHeights = {34, 32, 48, 42, 146, 38, 38};
    for (int row = 0; row < rowHeights.size(); ++row) {
        forecastTable->setRowHeight(row, rowHeights.at(row));
    }

    forecastTable->setStyleSheet(R"(
        QTableWidget#forecastTable {
            background-color: rgba(0, 20, 25, 45);
            alternate-background-color: rgba(0, 0, 0, 0);
            border: 1px solid rgba(50, 220, 130, 150);
            gridline-color: rgba(50, 220, 130, 125);
            color: rgb(242, 248, 244);
            font: 10pt "Microsoft YaHei";
        }
        QTableWidget#forecastTable::item {
            padding: 2px;
            background-color: rgba(0, 20, 25, 35);
        }
        QTableWidget#forecastTable::item:selected {
            background-color: rgba(0, 20, 25, 35);
        }
        QTableWidget#forecastTable QHeaderView::section {
            background-color: rgba(0, 20, 25, 0);
            border: none;
        }
    )");
}

void MainWindow::populateForecastTable(
    const QVector<ForecastDayData>& forecast)
{
    const QColor normalColor(242, 248, 244);
    const QFont weekdayFont("Microsoft YaHei", 10, QFont::DemiBold);
    const QFont dateFont("Microsoft YaHei", 9);
    const QFont airQualityFont("Microsoft YaHei", 8);
    const QFont weatherFont("Microsoft YaHei", 10);
    const QFont temperatureFont("Microsoft YaHei", 10, QFont::DemiBold);

    // 把“创建单元格、设置对齐和样式”的重复代码提取为 lambda。
    // setItem() 后 QTableWidget 会接管 item 的所有权，不需要手动 delete。
    auto setTextItem = [this](int row, int column, const QString& text,
                              const QColor& color, const QFont& font) {
        auto* item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        item->setForeground(color);
        item->setBackground(QBrush(Qt::transparent));
        item->setFont(font);
        item->setFlags(Qt::ItemIsEnabled);
        forecastTable->setItem(row, column, item);
    };

    // 先清空旧数据，防止新城市预报不足六天时残留上一次的单元格。
    for (int column = 0; column < forecastTable->columnCount(); ++column) {
        for (int row = 0; row < forecastTable->rowCount(); ++row) {
            setTextItem(row, column, QString(), normalColor, weatherFont);
        }
    }

    for (int column = 0;
         column < forecast.size() && column < forecastTable->columnCount();
         ++column) {
        const ForecastDayData& day = forecast.at(column);

        setTextItem(0, column, day.weekday, normalColor, weekdayFont);
        setTextItem(1, column, day.date.toString("MM/dd"),
                    normalColor, dateFont);
        setTextItem(2, column, day.airQuality,
                    airQualityColor(day.airQuality), airQualityFont);
        setTextItem(3, column, day.condition.text,
                    normalColor, weatherFont);

        auto* iconItem = new QTableWidgetItem;
        iconItem->setTextAlignment(Qt::AlignCenter);
        iconItem->setBackground(QBrush(Qt::transparent));
        iconItem->setFlags(Qt::ItemIsEnabled);
        const QString iconPath = weatherIconPath(day.condition.iconCode);
        if (!iconPath.isEmpty()) {
            iconItem->setIcon(QIcon(iconPath));
        }
        forecastTable->setItem(4, column, iconItem);

        setTextItem(5, column, QString("%1°").arg(day.highTemperature),
                    normalColor, temperatureFont);
        setTextItem(6, column, QString("%1°").arg(day.lowTemperature),
                    normalColor, temperatureFont);
    }
}

QString MainWindow::weatherIconPath(const QString& weatherCode) const
{
    QString iconName = weatherCode.trimmed().toLower();
    if (iconName == "晴") {
        iconName = "clear-day";
    } else if (iconName == "多云") {
        iconName = "partly-cloudy-day";
    } else if (iconName == "阴") {
        iconName = "cloudy";
    } else if (iconName == "雨" || iconName == "小雨"
               || iconName == "中雨" || iconName == "大雨") {
        iconName = "rain";
    } else if (iconName == "雷雨" || iconName == "雷阵雨") {
        iconName = "thunderstorms-rain";
    }

    if (iconName.startsWith(":/")) {
        return QFileInfo::exists(iconName) ? iconName : QString();
    }
    if (!iconName.endsWith(".svg")) {
        iconName += ".svg";
    }

    const QString resourcePath = ":/weather/icons/" + iconName;
    if (QFileInfo::exists(resourcePath)) {
        return resourcePath;
    }
    const QString fallbackPath = ":/weather/icons/cloudy.svg";
    return QFileInfo::exists(fallbackPath) ? fallbackPath : QString();
}

QColor MainWindow::airQualityColor(const QString& airQuality) const
{
    if (airQuality == "优质" || airQuality == "优") {
        return QColor(87, 230, 157);
    }
    if (airQuality == "良好" || airQuality == "良") {
        return QColor(243, 212, 90);
    }
    if (airQuality == "轻度污染") {
        return QColor(255, 157, 69);
    }
    if (airQuality == "中度污染") {
        return QColor(255, 107, 82);
    }
    if (airQuality == "重度污染" || airQuality == "严重污染") {
        return QColor(238, 72, 72);
    }
    return QColor(242, 248, 244);
}

QString MainWindow::airQualityShortText(const QString& airQuality) const
{
    if (airQuality == "轻度污染") {
        return "轻污";
    }
    if (airQuality == "中度污染") {
        return "中污";
    }
    if (airQuality == "重度污染") {
        return "重污";
    }
    if (airQuality == "严重污染") {
        return "严重";
    }
    return airQuality;
}

void MainWindow::handleWeatherData(const WeatherData& data)
{
    // WeatherData 已完成 JSON 解析和单位转换。
    // 这里只做一件事：把模型字段映射到界面控件。
    textEdit->setPlainText(data.current.date.toString("yyyy-MM-dd"));
    city->setText(data.displayCityName.isEmpty()
                      ? data.cityName
                      : data.displayCityName);
    city->setToolTip(data.region);

    currentTemperature->setText(
        QString("%1°").arg(qRound(data.current.temperature)));
    currentCondition->setText(data.current.condition.text);
    lab2->setText(QString("%1%").arg(qRound(data.current.humidity)));
    lab6->setText(data.current.windDirection);
    lab8->setText(data.current.windLevel);
    lab4->setText(data.current.pm25 >= 0.0
                      ? QString::number(data.current.pm25, 'f', 0)
                      : "--");

    airQualityLabel->setText(
        airQualityShortText(data.current.airQuality));
    airQualityLabel->setStyleSheet(
        QString(
            "background-color: rgba(0, 0, 0, 0);"
            "color: %1;"
            "font: 12pt \"Microsoft YaHei\";")
            .arg(airQualityColor(data.current.airQuality).name()));
    airQualityLabel->setToolTip(
        data.current.aqi >= 0.0
            ? QString("US AQI：%1 (%2)")
                  .arg(qRound(data.current.aqi))
                  .arg(data.current.airQuality)
            : QString());

    const QString iconPath =
        weatherIconPath(data.current.condition.iconCode);
    if (!iconPath.isEmpty()) {
        currentWeatherIcon->setPixmap(
            QIcon(iconPath).pixmap(QSize(88, 88)));
    }

    weatherTip->setText(
        QString("天气提示：%1").arg(data.current.advice));
    weatherTip->setToolTip(QString());
    populateForecastTable(data.forecast);
    updateColdIndex(data);
}

void MainWindow::handleWeatherError(const QString& message)
{
    showQueryError(message);
}

void MainWindow::handleLoadingChanged(bool loading)
{
    refreshButton->setEnabled(!loading);
    if (loading) {
        weatherTip->setText("正在加载天气...");
        coldIndexPanel->setText("数据加载中...");
    }
}

void MainWindow::updateColdIndex(const WeatherData& data)
{
    // 感冒指数是一个本地启发式评分，不是医学诊断。
    // 分值越高表示寒冷、潮湿、大风或温差带来的不适风险越高。
    const CurrentWeatherData& current = data.current;
    int score = 0;
    if (current.apparentTemperature <= 0.0) {
        score += 4;
    } else if (current.apparentTemperature <= 5.0) {
        score += 3;
    } else if (current.apparentTemperature <= 12.0) {
        score += 2;
    } else if (current.apparentTemperature <= 18.0) {
        score += 1;
    }

    if (current.temperature <= 8.0) {
        ++score;
    }
    if (current.humidity >= 80.0
        && current.apparentTemperature <= 16.0) {
        ++score;
    }
    if (current.windSpeedKmh >= 20.0) {
        ++score;
    }
    if (current.todayTemperatureRange >= 10.0) {
        ++score;
    }
    if (current.hasPrecipitation) {
        ++score;
    }

    QString level;
    QString advice;
    QString color;
    if (score <= 1) {
        level = "低风险";
        advice = "天气较舒适，注意通风";
        color = "#2e7d32";
    } else if (score <= 4) {
        level = "中等风险";
        advice = "温差较明显，及时增减衣物";
        color = "#ef6c00";
    } else {
        level = "高风险";
        advice = "体感寒冷，注意防寒保暖";
        color = "#c62828";
    }

    coldIndexPanel->setText(
        QString(
            "<div style=\"text-align:center;\">"
            "<span style=\"font-size:15pt;font-weight:600;color:%1;\">%2</span>"
            "<br>"
            "<span style=\"font-size:8pt;color:#455a64;\">%3</span>"
            "</div>")
            .arg(color, level, advice));
    coldIndexPanel->setToolTip(
        QString("体感温度：%1°\n湿度：%2%\n风速：%3 km/h\n"
                "昼夜温差：%4°\n综合评分：%5")
            .arg(qRound(current.apparentTemperature))
            .arg(qRound(current.humidity))
            .arg(QString::number(current.windSpeedKmh, 'f', 1))
            .arg(QString::number(current.todayTemperatureRange, 'f', 1))
            .arg(score));
}

void MainWindow::showQueryError(const QString& message)
{
    weatherTip->setText("天气提示：查询失败，请稍后重试");
    weatherTip->setToolTip(message);
    coldIndexPanel->setText("暂无数据");
    coldIndexPanel->setToolTip(message);
    lab4->setText("--");
    airQualityLabel->setText("--");
    airQualityLabel->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(242, 248, 244);
        font: 12pt "Microsoft YaHei";
    )");
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    exitMenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    // globalPosition() 是屏幕坐标，mPos 是按下鼠标时记录的窗口内偏移。
    move(event->globalPosition().toPoint() - mPos);
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    mPos = event->globalPosition().toPoint() - pos();
}

void MainWindow::slot_searchCity()
{
    // 用户输入只负责触发服务层查询，窗口不直接构造 URL 或解析 JSON。
    const QString cityName = lineEdit->text().trimmed();
    lineEdit->clear();
    if (cityName.isEmpty()) {
        showQueryError("请输入城市名称");
        return;
    }
    weatherService->queryCity(cityName);
}

void MainWindow::slot_refreshWeather()
{
    weatherService->refresh();
}

void MainWindow::slot_exitapp()
{
    close();
}
