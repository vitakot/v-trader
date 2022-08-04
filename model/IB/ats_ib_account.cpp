#include "ats_ib_account.h"
#include "ats_ib_client.hpp"
#include "thirdparty/IB/client/EClientSocket.h"
#include <QDebug>
#include <QTimer>
#include <fstream>
#include <QDateTime>

namespace trader {

//---------------------------------------------------------------------------------------------------------------------
struct AccountSummaryTags {

    static std::string AccountType;
    static std::string NetLiquidation;
    static std::string TotalCashValue;
    static std::string SettledCash;
    static std::string AccruedCash;
    static std::string BuyingPower;
    static std::string EquityWithLoanValue;
    static std::string PreviousEquityWithLoanValue;
    static std::string GrossPositionValue;
    static std::string ReqTEquity;
    static std::string ReqTMargin;
    static std::string SMA;
    static std::string InitMarginReq;
    static std::string MaintMarginReq;
    static std::string AvailableFunds;
    static std::string ExcessLiquidity;
    static std::string Cushion;
    static std::string FullInitMarginReq;
    static std::string FullMaintMarginReq;
    static std::string FullAvailableFunds;
    static std::string FullExcessLiquidity;
    static std::string LookAheadNextChange;
    static std::string LookAheadInitMarginReq;
    static std::string LookAheadMaintMarginReq;
    static std::string LookAheadAvailableFunds;
    static std::string LookAheadExcessLiquidity;
    static std::string HighestSeverity;
    static std::string DayTradesRemaining;
    static std::string Leverage;

    static std::string getAllTags();
};

std::string AccountSummaryTags::AccountType = "AccountType";
std::string AccountSummaryTags::NetLiquidation = "NetLiquidation";
std::string AccountSummaryTags::TotalCashValue = "TotalCashValue";
std::string AccountSummaryTags::SettledCash = "SettledCash";
std::string AccountSummaryTags::AccruedCash = "AccruedCash";
std::string AccountSummaryTags::BuyingPower = "BuyingPower";
std::string AccountSummaryTags::EquityWithLoanValue = "EquityWithLoanValue";
std::string AccountSummaryTags::PreviousEquityWithLoanValue = "PreviousEquityWithLoanValue";
std::string AccountSummaryTags::GrossPositionValue = "GrossPositionValue";
std::string AccountSummaryTags::ReqTEquity = "ReqTEquity";
std::string AccountSummaryTags::ReqTMargin = "ReqTMargin";
std::string AccountSummaryTags::SMA = "SMA";
std::string AccountSummaryTags::InitMarginReq = "InitMarginReq";
std::string AccountSummaryTags::MaintMarginReq = "MaintMarginReq";
std::string AccountSummaryTags::AvailableFunds = "AvailableFunds";
std::string AccountSummaryTags::ExcessLiquidity = "ExcessLiquidity";
std::string AccountSummaryTags::Cushion = "Cushion";
std::string AccountSummaryTags::FullInitMarginReq = "FullInitMarginReq";
std::string AccountSummaryTags::FullMaintMarginReq = "FullMaintMarginReq";
std::string AccountSummaryTags::FullAvailableFunds = "FullAvailableFunds";
std::string AccountSummaryTags::FullExcessLiquidity = "FullExcessLiquidity";
std::string AccountSummaryTags::LookAheadNextChange = "LookAheadNextChange";
std::string AccountSummaryTags::LookAheadInitMarginReq = "LookAheadInitMarginReq";
std::string AccountSummaryTags::LookAheadMaintMarginReq = "LookAheadMaintMarginReq";
std::string AccountSummaryTags::LookAheadAvailableFunds = "LookAheadAvailableFunds";
std::string AccountSummaryTags::LookAheadExcessLiquidity = "LookAheadExcessLiquidity";
std::string AccountSummaryTags::HighestSeverity = "HighestSeverity";
std::string AccountSummaryTags::DayTradesRemaining = "DayTradesRemaining";
std::string AccountSummaryTags::Leverage = "Leverage";

std::string AccountSummaryTags::getAllTags() {
    return AccountType + "," + NetLiquidation + "," + TotalCashValue + "," + SettledCash + "," + AccruedCash + "," +
           BuyingPower + "," +
           EquityWithLoanValue + "," + PreviousEquityWithLoanValue + "," + GrossPositionValue + "," + ReqTEquity
           + "," + ReqTMargin + "," + SMA + "," + InitMarginReq + "," + MaintMarginReq + "," + AvailableFunds +
           "," + ExcessLiquidity + "," + Cushion + "," +
           FullInitMarginReq + "," + FullMaintMarginReq + "," + FullAvailableFunds + "," + FullExcessLiquidity
           + "," + LookAheadNextChange + "," + LookAheadInitMarginReq + "," + LookAheadMaintMarginReq + "," +
           LookAheadAvailableFunds + "," +
           LookAheadExcessLiquidity + "," + HighestSeverity + "," + DayTradesRemaining + "," + Leverage;
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSIBPosition::toJsonString() const {
    return toJson().dump(4);
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSIBPosition::toString() const {
    std::string retVal;
    retVal = "ATSIBPosition, symbol: " + m_contract.symbol + ", position: " + std::to_string(m_position) +
             ", averageCost: " + std::to_string(
            m_averageCost) + ", marketPrice: " + std::to_string(m_marketPrice) + ", marketValue: " +
             std::to_string(m_marketValue) +
             ", unrealizedPNL: " + std::to_string(m_unrealizedPNL) + ", realizedPNL: " +
             std::to_string(m_realizedPNL);

    return retVal;
}

//---------------------------------------------------------------------------------------------------------------------
nlohmann::json ATSIBPosition::toJson() const {
    nlohmann::json jsonObject;

    jsonObject["Account"] = m_account;
    jsonObject["Position"] = m_position;
    jsonObject["AverageCost"] = m_averageCost;
    jsonObject["MarketPrice"] = m_marketPrice;
    jsonObject["MarketValue"] = m_marketValue;
    jsonObject["UnrealizedPNL"] = m_unrealizedPNL;
    jsonObject["RealizedPNL"] = m_realizedPNL;

    nlohmann::json contractObject;

    contractObject["Symbol"] = m_contract.symbol;
    contractObject["SecurityType"] = m_contract.secType;
    contractObject["Currency"] = m_contract.currency;
    contractObject["Exchange"] = m_contract.exchange;

    jsonObject["Contract"] = contractObject;

    return jsonObject;
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSIBPositionBrief::toJsonString() const {
    return toJson().dump(4);
}

//---------------------------------------------------------------------------------------------------------------------
nlohmann::json ATSIBPositionBrief::toJson() const {

    nlohmann::json jsonObject;

    jsonObject["Account"] = m_account;
    jsonObject["Position"] = m_position;
    jsonObject["AverageCost"] = m_averageCost;

    nlohmann::json contractObject;

    contractObject["Symbol"] = m_contract.symbol;
    contractObject["SecurityType"] = m_contract.secType;
    contractObject["Currency"] = m_contract.currency;
    contractObject["Exchange"] = m_contract.exchange;

    jsonObject["Contract"] = contractObject;

    return jsonObject;
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSIBPositionBrief::toString() const {
    std::string retVal;
    retVal = "ATSIBPosition, symbol: " + m_contract.symbol + ", position: " + std::to_string(m_position) +
             ", averageCost: " + std::to_string(
            m_averageCost);
    return retVal;
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSIBPortfolio::toJsonString() const {
    return toJson().dump(4);
}

//---------------------------------------------------------------------------------------------------------------------
nlohmann::json ATSIBPortfolio::toJson() const {
    nlohmann::json jsonObject;
    jsonObject["Account"] = m_account;

    nlohmann::json positionsArray = nlohmann::json::array();

    for (auto &entry : m_positions) {
        positionsArray.push_back(entry.second.toJson());
    }

    jsonObject["Positions"] = positionsArray;
    std::string m_account;
    std::vector<ATSIBPosition> m_positions;

    return jsonObject;
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSIBAccountInfo::toJsonString() const {
    nlohmann::json jsonObject;

    jsonObject["BuyingPower"] = m_buyingPower;
    jsonObject["Leverage"] = m_leverage;
    jsonObject["Account"] = m_account;
    jsonObject["Curency"] = m_currency;
    jsonObject["AvailableFunds"] = m_availableFunds;
    jsonObject["NetLiquidation"] = m_netLiquidation;
    jsonObject["TotalCashBalance"] = m_totalCashBalance;
    jsonObject["CashBalance"] = m_cashBalance;
    jsonObject["AccruedCash"] = m_accruedCash;
    jsonObject["SettledCash"] = m_settledCash;
    jsonObject["RealizedPnL"] = m_realizedPNL;
    jsonObject["UnrealizedPnL"] = m_unrealizedPNL;
    jsonObject["InitMarginReq"] = m_initMarginReq;
    jsonObject["MaintMarginReq"] = m_maintMarginReq;
    jsonObject["Cushion"] = m_cushion;
    jsonObject["Portfolio"] = m_portfolio.toJson();

    return jsonObject.dump(4);
}

//---------------------------------------------------------------------------------------------------------------------
bool ATSIBAccountInfo::positionForSymbol(const std::string &symbol, double &position, std::int64_t &contractId) {
    for (auto &item : m_portfolio.m_positions) {
        if (item.second.m_contract.symbol == symbol) {
            position = item.second.m_position;
            contractId = item.second.m_contract.conId;
            return true;
        }
    }

    position = 0.0;
    contractId = -1;
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccountInfo::updatePositionForSymbol(const std::string &symbol, double position, std::int64_t contractId) {
    for (auto &item : m_portfolio.m_positions) {
        if (item.second.m_contract.symbol == symbol && item.second.m_contract.conId == contractId) {
            item.second.m_position = position;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
std::vector<std::int64_t> ATSIBAccountInfo::contractsId() {
    std::vector<std::int64_t> retVal;

    for (auto &item : m_portfolio.m_positions) {
        retVal.push_back(item.first);
    }

    return retVal;
}

//---------------------------------------------------------------------------------------------------------------------
struct ATSIBAccount::P {
    ATSIBClient *m_ibClient = nullptr;
    ATSIBAccount *m_parent = nullptr;
    ATSIBAccountInfo m_accountInfo;
    IBCleaner m_ibCleaner;
    std::recursive_mutex m_mutex;
    std::string m_accountNumber;
    QTimer *m_positionsUpdater = nullptr;
    std::ofstream m_accountStateFile;

    P(ATSIBAccount *parent) {
        m_parent = parent;
    }
};

//---------------------------------------------------------------------------------------------------------------------
ATSIBAccount::ATSIBAccount(QObject *parent) : QObject(parent), m_p(spimpl::make_unique_impl<P>(this)) {
    m_p->m_ibClient = qobject_cast<ATSIBClient *>(parent);
}

//---------------------------------------------------------------------------------------------------------------------
ATSIBAccount::~ATSIBAccount() {
    m_p->m_accountStateFile.flush();
    m_p->m_accountStateFile.close();
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::init() {
    if (m_p->m_ibClient->configuration().m_apiPortDemo) {
        m_p->m_accountNumber = m_p->m_ibClient->configuration().m_demoAccount;
    } else {
        m_p->m_accountNumber = m_p->m_ibClient->configuration().m_liveAccount;
    }

    m_p->m_accountStateFile.open(m_p->m_ibClient->configuration().m_accountStateBaseFilePath);
    m_p->m_positionsUpdater = new QTimer(this);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::subscribeAllData() {
    auto tickerId = TickerIdGenerator::nextId();

    m_p->m_ibClient->socket()->reqAccountSummary(static_cast<int>(tickerId), "All",
                                                 "$LEDGER:" + m_p->m_ibClient->configuration().m_baseCurrency);

    m_p->m_ibCleaner.m_cleaners.insert(std::pair<TickerId, std::function<void()>>(tickerId, [=]() {
        m_p->m_ibClient->socket()->cancelAccountSummary(static_cast<int>(tickerId));
    }));

    m_p->m_ibClient->socket()->reqAccountUpdates(true, m_p->m_accountNumber);

    connect(m_p->m_positionsUpdater, &QTimer::timeout, this, [&]() {
        m_p->m_ibClient->socket()->reqPositions();
    });

    m_p->m_positionsUpdater->start(1000 * 30);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::unsubscribeAllData() {
    m_p->m_ibClient->socket()->cancelPositions();
    m_p->m_positionsUpdater->stop();
    m_p->m_ibCleaner.cleanAll();
    m_p->m_ibClient->socket()->reqAccountUpdates(false, m_p->m_accountNumber);
}

//---------------------------------------------------------------------------------------------------------------------
ATSIBAccountInfo ATSIBAccount::accountInfo() const {
    return m_p->m_accountInfo;
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::onAccountInfo(TickerId, const std::string &account, const std::string &curency,
                                 const std::string &tag,
                                 const std::string &value) {
    m_p->m_accountInfo.m_account = account;
    m_p->m_accountInfo.m_currency = curency;

    if (tag == AccountSummaryTags::BuyingPower) {

        m_p->m_accountInfo.m_buyingPower = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::Leverage) {
        m_p->m_accountInfo.m_leverage = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::AvailableFunds) {
        m_p->m_accountInfo.m_availableFunds = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::NetLiquidation) {
        m_p->m_accountInfo.m_netLiquidation = QString::fromStdString(value).toDouble();
    } else if (tag == "TotalCashBalance") {
        m_p->m_accountInfo.m_totalCashBalance = QString::fromStdString(value).toDouble();
    } else if (tag == "CashBalance") {
        m_p->m_accountInfo.m_cashBalance = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::InitMarginReq) {
        m_p->m_accountInfo.m_initMarginReq = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::MaintMarginReq) {
        m_p->m_accountInfo.m_initMarginReq = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::Cushion) {
        m_p->m_accountInfo.m_cushion = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::AccruedCash) {
        m_p->m_accountInfo.m_accruedCash = QString::fromStdString(value).toDouble();
    } else if (tag == AccountSummaryTags::SettledCash) {
        m_p->m_accountInfo.m_settledCash = QString::fromStdString(value).toDouble();
    } else if (tag == "RealizedPnL") {
        m_p->m_accountInfo.m_realizedPNL = QString::fromStdString(value).toDouble();
    } else if (tag == "UnrealizedPnL") {
        m_p->m_accountInfo.m_unrealizedPNL = QString::fromStdString(value).toDouble();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::onAccountInfoEnd(TickerId tickerId) {
    m_p->m_ibCleaner.clean(tickerId);

    m_p->m_accountStateFile << QDateTime::currentSecsSinceEpoch()
                            << "," << m_p->m_accountInfo.m_realizedPNL << "," << m_p->m_accountInfo.m_unrealizedPNL
                            << std::endl;
    m_p->m_accountStateFile.flush();
    emit accountInfoUpdated(m_p->m_accountInfo);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::onAccountPositionReceived(const ATSIBPosition &position) {
    m_p->m_accountInfo.m_portfolio.m_positions.insert_or_assign(position.m_contract.conId, position);
    emit accountPositionUpdated(position);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::onAccountPositionBriefReceived(const ATSIBPositionBrief &position) {
    m_p->m_accountInfo.updatePositionForSymbol(position.m_contract.symbol, position.m_position,
                                               position.m_contract.conId);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSIBAccount::onOrderCompleted(const ATSIBOrder &) {
    m_p->m_ibClient->socket()->reqPositions();
}
}
