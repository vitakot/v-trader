#include "ats_configuration.h"
#include "thirdparty/nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include "ats_utils.h"
#include <QDebug>
#include <QStandardPaths>

namespace trader {

//----------------------------------------------------------------------------------------------------------------------
IBConfiguration::IBConfiguration() {
    {
        Contract contract;
        contract.symbol = "GOOG";
        contract.secType = "STK";
        contract.currency = m_baseCurrency;
        contract.exchange = "SMART";

        m_activeStockContracts.push_back(contract);
    }

    {
        Contract contract;
        contract.symbol = "EUR";
        contract.secType = "CASH";
        contract.currency = m_baseCurrency;
        contract.exchange = "IDEALPRO";

        m_activeForexContracts.push_back(contract);
    }

    m_contracts.clear();

    if (m_fxTraderEnabled) {
        m_contracts.insert(m_contracts.begin(), m_activeForexContracts.begin(), m_activeForexContracts.end());
    } else {
        m_contracts.insert(m_contracts.begin(), m_activeStockContracts.begin(), m_activeStockContracts.end());
    }

    m_realTimeDataBaseFilePath = (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(
            rootDir) + QString("/Logs")).toStdString();

    m_aggregatedDataBaseFilePath = (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(
            rootDir) + QString("/Logs")).toStdString();

    m_tradesLogBaseFilePath = (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                               QString(rootDir) + QString("/Logs")).toStdString();

    m_downloaderBaseFilePath = (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(
            rootDir) + QString("/HistoricalData")).toStdString();

    m_optimizerBaseFilePath = (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(
            rootDir) + QString("/OptimizerResults")).toStdString();

    m_accountStateBaseFilePath = (QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(
            rootDir) + QString("/Logs")).toStdString();

    m_activeResolutions.clear();
    m_activeResolutions.push_back(BarResolution::Sec30);
    m_activeResolutions.push_back(BarResolution::Min5);
}

//----------------------------------------------------------------------------------------------------------------------
std::string IBConfiguration::marketOpens() const {
    if (m_fxTraderEnabled) {
        return m_fxMarketOpens;
    } else {
        return m_stockMarketOpens;
    }
}

//----------------------------------------------------------------------------------------------------------------------
std::string IBConfiguration::marketCloses() const {
    if (m_fxTraderEnabled) {
        return m_fxMarketCloses;
    } else {
        return m_stockMarketCloses;
    }
}

//----------------------------------------------------------------------------------------------------------------------
std::string IBConfiguration::tradingDayStart() const {
    if (m_fxTraderEnabled) {
        return m_forexTradingDayStart;
    } else {
        return m_stocksTradingDayStart;
    }
}

//----------------------------------------------------------------------------------------------------------------------
std::string IBConfiguration::tradingDayEnd() const {
    if (m_fxTraderEnabled) {
        return m_forexTradingDayEnd;
    } else {
        return m_stocksTradingDayEnd;
    }
}

//----------------------------------------------------------------------------------------------------------------------
bool IBConfiguration::loadFromJson(const nlohmann::json &jsonObject) {
    try {
        readValue<std::string>(jsonObject, "APIIP", m_apiIP);
        readValue<std::uint16_t>(jsonObject, "APIPortDemo", m_apiPortDemo);
        readValue<std::uint16_t>(jsonObject, "APIPortLive", m_apiPortLive);
        readValue<bool>(jsonObject, "UsingRealAccount", m_usingRealAccount);
        readValue<std::string>(jsonObject, "HistoricDataLength", m_historicDataLength);
        readValue<std::string>(jsonObject, "HistoricalBarResolution", m_historicalBarResolution);
        readValue<double>(jsonObject, "BuyingPower", m_buyingPower);
        readValue<double>(jsonObject, "DemoCommision", m_demoCommission);
        readValue<bool>(jsonObject, "FxTraderEnabled", m_fxTraderEnabled);
        readValue<std::string>(jsonObject, "StocksTradingDayStart", m_stocksTradingDayStart);
        readValue<std::string>(jsonObject, "StocksTradingDayEnd", m_stocksTradingDayEnd);
        readValue<std::string>(jsonObject, "ForexTradingDayStart", m_forexTradingDayStart);
        readValue<std::string>(jsonObject, "ForexTradingDayEnd", m_forexTradingDayEnd);
        readValue<std::string>(jsonObject, "StockMarketOpens", m_stockMarketOpens);
        readValue<std::string>(jsonObject, "StockMarketCloses", m_stockMarketCloses);
        readValue<std::string>(jsonObject, "FXMarketOpens", m_fxMarketOpens);
        readValue<std::string>(jsonObject, "FXMarketCloses", m_fxMarketCloses);
        readValue<std::string>(jsonObject, "DemoAsset", m_demoAsset);
        readValue<bool>(jsonObject, "SaveRealTimeData", m_saveRealTimeData);
        readValue<bool>(jsonObject, "EnableTradesLogging", m_enableTradesLogging);
        readValue<std::string>(jsonObject, "RealTimeDataBaseFilePath", m_realTimeDataBaseFilePath);
        readValue<std::string>(jsonObject, "AggregatedDataBaseFilePath", m_aggregatedDataBaseFilePath);
        readValue<std::string>(jsonObject, "TradesLogBaseFilePath", m_tradesLogBaseFilePath);
        readValue<std::string>(jsonObject, "DownloaderBaseFilePath", m_downloaderBaseFilePath);
        readValue<std::string>(jsonObject, "OptimizerBaseFilePath", m_optimizerBaseFilePath);
        readValue<std::string>(jsonObject, "AccountStateBaseFilePath", m_accountStateBaseFilePath);
        readValue<std::uint32_t>(jsonObject, "RealtimeBarAggregation", m_realtimeBarAggregation);
        readValue<std::string>(jsonObject, "BaseCurrency", m_baseCurrency);
        readValue<double>(jsonObject, "FXContractSize", m_fxContractSize);
        readValue<bool>(jsonObject, "TradingHalted", m_tradingHalted);

        {
            nlohmann::json activeStockContracts = jsonObject["ActiveStockContracts"];

            m_activeStockContracts.clear();

            for (const auto &contractObject : activeStockContracts) {
                Contract contract;
                readValue<std::string>(contractObject, "Symbol", contract.symbol);
                readValue<std::string>(contractObject, "SecurityType", contract.secType);
                readValue<std::string>(contractObject, "Currency", contract.currency);
                readValue<std::string>(contractObject, "Exchange", contract.exchange);
                m_activeStockContracts.push_back(contract);
            }
        }

        {
            nlohmann::json activeForexContracts = jsonObject["ActiveForexContracts"];

            m_activeForexContracts.clear();

            for (const auto &contractObject : activeForexContracts) {
                Contract contract;
                readValue<std::string>(contractObject, "Symbol", contract.symbol);
                readValue<std::string>(contractObject, "SecurityType", contract.secType);
                readValue<std::string>(contractObject, "Currency", contract.currency);
                readValue<std::string>(contractObject, "Exchange", contract.exchange);
                m_activeForexContracts.push_back(contract);
            }
        }

        m_contracts.clear();

        if (m_fxTraderEnabled) {
            m_contracts.insert(m_contracts.begin(), m_activeForexContracts.begin(), m_activeForexContracts.end());
        } else {
            m_contracts.insert(m_contracts.begin(), m_activeStockContracts.begin(), m_activeStockContracts.end());
        }

        {
            auto it = jsonObject.find("ActiveResolutions");

            if (it != jsonObject.end()) {
                m_activeResolutions = std::vector<BarResolution>(*it);
            }
        }

        {
            auto it = jsonObject.find("ActiveOrder");

            if (it != jsonObject.end()) {
                m_activeOrder = std::make_shared<ATSOrderRequest>();
                if (!m_activeOrder->loadFromJson(*it)) {
                    m_activeOrder = nullptr;
                }
            }
        }
    }
    catch (nlohmann::json::exception &) {
        qCritical() << "Couldn't parse json file";
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool IBConfiguration::saveToJson(nlohmann::json &jsonObject) const {
    try {
        jsonObject["APIIP"] = m_apiIP;
        jsonObject["APIPortDemo"] = m_apiPortDemo;
        jsonObject["APIPortLive"] = m_apiPortLive;
        jsonObject["UsingRealAccount"] = m_usingRealAccount;
        jsonObject["HistoricDataLength"] = m_historicDataLength;
        jsonObject["HistoricalBarResolution"] = m_historicalBarResolution;
        jsonObject["BuyingPower"] = m_buyingPower;
        jsonObject["DemoCommision"] = m_demoCommission;
        jsonObject["FxTraderEnabled"] = m_fxTraderEnabled;
        jsonObject["StocksTradingDayStart"] = m_stocksTradingDayStart;
        jsonObject["StocksTradingDayEnd"] = m_stocksTradingDayEnd;
        jsonObject["ForexTradingDayStart"] = m_forexTradingDayStart;
        jsonObject["ForexTradingDayEnd"] = m_forexTradingDayEnd;
        jsonObject["StockMarketOpens"] = m_stockMarketOpens;
        jsonObject["StockMarketCloses"] = m_stockMarketCloses;
        jsonObject["FXMarketOpens"] = m_fxMarketOpens;
        jsonObject["FXMarketCloses"] = m_fxMarketCloses;
        jsonObject["DemoAsset"] = m_demoAsset;
        jsonObject["SaveRealTimeData"] = m_saveRealTimeData;
        jsonObject["EnableTradesLogging"] = m_enableTradesLogging;
        jsonObject["RealTimeDataBaseFilePath"] = m_realTimeDataBaseFilePath;
        jsonObject["AggregatedDataBaseFilePath"] = m_aggregatedDataBaseFilePath;
        jsonObject["TradesLogBaseFilePath"] = m_tradesLogBaseFilePath;
        jsonObject["DownloaderBaseFilePath"] = m_downloaderBaseFilePath;
        jsonObject["OptimizerBaseFilePath"] = m_optimizerBaseFilePath;
        jsonObject["AccountStateBaseFilePath"] = m_accountStateBaseFilePath;
        jsonObject["RealtimeBarAggregation"] = m_realtimeBarAggregation;
        jsonObject["BaseCurrency"] = m_baseCurrency;
        jsonObject["FXContractSize"] = m_fxContractSize;
        jsonObject["ActiveResolutions"] = m_activeResolutions;
        jsonObject["TradingHalted"] = m_tradingHalted;

        {
            nlohmann::json activeStockContracts = nlohmann::json::array();

            for (const auto &contract : m_activeStockContracts) {
                nlohmann::json contractObject;
                contractObject["Symbol"] = contract.symbol;
                contractObject["SecurityType"] = contract.secType;
                contractObject["Currency"] = contract.currency;
                contractObject["Exchange"] = contract.exchange;
                activeStockContracts.push_back(contractObject);
            }

            jsonObject["ActiveStockContracts"] = activeStockContracts;
        }
        {
            nlohmann::json activeForexContracts = nlohmann::json::array();

            for (const auto &contract : m_activeForexContracts) {
                nlohmann::json contractObject;
                contractObject["Symbol"] = contract.symbol;
                contractObject["SecurityType"] = contract.secType;
                contractObject["Currency"] = contract.currency;
                contractObject["Exchange"] = contract.exchange;
                activeForexContracts.push_back(contractObject);
            }

            jsonObject["ActiveForexContracts"] = activeForexContracts;
        }
        {
            if (m_activeOrder) {
                nlohmann::json orderObject;
                if (m_activeOrder->saveToJson(orderObject)) {
                    jsonObject["ActiveOrder"] = orderObject;
                }
            }
        }
    }
    catch (nlohmann::json::exception &) {
        qCritical() << "Couldn't parse json file";
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
ATSConfiguration::ATSConfiguration() {

    ATSIndicator adx;
    adx.m_name = INDICATOR_NAME_ADX;
    adx.m_signalNames.emplace_back("ADX");
    adx.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD, 14.0);
    adx.m_parameters.insert_or_assign(INDICATOR_PARAMETER_THRESHOLD, 25.0);

    ATSIndicator macd;
    macd.m_name = INDICATOR_NAME_MACD;
    macd.m_signalNames.emplace_back("MACD");
    macd.m_signalNames.emplace_back("Signal");
    macd.m_signalNames.emplace_back("Histogram");
    macd.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD_SLOW, 26.0);
    macd.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD_FAST, 12.0);
    macd.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD_SIGNAL, 9.0);

    ATSIndicator aroon;
    aroon.m_name = INDICATOR_NAME_AROON;
    aroon.m_signalNames.emplace_back("Down");
    aroon.m_signalNames.emplace_back("Up");
    aroon.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD, 14.0);

    ATSIndicator atr;
    atr.m_name = INDICATOR_NAME_ATR;
    atr.m_signalNames.emplace_back("ATR");
    atr.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD, 14.0);

    ATSIndicator bb;
    bb.m_name = INDICATOR_NAME_BB;
    bb.m_signalNames.emplace_back("Upper");
    bb.m_signalNames.emplace_back("Middle");
    bb.m_signalNames.emplace_back("Lower");
    bb.m_parameters.insert_or_assign(INDICATOR_PARAMETER_DEVIATION_UP, 2.0);
    bb.m_parameters.insert_or_assign(INDICATOR_PARAMETER_DEVIATION_DOWN, 2.0);
    bb.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD, 14.0);

    ATSIndicator donchian;
    donchian.m_name = INDICATOR_NAME_DONCHIAN;
    donchian.m_signalNames.emplace_back("Upper");
    donchian.m_signalNames.emplace_back("Middle");
    donchian.m_signalNames.emplace_back("Lower");
    donchian.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD, 14.0);

    ATSIndicator mama;
    mama.m_name = INDICATOR_NAME_MAMA;
    mama.m_signalNames.emplace_back("MAMA");
    mama.m_signalNames.emplace_back("FAMA");
    mama.m_parameters.insert_or_assign(INDICATOR_PARAMETER_FAST_LIMIT, 0.5);
    mama.m_parameters.insert_or_assign(INDICATOR_PARAMETER_SLOW_LIMIT, 0.05);

    ATSIndicator alma;
    alma.m_name = INDICATOR_NAME_ALMA;
    alma.m_signalNames.emplace_back("ALMA");
    alma.m_parameters.insert_or_assign(INDICATOR_PARAMETER_TIME_PERIOD, 20.0);

    m_indicators.insert_or_assign(INDICATOR_NAME_ADX, adx);
    m_indicators.insert_or_assign(INDICATOR_NAME_MACD, macd);
    m_indicators.insert_or_assign(INDICATOR_NAME_AROON, aroon);
    m_indicators.insert_or_assign(INDICATOR_NAME_ATR, atr);
    m_indicators.insert_or_assign(INDICATOR_NAME_BB, bb);
    m_indicators.insert_or_assign(INDICATOR_NAME_DONCHIAN, donchian);
    m_indicators.insert_or_assign(INDICATOR_NAME_MAMA, mama);
    m_indicators.insert_or_assign(INDICATOR_NAME_ALMA, alma);
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSConfiguration::load(const std::string &path) {
    std::ifstream ifs(path);

    if (!ifs.is_open()) {
        qCritical() << "Couldn't open json file";
        return false;
    }

    try {
        nlohmann::json jsonObject = nlohmann::json::parse(ifs);
        readValue<bool>(jsonObject, "ShowCandles", m_showCandles);

        nlohmann::json indicatorsObject = jsonObject["Indicators"];

        for (const auto &indObject : indicatorsObject) {
            ATSIndicator indicator;
            if (indicator.fromJson(indObject)) {
                m_indicators.insert_or_assign(indicator.m_name, indicator);
            }
        }

        nlohmann::json cfgIBObject = jsonObject["IBConfiguration"];
        m_ibConfiguration.loadFromJson(cfgIBObject);
    }

    catch (nlohmann::json::exception &) {
        qCritical() << "Couldn't parse json file";
        ifs.close();
        return save(path);
    }

    ifs.close();

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSConfiguration::save(const std::string &path) const {
    nlohmann::json cfgObject;

    cfgObject["ShowCandles"] = m_showCandles;

    nlohmann::json indicatorsObject = nlohmann::json::array();

    for (const auto &indicator : m_indicators) {
        indicatorsObject.push_back(indicator.second.toJson());
    }

    nlohmann::json cfgIBObject;
    m_ibConfiguration.saveToJson(cfgIBObject);
    cfgObject["IBConfiguration"] = cfgIBObject;
    cfgObject["Indicators"] = indicatorsObject;

    std::ofstream file(path);

    if (file.is_open()) {
        file << cfgObject.dump(4);
        return true;
    } else {
        qCritical() << "Couldn't save json file";
        return false;
    }
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSCompletedOrders::loadFromJson(const nlohmann::json &jsonObject) {
    nlohmann::json completedOrders = jsonObject["CompletedOrders"];

    m_orders.clear();

    try {
        for (const auto &orderObject : completedOrders) {
            ATSIBOrder order;
            order.fromJson(orderObject);
            m_orders.insert_or_assign(order.m_order.permId, order);
        }
    }
    catch (nlohmann::json::exception &) {
        qCritical() << "Couldn't parse json file";
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSCompletedOrders::saveToJson(nlohmann::json &jsonObject) const {
    nlohmann::json completedOrders = nlohmann::json::array();
    try {

        for (const auto &order : m_orders) {
            completedOrders.push_back(order.second.toJson());
        }

        jsonObject["CompletedOrders"] = completedOrders;
    }
    catch (nlohmann::json::exception &) {
        qCritical() << "Couldn't parse json file";
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSCompletedOrders::load(const std::string &path) {
    std::ifstream ifs(path);

    if (!ifs.is_open()) {
        ifs.close();

        if (!save(path)) {
            return false;
        }
    }

    ifs.open(path);

    if (!ifs.is_open()) {
        qCritical() << "Couldn't open json file";
        return false;
    }

    try {
        nlohmann::json jsonObject = nlohmann::json::parse(ifs);
        loadFromJson(jsonObject);
    }

    catch (nlohmann::json::exception &) {
        qCritical() << "Couldn't parse json file";
        ifs.close();
        return save(path);
    }

    ifs.close();

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSCompletedOrders::save(const std::string &path) const {
    nlohmann::json cfgObject;

    if (!saveToJson(cfgObject)) {
        return false;
    }

    std::ofstream file(path);

    if (file.is_open()) {
        file << cfgObject.dump(4);
        return true;
    } else {
        qCritical() << "Couldn't save json file";
        return false;
    }
}

}
