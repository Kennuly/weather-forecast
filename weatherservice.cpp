#include "weatherservice.h"

#include <QDate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>
#include <QtMath>

namespace {

// 以下函数只在本 .cpp 文件中使用。
// 放在匿名 namespace 中可避免污染全局命名空间。

// Open-Meteo 使用 WMO 数字天气代码，这里转换成中文和本地图标代码。
WeatherConditionData conditionFromCode(int weatherCode)
{
    if (weatherCode == 0) {
        return {"晴", "clear-day"};
    }
    if (weatherCode == 1) {
        return {"大部晴朗", "clear-day"};
    }
    if (weatherCode == 2) {
        return {"多云", "partly-cloudy-day"};
    }
    if (weatherCode == 3) {
        return {"阴", "cloudy"};
    }
    if (weatherCode == 45 || weatherCode == 48) {
        return {"雾", "cloudy"};
    }
    if (weatherCode == 51 || weatherCode == 53 || weatherCode == 55) {
        return {"毛毛雨", "rain"};
    }
    if (weatherCode == 56 || weatherCode == 57
        || weatherCode == 66 || weatherCode == 67) {
        return {"冻雨", "rain"};
    }
    if (weatherCode == 61) {
        return {"小雨", "rain"};
    }
    if (weatherCode == 63) {
        return {"中雨", "rain"};
    }
    if (weatherCode == 65) {
        return {"大雨", "rain"};
    }
    if (weatherCode >= 71 && weatherCode <= 77) {
        return {"降雪", "cloudy"};
    }
    if (weatherCode == 80 || weatherCode == 81 || weatherCode == 82) {
        return {"阵雨", "rain"};
    }
    if (weatherCode == 85 || weatherCode == 86) {
        return {"阵雪", "cloudy"};
    }
    if (weatherCode == 95) {
        return {"雷雨", "thunderstorms-rain"};
    }
    if (weatherCode == 96 || weatherCode == 99) {
        return {"雷暴冰雹", "thunderstorms-rain"};
    }
    return {"未知", "cloudy"};
}

// 把 US AQI 数值转换成界面显示的空气质量等级。
QString airQualityFromAqi(double aqi)
{
    if (aqi <= 50.0) {
        return "优";
    }
    if (aqi <= 100.0) {
        return "良";
    }
    if (aqi <= 150.0) {
        return "轻度污染";
    }
    if (aqi <= 200.0) {
        return "中度污染";
    }
    if (aqi <= 300.0) {
        return "重度污染";
    }
    return "严重污染";
}

// 气象风向通常用 0~359 度表示，这里简化为八个方向。
QString windDirectionText(double degrees)
{
    static const QStringList directions = {
        "北风", "东北风", "东风", "东南风",
        "南风", "西南风", "西风", "西北风"
    };
    const int index = qRound(degrees / 45.0) % directions.size();
    return directions.at(index);
}

// 把 km/h 风速近似转换为常用的蒲福风力等级。
QString windLevelText(double speedKmh)
{
    static const QList<double> limits = {
        1, 6, 12, 20, 29, 39, 50, 62, 75, 89, 103, 118
    };
    int level = 0;
    while (level < limits.size() && speedKmh >= limits.at(level)) {
        ++level;
    }
    return QString("%1级").arg(level);
}

// 根据当前数据生成一句基础出行提示。
QString weatherAdvice(const CurrentWeatherData& current)
{
    if (current.airQuality.contains("污染")) {
        return "空气污染，建议减少户外活动";
    }
    if (current.hasPrecipitation) {
        return "有降水，出门记得带伞";
    }
    if (current.windSpeedKmh >= 20.0) {
        return "风力较大，注意添衣";
    }
    if (current.temperature <= 5.0) {
        return "气温较低，注意保暖";
    }
    if (current.temperature >= 30.0) {
        return "气温较高，注意防晒补水";
    }
    return "天气平稳，适合出行";
}

// 对比预报日期和当前日期，生成“昨天/今天/明天/后天/周几”。
QString weekdayText(const QDate& date, const QDate& currentDate)
{
    const int offset = currentDate.daysTo(date);
    if (offset == -1) {
        return "昨天";
    }
    if (offset == 0) {
        return "今天";
    }
    if (offset == 1) {
        return "明天";
    }
    if (offset == 2) {
        return "后天";
    }

    static const QStringList weekdays = {
        "周一", "周二", "周三", "周四", "周五", "周六", "周日"
    };
    return weekdays.at(date.dayOfWeek() - 1);
}

// 统一设置请求头、15 秒超时等公共参数。
QNetworkRequest createRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "WeatherForecastDemo/1.0");
    request.setTransferTimeout(15000);
    return request;
}

// 将网络返回的 QByteArray 解析为 JSON 根对象。
// QJsonParseError 用来区分“网络成功但 JSON 格式错误”的情况。
bool parseJson(const QByteArray& payload, QJsonObject& root, QString& error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        error = "接口返回的 JSON 格式错误";
        return false;
    }
    root = document.object();
    return true;
}

// Open-Meteo 有时只收录“厦门市”而不收录“厦门”。
// 该函数判断是否适合自动追加“市”后进行第二次查询。
bool canAppendCitySuffix(const QString& cityName, bool allowSuffixFallback)
{
    if (!allowSuffixFallback || cityName.endsWith("市")
        || cityName.size() > 8) {
        return false;
    }

    bool containsChinese = false;
    for (const QChar character : cityName) {
        const ushort code = character.unicode();
        if (code >= 0x4E00 && code <= 0x9FFF) {
            containsChinese = true;
            break;
        }
    }
    return containsChinese;
}

} // namespace

WeatherService::WeatherService(QObject* parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
}

// 开始一次新查询。
// 每次先取消旧请求并递增编号，防止快速搜索时旧结果覆盖新结果。
void WeatherService::queryCity(const QString& cityName)
{
    const QString requestedName = cityName.trimmed();
    if (requestedName.isEmpty()) {
        emit errorOccurred("请输入城市名称");
        return;
    }

    ++m_requestId;
    cancelReply(m_geocodeReply);
    cancelReply(m_weatherReply);
    cancelReply(m_airQualityReply);

    m_requestedCityName = requestedName;
    m_currentCityName = requestedName;
    m_pendingData = WeatherData();
    m_pendingData.requestedCity = requestedName;
    m_dailyAirQuality.clear();
    m_weatherDone = false;
    m_airQualityDone = false;
    m_finished = false;

    emit loadingChanged(true);
    requestGeocoding(requestedName, true, m_requestId);
}

// 刷新时复用最近一次解析出的完整城市名。
void WeatherService::refresh()
{
    queryCity(m_currentCityName);
}

QString WeatherService::currentCityName() const
{
    return m_currentCityName;
}

void WeatherService::requestGeocoding(const QString& cityName,
                                      bool allowSuffixFallback,
                                      int requestId)
{
    if (!isCurrentRequest(requestId)) {
        return;
    }

    cancelReply(m_geocodeReply);

    QUrl url("https://geocoding-api.open-meteo.com/v1/search");
    QUrlQuery query;
    query.addQueryItem("name", cityName);
    query.addQueryItem("count", "10");
    query.addQueryItem("language", "zh");
    query.addQueryItem("format", "json");
    url.setQuery(query);

    // QNetworkAccessManager 是异步请求：get() 会立即返回 reply，
    // 真正的数据要等 finished 信号发出后才可以读取。
    QNetworkReply* reply = m_network->get(createRequest(url));
    m_geocodeReply = reply;

    // lambda 捕获本次 reply 和 requestId，响应到达后交给专用函数解析。
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, cityName, allowSuffixFallback, requestId]() {
                handleGeocodingReply(reply, cityName, allowSuffixFallback,
                                     requestId);
            });
}

void WeatherService::requestWeather(double latitude, double longitude,
                                    int requestId)
{
    if (!isCurrentRequest(requestId)) {
        return;
    }

    cancelReply(m_weatherReply);

    QUrl url("https://api.open-meteo.com/v1/forecast");
    QUrlQuery query;
    query.addQueryItem("latitude", QString::number(latitude, 'f', 6));
    query.addQueryItem("longitude", QString::number(longitude, 'f', 6));
    query.addQueryItem(
        "current",
        "temperature_2m,relative_humidity_2m,apparent_temperature,"
        "is_day,weather_code,wind_speed_10m,wind_direction_10m");
    query.addQueryItem("daily",
                       "weather_code,temperature_2m_max,temperature_2m_min");
    query.addQueryItem("timezone", "auto");
    query.addQueryItem("past_days", "1");
    query.addQueryItem("forecast_days", "5");
    url.setQuery(query);

    // 天气请求只依赖经纬度，因此可以和空气质量请求并行执行。
    QNetworkReply* reply = m_network->get(createRequest(url));
    m_weatherReply = reply;
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, requestId]() {
                handleWeatherReply(reply, requestId);
            });
}

void WeatherService::requestAirQuality(double latitude, double longitude,
                                       int requestId)
{
    if (!isCurrentRequest(requestId)) {
        return;
    }

    cancelReply(m_airQualityReply);

    QUrl url("https://air-quality-api.open-meteo.com/v1/air-quality");
    QUrlQuery query;
    query.addQueryItem("latitude", QString::number(latitude, 'f', 6));
    query.addQueryItem("longitude", QString::number(longitude, 'f', 6));
    query.addQueryItem("current", "us_aqi,pm2_5");
    query.addQueryItem("hourly", "us_aqi,pm2_5");
    query.addQueryItem("timezone", "auto");
    query.addQueryItem("past_days", "1");
    query.addQueryItem("forecast_days", "5");
    url.setQuery(query);

    // hourly 数据会在响应解析阶段按天聚合，用于填充六日空气质量。
    QNetworkReply* reply = m_network->get(createRequest(url));
    m_airQualityReply = reply;
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, requestId]() {
                handleAirQualityReply(reply, requestId);
            });
}

void WeatherService::handleGeocodingReply(QNetworkReply* reply,
                                          const QString& cityName,
                                          bool allowSuffixFallback,
                                          int requestId)
{
    // 如果用户已经发起了更新的一次查询，就丢弃这个旧响应。
    if (!isCurrentRequest(requestId) || reply != m_geocodeReply) {
        reply->deleteLater();
        return;
    }

    // 先解除成员引用，再安排 QObject 在事件循环中安全释放自己。
    m_geocodeReply = nullptr;
    if (reply->error() != QNetworkReply::NoError) {
        const QString error = reply->errorString();
        reply->deleteLater();
        failRequest(requestId, QString("城市查询失败：%1").arg(error));
        return;
    }

    QJsonObject root;
    QString error;
    const bool validJson = parseJson(reply->readAll(), root, error);
    reply->deleteLater();
    if (!validJson) {
        failRequest(requestId, error);
        return;
    }

    const bool canRetryWithSuffix =
        canAppendCitySuffix(m_requestedCityName, allowSuffixFallback);
    const QJsonArray results = root.value("results").toArray();
    if (results.isEmpty()) {
        // “厦门”会得到空 results，而“厦门市”可以正常工作。
        if (canRetryWithSuffix) {
            requestGeocoding(m_requestedCityName + "市", false, requestId);
            return;
        }
        failRequest(requestId, QString("没有找到城市：%1").arg(cityName));
        return;
    }

    QJsonObject bestResult;
    double bestScore = -1.0;
    double bestPopulation = 0.0;
    // 同名地点可能有很多。这里按人口和行政中心代码打分，
    // 避免“长沙”被误解析成其他同名小镇。
    for (const QJsonValue& value : results) {
        const QJsonObject candidate = value.toObject();
        const double population = candidate.value("population").toDouble();
        const QString featureCode = candidate.value("feature_code").toString();

        double score = population;
        if (featureCode == "PPLC") {
            score += 200000000.0;
        } else if (featureCode == "PPLA") {
            score += 100000000.0;
        } else if (featureCode.startsWith("PPLA")) {
            score += 50000000.0;
        }

        if (score > bestScore) {
            bestScore = score;
            bestPopulation = population;
            bestResult = candidate;
        }
    }

    // 小结果没有命中主城市时，尝试追加“市”再查询一次。
    if (bestPopulation < 500000.0 && canRetryWithSuffix) {
        requestGeocoding(m_requestedCityName + "市", false, requestId);
        return;
    }

    m_pendingData.latitude = bestResult.value("latitude").toDouble();
    m_pendingData.longitude = bestResult.value("longitude").toDouble();
    m_pendingData.cityName =
        bestResult.value("name").toString(m_requestedCityName);
    m_pendingData.displayCityName = m_pendingData.cityName;
    if (m_pendingData.displayCityName.endsWith("市")
        && !m_requestedCityName.endsWith("市")) {
        m_pendingData.displayCityName.chop(1);
    }

    m_pendingData.region = bestResult.value("admin1").toString();
    const QString country = bestResult.value("country").toString();
    if (!country.isEmpty() && country != m_pendingData.region) {
        m_pendingData.region =
            m_pendingData.region.isEmpty()
                ? country
                : QString("%1 · %2").arg(m_pendingData.region, country);
    }

    m_currentCityName = m_pendingData.cityName;

    // 拿到经纬度后，天气和空气质量两个请求可以同时开始。
    requestWeather(m_pendingData.latitude, m_pendingData.longitude, requestId);
    requestAirQuality(m_pendingData.latitude, m_pendingData.longitude,
                      requestId);
}

void WeatherService::handleWeatherReply(QNetworkReply* reply, int requestId)
{
    // reply 必须同时属于当前查询，且是最新一次天气请求。
    if (!isCurrentRequest(requestId) || reply != m_weatherReply) {
        reply->deleteLater();
        return;
    }

    m_weatherReply = nullptr;
    if (reply->error() != QNetworkReply::NoError) {
        const QString error = reply->errorString();
        reply->deleteLater();
        failRequest(requestId, QString("天气加载失败：%1").arg(error));
        return;
    }

    QJsonObject root;
    QString error;
    const bool validJson = parseJson(reply->readAll(), root, error);
    reply->deleteLater();
    if (!validJson) {
        failRequest(requestId, error);
        return;
    }

    // 当前天气 JSON 的主要结构：
    // root["current"] -> 当前温度、湿度、风向等
    // root["daily"]   -> 日期、最高温和最低温数组
    const QJsonObject current = root.value("current").toObject();
    const QJsonObject daily = root.value("daily").toObject();
    if (current.isEmpty() || daily.isEmpty()) {
        failRequest(requestId, "天气 JSON 中缺少 current 或 daily 数据");
        return;
    }

    // 先把 JSON 字段复制到强类型对象中。
    // 这样界面层不用知道 JSON 字段名，也不容易写错类型。
    CurrentWeatherData currentData;
    currentData.aqi = m_pendingData.current.aqi;
    currentData.pm25 = m_pendingData.current.pm25;
    currentData.airQuality = m_pendingData.current.airQuality;
    currentData.weatherCode = current.value("weather_code").toInt();
    currentData.condition = conditionFromCode(currentData.weatherCode);
    currentData.temperature = current.value("temperature_2m").toDouble();
    currentData.apparentTemperature =
        current.value("apparent_temperature").toDouble();
    currentData.humidity =
        current.value("relative_humidity_2m").toDouble();
    currentData.windSpeedKmh =
        current.value("wind_speed_10m").toDouble();
    currentData.windDirectionDegrees =
        current.value("wind_direction_10m").toDouble();
    currentData.windDirection =
        windDirectionText(currentData.windDirectionDegrees);
    currentData.windLevel = windLevelText(currentData.windSpeedKmh);
    currentData.hasPrecipitation =
        (currentData.weatherCode >= 51 && currentData.weatherCode <= 67)
        || (currentData.weatherCode >= 80 && currentData.weatherCode <= 82)
        || (currentData.weatherCode >= 95 && currentData.weatherCode <= 99);

    currentData.date =
        QDate::fromString(current.value("time").toString().left(10),
                          "yyyy-MM-dd");
    if (!currentData.date.isValid()) {
        currentData.date = QDate::currentDate();
    }

    // daily 中的每个数组使用相同下标共同描述同一天。
    const QJsonArray dates = daily.value("time").toArray();
    const QJsonArray codes = daily.value("weather_code").toArray();
    const QJsonArray maximums =
        daily.value("temperature_2m_max").toArray();
    const QJsonArray minimums =
        daily.value("temperature_2m_min").toArray();

    // 最多显示六列；同时取所有数组长度最小值，避免访问越界。
    int count = dates.size();
    count = qMin(count, 6);
    count = qMin(count, static_cast<int>(codes.size()));
    count = qMin(count, static_cast<int>(maximums.size()));
    count = qMin(count, static_cast<int>(minimums.size()));

    m_pendingData.forecast.clear();
    m_pendingData.forecast.reserve(count);
    for (int index = 0; index < count; ++index) {
        const QDate date =
            QDate::fromString(dates.at(index).toString(), "yyyy-MM-dd");
        if (!date.isValid()) {
            continue;
        }

        const int high = qRound(maximums.at(index).toDouble());
        const int low = qRound(minimums.at(index).toDouble());
        if (date == currentData.date) {
            currentData.todayTemperatureRange = high - low;
        }

        ForecastDayData day;
        day.date = date;
        day.weekday = weekdayText(date, currentData.date);
        day.condition = conditionFromCode(codes.at(index).toInt());
        day.highTemperature = high;
        day.lowTemperature = low;
        m_pendingData.forecast.append(day);
    }

    m_pendingData.current = currentData;
    m_weatherDone = true;

    // 天气可能先返回，也可能空气质量先返回。
    // finishRequest() 只在两个请求都完成时才通知界面。
    finishRequest();
}

void WeatherService::handleAirQualityReply(QNetworkReply* reply, int requestId)
{
    if (!isCurrentRequest(requestId) || reply != m_airQualityReply) {
        reply->deleteLater();
        return;
    }

    m_airQualityReply = nullptr;
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        m_airQualityDone = true;
        finishRequest();
        return;
    }

    QJsonObject root;
    QString error;
    const bool validJson = parseJson(reply->readAll(), root, error);
    reply->deleteLater();
    if (!validJson) {
        m_airQualityDone = true;
        finishRequest();
        return;
    }

    // current 用于左侧实时显示，hourly 用于计算未来几天空气质量。
    const QJsonObject current = root.value("current").toObject();
    if (current.value("us_aqi").isDouble()) {
        m_pendingData.current.aqi = current.value("us_aqi").toDouble();
        m_pendingData.current.airQuality =
            airQualityFromAqi(m_pendingData.current.aqi);
    }
    if (current.value("pm2_5").isDouble()) {
        m_pendingData.current.pm25 =
            current.value("pm2_5").toDouble();
    }

    const QJsonObject hourly = root.value("hourly").toObject();
    const QJsonArray times = hourly.value("time").toArray();
    const QJsonArray aqiValues = hourly.value("us_aqi").toArray();
    int hourCount = times.size();
    hourCount = qMin(hourCount, static_cast<int>(aqiValues.size()));

    // 同一天有 24 个 AQI 小时值。
    // 这里取当天最大值，作为该日污染风险较高的等级。
    QMap<QString, double> dailyMaximums;
    for (int hour = 0; hour < hourCount; ++hour) {
        if (!aqiValues.at(hour).isDouble()) {
            continue;
        }
        const QString dateKey = times.at(hour).toString().left(10);
        const double value = aqiValues.at(hour).toDouble();
        dailyMaximums[dateKey] =
            qMax(dailyMaximums.value(dateKey, 0.0), value);
    }

    m_dailyAirQuality.clear();
    for (auto iterator = dailyMaximums.constBegin();
         iterator != dailyMaximums.constEnd(); ++iterator) {
        m_dailyAirQuality.insert(
            iterator.key(), airQualityFromAqi(iterator.value()));
    }

    m_airQualityDone = true;
    finishRequest();
}

void WeatherService::finishRequest()
{
    // 两个并行请求都完成前，不向界面发送半成品数据。
    if (m_finished || !m_weatherDone || !m_airQualityDone) {
        return;
    }

    // 把每日 AQI 合并到六日预报中；今天优先使用实时 AQI。
    const QString todayKey =
        m_pendingData.current.date.toString("yyyy-MM-dd");
    for (ForecastDayData& day : m_pendingData.forecast) {
        const QString dateKey = day.date.toString("yyyy-MM-dd");
        day.airQuality = m_dailyAirQuality.value(dateKey, "--");
        if (dateKey == todayKey
            && m_pendingData.current.airQuality != "--") {
            day.airQuality = m_pendingData.current.airQuality;
        }
    }

    m_pendingData.current.advice = weatherAdvice(m_pendingData.current);
    // 先完成所有数据整理，再通过信号一次性交给 MainWindow。
    m_finished = true;
    m_currentCityName = m_pendingData.cityName;
    emit weatherReady(m_pendingData);
    emit loadingChanged(false);
}

void WeatherService::failRequest(int requestId, const QString& message)
{
    if (!isCurrentRequest(requestId)) {
        return;
    }

    // 任意关键请求失败时，取消同一批次的其余请求。
    m_finished = true;
    cancelReply(m_geocodeReply);
    cancelReply(m_weatherReply);
    cancelReply(m_airQualityReply);
    emit errorOccurred(message);
    emit loadingChanged(false);
}

bool WeatherService::isCurrentRequest(int requestId) const
{
    return requestId == m_requestId && !m_finished;
}

void WeatherService::cancelReply(QPointer<QNetworkReply>& reply)
{
    if (!reply) {
        return;
    }

    // 先断开 finished 连接，避免 abort() 后仍触发旧响应处理。
    disconnect(reply.data(), nullptr, this, nullptr);
    reply->abort();

    // deleteLater() 不在当前函数中立即 delete，
    // 而是等控制权返回 Qt 事件循环后再释放，更安全。
    reply->deleteLater();
    reply = nullptr;
}
