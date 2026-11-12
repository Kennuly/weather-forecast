#ifndef WEATHERSERVICE_H
#define WEATHERSERVICE_H

#include "weatherdata.h"

#include <QMap>
#include <QObject>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;

// WeatherService 负责与天气接口通信，并把 JSON 转换成 WeatherData。
// 它不直接操作任何 QLabel 或 QTableWidget，因此网络逻辑和界面逻辑相互独立。
class WeatherService : public QObject
{
    // Q_OBJECT 是自定义 signals/slots 的必要宏；
    // 构建时 moc 会为它生成额外代码。
    Q_OBJECT

public:
    // parent 用于 Qt 的对象树管理：父对象析构时会自动释放它。
    explicit WeatherService(QObject* parent = nullptr);

    // 查询一个城市；同名城市有多个时，内部会选择更可能是主城市的结果。
    void queryCity(const QString& cityName);

    // 使用当前已解析的城市重新查询。
    void refresh();

    // 返回最近一次解析成功的城市名，供刷新功能使用。
    QString currentCityName() const;

signals:
    // 网络请求开始或结束时发出，界面据此更新加载状态和按钮状态。
    void loadingChanged(bool loading);

    // 当前天气和空气质量都准备完成后发出完整数据。
    void weatherReady(const WeatherData& data);

    // 网络错误、JSON 格式错误或城市不存在时发出。
    void errorOccurred(const QString& message);

private:
    // 接口调用链：城市名 -> 经纬度 -> 当前天气/六日预报 -> 空气质量。
    void requestGeocoding(const QString& cityName, bool allowSuffixFallback,
                          int requestId);
    void requestWeather(double latitude, double longitude, int requestId);
    void requestAirQuality(double latitude, double longitude, int requestId);
    void handleGeocodingReply(QNetworkReply* reply, const QString& cityName,
                              bool allowSuffixFallback, int requestId);
    void handleWeatherReply(QNetworkReply* reply, int requestId);
    void handleAirQualityReply(QNetworkReply* reply, int requestId);
    void finishRequest();
    void failRequest(int requestId, const QString& message);
    bool isCurrentRequest(int requestId) const;
    void cancelReply(QPointer<QNetworkReply>& reply);

    // QNetworkAccessManager 统一管理 HTTP 请求；
    // 以 this 为父对象后，会随 WeatherService 一起释放。
    QNetworkAccessManager* m_network;

    // QPointer 在 reply 被删除后会自动变成 nullptr，避免悬空指针。
    QPointer<QNetworkReply> m_geocodeReply;
    QPointer<QNetworkReply> m_weatherReply;
    QPointer<QNetworkReply> m_airQualityReply;

    // 每次查询递增编号。旧请求即使晚到，也能通过编号判断并丢弃。
    int m_requestId = 0;
    QString m_currentCityName = "长沙";
    QString m_requestedCityName;

    // 先暂存各部分结果，等天气和空气质量都完成后一次交给界面。
    WeatherData m_pendingData;

    // 日期字符串 -> 当天空气质量，例如 "2026-11-12" -> "中度污染"。
    QMap<QString, QString> m_dailyAirQuality;

    // 两个并行请求的完成标记，以及整个查询是否已经结束的标记。
    bool m_weatherDone = false;
    bool m_airQualityDone = false;
    bool m_finished = false;
};

#endif // WEATHERSERVICE_H
