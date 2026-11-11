#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBrush>
#include <QDir>
#include <QFileInfo>
#include <QHeaderView>
#include <QMouseEvent>
#include <QTableWidgetItem>


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


    setupCurrentWeatherInfo();
    setupColdIndexPanel();

    setupForecastTable();
    populateForecastTable({
        {"昨天", "01/01", "优质", "阴", "cloudy", "10°", "-1°"},
        {"今天", "02/02", "良好", "阴", "cloudy", "10°", "-1°"},
        {"明天", "03/03", "轻度污染", "阴", "cloudy", "10°", "-1°"},
        {"后天", "04/04", "中度污染", "阴", "cloudy", "10°", "-1°"},
        {"周五", "05/05", "严重污染", "阴", "cloudy", "10°", "-1°"},
        {"周六", "06/06", "良", "阴", "cloudy", "10°", "-1°"}
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 添加顶部搜索和刷新按钮，并接通城市名搜索
void MainWindow::setupSearchControls()
{
    /**
     *  这里有三个状态：
     *  - QToolButton：按钮正常状态。
     *  - QToolButton:hover：鼠标悬停状态。
     *  - QToolButton:pressed：鼠标按下状态。
    **/
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
    )";

    searchButton = new QToolButton(this);
    searchButton->setObjectName("searchButton");
    searchButton->setGeometry(711, 10, 34, 31);
    searchButton->setIcon(QIcon(":/weather/actions/search.svg"));
    searchButton->setIconSize(QSize(18, 18));
    searchButton->setCursor(Qt::PointingHandCursor);    //setCursor(...)设置鼠标移动到按钮上方时，鼠标指针显示的形状。 Qt::PointingHandCursor 表示手指形状
    searchButton->setStyleSheet(buttonStyle);
    searchButton->setToolTip("搜索城市");               //设置鼠标悬停提示,当鼠标放在搜索按钮上停留一段时间，就会显示：设置的提示词

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
    )");
    refreshButton->setToolTip("刷新天气");

    connect(searchButton, &QToolButton::clicked, this, &MainWindow::slot_searchCity);
    connect(lineEdit, &QLineEdit::returnPressed, this, &MainWindow::slot_searchCity);
}

// 补全当前城市、温度、天气和空气质量摘要
void MainWindow::setupCurrentWeatherInfo()
{
    const QString transparentStyle = R"(
        background-color: rgba(0, 0, 0, 0);
        border: none;
    )";

    airQualityIcon = new QLabel(this);
    airQualityIcon->setGeometry(18, 40, 24, 24);
    airQualityIcon->setStyleSheet(transparentStyle);
    //这里的 setPixmap() 通常是 QLabel 的成员函数。它负责将传入的 QPixmap 设置为 QLabel 显示的图片。
    airQualityIcon->setPixmap(QIcon(":/weather/actions/leaf.svg").pixmap(QSize(24, 24)));   //将 QIcon 转换成指定尺寸的 QPixmap。
    airQualityLabel = new QLabel("优", this);
    airQualityLabel->setGeometry(42, 36, 44, 30);
    airQualityLabel->setAlignment(Qt::AlignCenter);
    airQualityLabel->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(85, 255, 0);
        font: 12pt "Microsoft YaHei";
    )");

    currentTemperature = new QLabel("9°", this);
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
    cityTit->setPixmap(QIcon(":/weather/actions/location.svg").pixmap(QSize(30, 30)));

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
    const QString currentIconPath = weatherIconPath("clear-day");
    if (!currentIconPath.isEmpty()) {
        currentWeatherIcon->setPixmap(QIcon(currentIconPath).pixmap(QSize(88, 88)));
    }

    currentCondition = new QLabel("晴", this);
    currentCondition->setGeometry(179, 91, 54, 28);
    currentCondition->setAlignment(Qt::AlignCenter);
    currentCondition->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 11pt "Microsoft YaHei";
    )");

    weatherTip = new QLabel("天气提示", this);
    weatherTip->setGeometry(10, 148, 100, 30);
    weatherTip->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    weatherTip->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 0);
        color: rgb(255, 255, 255);
        font: 11pt "Microsoft YaHei";
    )");
}

// 添加左下角的感冒指数标题和数据显示区域
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

    coldIndexPanel = new QLabel(this);
    coldIndexPanel->setGeometry(44, 348, 158, 88);
    coldIndexPanel->setStyleSheet(R"(
        background-color: rgba(255, 255, 255, 245);
        border: 2px solid rgba(255, 255, 255, 255);
        border-radius: 0px;
    )");
}

void MainWindow::setupForecastTable()
{
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

void MainWindow::populateForecastTable(const QVector<ForecastItem>& forecast)
{
    const QColor normalColor(242, 248, 244);

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

    for (int column = 0; column < forecast.size(); ++column) {
        const ForecastItem& item = forecast.at(column);

        QFont weekdayFont("Microsoft YaHei", 10, QFont::DemiBold);
        QFont dateFont("Microsoft YaHei", 9);
        QFont airQualityFont("Microsoft YaHei", 8);
        QFont weatherFont("Microsoft YaHei", 10);
        QFont temperatureFont("Microsoft YaHei", 10, QFont::DemiBold);

        setTextItem(0, column, item.weekday, normalColor, weekdayFont);
        setTextItem(1, column, item.date, normalColor, dateFont);
        setTextItem(2, column, item.airQuality,
                    airQualityColor(item.airQuality), airQualityFont);
        setTextItem(3, column, item.weather, normalColor, weatherFont);

        auto* iconItem = new QTableWidgetItem;
        iconItem->setTextAlignment(Qt::AlignCenter);
        iconItem->setBackground(QBrush(Qt::transparent));
        iconItem->setFlags(Qt::ItemIsEnabled);

        const QString iconPath = weatherIconPath(item.weatherCode);
        if (!iconPath.isEmpty()) {
            iconItem->setIcon(QIcon(iconPath));
        }
        forecastTable->setItem(4, column, iconItem);

        setTextItem(5, column, item.highTemperature, normalColor, temperatureFont);
        setTextItem(6, column, item.lowTemperature, normalColor, temperatureFont);
    }
}

QString MainWindow::weatherIconPath(const QString& weatherCode) const
{
    QString iconName = weatherCode.trimmed().toLower();

    // 同时兼容接口返回的中文天气描述和资源文件名。
    if (iconName == "晴") {
        iconName = "clear-day";
    } else if (iconName == "多云") {
        iconName = "partly-cloudy-day";
    } else if (iconName == "阴") {
        iconName = "cloudy";
    } else if (iconName == "雨" || iconName == "小雨" || iconName == "中雨"
               || iconName == "大雨") {
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

    // 新天气代码没有对应资源时，保证仍能显示一个有效图标。
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

// 点击搜索按钮或按回车后，把输入内容显示为当前城市
void MainWindow::slot_searchCity()
{
    const QString cityName = lineEdit->text().trimmed();
    if (!cityName.isEmpty()) {
        city->setText(cityName);
    }
    lineEdit->clear();
}

void MainWindow::slot_exitapp()
{
    //qApp->exit(0);
    this->close();
}
