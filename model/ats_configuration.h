#ifndef ATSCONFIGURATION_H
#define ATSCONFIGURATION_H

#include <string>
#include <model/interface/ats_ijson.hpp>
#include "thirdparty/IB/client/Contract.h"
#include "thirdparty/nlohmann/nlohmann_fwd.hpp"
#include "model/ats_order_request.h"
#include "model/IB/ats_ib_orders.h"

namespace trader {

constexpr const char *INDICATOR_NAME_ADX = "ADX";
constexpr const char *INDICATOR_NAME_MACD = "MACD";
constexpr const char *INDICATOR_NAME_AROON = "AROON";
constexpr const char *INDICATOR_NAME_ATR = "ATR";
constexpr const char *INDICATOR_NAME_BB = "Bollinger";
constexpr const char *INDICATOR_NAME_DONCHIAN = "Donchian";
constexpr const char *INDICATOR_NAME_MAMA = "MAMA";
constexpr const char *INDICATOR_NAME_MA = "MA";
constexpr const char *INDICATOR_NAME_ALMA = "ALMA";

constexpr const char *INDICATOR_PARAMETER_TIME_PERIOD = "TimePeriod";
constexpr const char *INDICATOR_PARAMETER_THRESHOLD= "Threshold";
constexpr const char *INDICATOR_PARAMETER_TIME_PERIOD_SLOW = "TimePeriodSlow";
constexpr const char *INDICATOR_PARAMETER_TIME_PERIOD_FAST = "TimePeriodFast";
constexpr const char *INDICATOR_PARAMETER_TIME_PERIOD_SIGNAL = "TimePeriodSignal";
constexpr const char *INDICATOR_PARAMETER_DEVIATION_UP = "DeviationUp";
constexpr const char *INDICATOR_PARAMETER_DEVIATION_DOWN = "DeviationDown";
constexpr const char *INDICATOR_PARAMETER_FAST_LIMIT = "FastLimit";
constexpr const char *INDICATOR_PARAMETER_SLOW_LIMIT = "SlowLimit";

constexpr const char *rootDir = "/Trader";

//----------------------------------------------------------------------------------------------------------------------
enum class BarResolution : int {
    Sec5, // realtime
    Sec30,
    Min1,
    Min5
};

//----------------------------------------------------------------------------------------------------------------------
static std::string barResolutionToAPIString(const BarResolution &barResolution) {
    switch (barResolution) {
        case BarResolution::Sec5:
            return "5 secs";
        case BarResolution::Sec30:
            return "30 secs";
        case BarResolution::Min1:
            return "1 min";
        case BarResolution::Min5:
            return "5 mins";
    }

    return "5 secs";
}

//----------------------------------------------------------------------------------------------------------------------
static std::string barResolutionToNiceString(const BarResolution &barResolution) {
    switch (barResolution) {
        case BarResolution::Sec5:
            return "5 secconds";
        case BarResolution::Sec30:
            return "30 seconds";
        case BarResolution::Min1:
            return "1 minute";
        case BarResolution::Min5:
            return "5 minutes";
    }

    return "5 secconds";
}

//----------------------------------------------------------------------------------------------------------------------
static int realTimeBarsForResolution(const BarResolution &barResolution) {
    // Real time bars come with 5s interval
    switch (barResolution) {
        case BarResolution::Sec5:
            return 1;
        case BarResolution::Sec30:
            return 6;
        case BarResolution::Min1:
            return 12;
        case BarResolution::Min5:
            return 60;
    }

    return 1;
}

//----------------------------------------------------------------------------------------------------------------------
static int secondsDurationForResolution(const BarResolution &barResolution) {
    // Real time bars come with 5s interval
    switch (barResolution) {
        case BarResolution::Sec5:
            return 5;
        case BarResolution::Sec30:
            return 30;
        case BarResolution::Min1:
            return 60;
        case BarResolution::Min5:
            return 60 * 5;
    }

    return 1;
}

//----------------------------------------------------------------------------------------------------------------------
class IBConfiguration {
    std::vector<Contract> m_activeStockContracts;
    std::vector<Contract> m_activeForexContracts;

    std::string m_stockMarketOpens = "15:30:00";
    std::string m_stockMarketCloses = "22:00:00";

    std::string m_fxMarketOpens = "23:15:00";
    std::string m_fxMarketCloses = "23:00:00";

    std::string m_stocksTradingDayStart = "15:31:00";
    std::string m_stocksTradingDayEnd = "21:55:00";

    std::string m_forexTradingDayStart = "23:16:00";
    std::string m_forexTradingDayEnd = "22:59:00";

public:

    std::string m_apiIP = "127.0.0.1";
    std::uint16_t m_apiPortDemo = 7497;
    std::uint16_t m_apiPortLive = 7496;
    std::string m_demoAccount = "DU1053213";
    std::string m_liveAccount = "U2364567";
    bool m_usingRealAccount = false;
    std::string m_historicalBarResolution = barResolutionToAPIString(BarResolution::Min5);
    std::string m_historicDataLength = "1 D";
    std::vector<Contract> m_contracts;
    double m_demoCommission = 5;
    double m_buyingPower = 20000;
    bool m_fxTraderEnabled = false;
    std::uint32_t m_realtimeBarAggregation = 6;
    bool m_saveRealTimeData = true;
    bool m_enableTradesLogging = true;
    std::string m_realTimeDataBaseFilePath;
    std::string m_aggregatedDataBaseFilePath;
    std::string m_tradesLogBaseFilePath;
    std::string m_downloaderBaseFilePath;
    std::string m_optimizerBaseFilePath;
    std::string m_accountStateBaseFilePath;
    std::string m_baseCurrency = "USD";
    double m_fxContractSize = 100000; // 1 Lot
    bool m_tradingHalted = true;
    std::vector<BarResolution> m_activeResolutions;
    std::shared_ptr<ATSOrderRequest> m_activeOrder;

    IBConfiguration();

    [[nodiscard]] std::string marketOpens() const;

    [[nodiscard]] std::string marketCloses() const;

    [[nodiscard]] std::string tradingDayStart() const;

    [[nodiscard]] std::string tradingDayEnd() const;

    std::string m_demoAsset;

    bool loadFromJson(const nlohmann::json &jsonObject);

    bool saveToJson(nlohmann::json &jsonObject) const;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSIndicator : public ATSIJson {
    bool m_enabled = false;
    std::string m_name;
    std::vector<std::string> m_signalNames;
    std::map<std::string, double> m_parameters;

    [[nodiscard]] nlohmann::json toJson() const override {
        nlohmann::json jsonObject;
        jsonObject["Enabled"] = m_enabled;
        jsonObject["Name"] = m_name;
        jsonObject["SignalNames"] = m_signalNames;
        jsonObject["Parameters"] = m_parameters;
        return jsonObject;
    }

    bool fromJson(nlohmann::json json) override {
        readValue<bool>(json, "Enabled", m_enabled);
        readValue<std::string>(json, "Name", m_name);

        auto it = json.find("SignalNames");

        if(it != json.end()){
            m_signalNames = it->get<std::vector<std::string>>();
        }

        it = json.find("Parameters");

        if(it != json.end()){

            m_parameters = it->get<std::map<std::string, double>>();
        }
        return true;
    }
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSConfiguration {

public:

    IBConfiguration m_ibConfiguration;
    static const std::int64_t s_numVisibleSeconds = 3600 * 4;
    bool m_showCandles = true;
    std::map<std::string, ATSIndicator> m_indicators;

    bool load(const std::string &path);

    [[nodiscard]] bool save(const std::string &path) const;

    ATSConfiguration();
};

//----------------------------------------------------------------------------------------------------------------------
class ATSCompletedOrders {

public:
    std::map<OrderId, ATSIBOrder> m_orders;

    bool loadFromJson(const nlohmann::json &jsonObject);

    bool saveToJson(nlohmann::json &jsonObject) const;

    bool load(const std::string &path);

    [[nodiscard]] bool save(const std::string &path) const;
};
}

#endif // ATSCONFIGURATION_H
