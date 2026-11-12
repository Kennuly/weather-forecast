#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QDate>
#include <QMetaType>
#include <QString>
#include <QVector>

// 一种天气的文字描述和对应资源图标代码。
// 例如：{ "小雨", "rain" }，界面会根据 iconCode 查找 qrc 中的 SVG。
struct WeatherConditionData {
    QString text;
    QString iconCode;
};

// 当前时刻的天气数据。
// -1 表示接口没有返回该项，界面据此显示 "--"。
struct CurrentWeatherData {
    QDate date;                         // 天气数据的日期
    WeatherConditionData condition;     // 天气文字和图标代码
    int weatherCode = 0;                // Open-Meteo 使用的 WMO 天气代码
    double temperature = 0.0;           // 当前温度，单位：摄氏度
    double apparentTemperature = 0.0;   // 体感温度
    double humidity = 0.0;              // 相对湿度，单位：百分比
    double windSpeedKmh = 0.0;          // 风速，单位：km/h
    double windDirectionDegrees = 0.0;  // 风向角度，0 度表示北
    double aqi = -1.0;                  // US AQI，-1 表示暂无数据
    double pm25 = -1.0;                 // PM2.5 浓度
    double todayTemperatureRange = 0.0; // 今天最高温与最低温之差
    bool hasPrecipitation = false;      // 是否包含雨、雪等降水天气
    QString windDirection;              // 已转换好的中文风向
    QString windLevel;                  // 已转换好的风力等级
    QString airQuality = "--";          // 已转换好的空气质量等级
    QString advice;                     // 根据当前天气生成的提示语
};

// 右侧六日预报中每一天的数据。
struct ForecastDayData {
    QDate date;                       // 完整日期，用于关联每日 AQI
    QString weekday;                  // 昨天、今天、明天或周几
    QString airQuality = "--";        // 当天的空气质量
    WeatherConditionData condition;   // 当天的天气和图标
    int highTemperature = 0;          // 当天最高温
    int lowTemperature = 0;           // 当天最低温
};

// 一次城市查询完成后，交给界面层使用的完整数据。
// MainWindow 不再直接处理 JSON 和网络对象，只接收这个对象。
struct WeatherData {
    QString requestedCity;              // 用户输入的城市名
    QString cityName;                   // 地理编码返回的完整城市名
    QString displayCityName;            // 适合显示在界面上的城市名
    QString region;                     // 省份、国家等地区信息
    double latitude = 0.0;              // 纬度
    double longitude = 0.0;             // 经度
    CurrentWeatherData current;         // 当前天气
    QVector<ForecastDayData> forecast;  // 右侧六日预报
};

// 注册为 Qt 元类型，方便以后跨线程传递 WeatherData；
// 当前信号仍在 GUI 线程中直接传递。
Q_DECLARE_METATYPE(WeatherData)

#endif // WEATHERDATA_H
