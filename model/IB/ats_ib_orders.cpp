#include "ats_ib_orders.h"
#include "thirdparty/IB/client/Order.h"
#include "model/IB/ats_ib_client.hpp"
#include "model/ats_utils.h"
#include "thirdparty/IB/client/EClientSocket.h"
#include "thirdparty/IB/client/Execution.h"
#include <QDebug>
#include <QTime>
#include <cmath>
#include <fstream>
#include <QMessageBox>

namespace trader {

//----------------------------------------------------------------------------------------------------------------------
std::string ATSIBOrder::toJsonString() const {
    return toJson().dump(4);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrder::fromJson(const nlohmann::json &json) {
    nlohmann::json orderObject = json["Order"];
    nlohmann::json contractObject = json["Contract"];
    nlohmann::json orderStateObject = json["OrderState"];

    readValue<double>(orderObject, "Position", m_position);
    readValue<double>(orderObject, "ExitPrice", m_exitPrice);
    readValue<OrderId>(orderObject, "OrderId", m_order.orderId);
    readValue<int>(orderObject, "OrderPermanentId", m_order.permId);
    readValue<long>(orderObject, "ClientId", m_order.clientId);
    readValue<std::string>(orderObject, "Account", m_order.account);
    readValue<std::string>(orderObject, "Action", m_order.action);
    readValue<std::string>(orderObject, "OrderType", m_order.orderType);
    readValue<double>(orderObject, "TotalQuantity", m_order.totalQuantity);
    readValue<double>(orderObject, "CashQantity", m_order.cashQty);
    readValue<double>(orderObject, "LimitPrice", m_order.lmtPrice);
    readValue<double>(orderObject, "AuxPrice", m_order.auxPrice);

    readValue<std::string>(contractObject, "Symbol", m_contract.symbol);
    readValue<std::string>(contractObject, "SecurityType", m_contract.secType);
    readValue<std::string>(contractObject, "Exchange", m_contract.exchange);
    readValue<std::string>(contractObject, "Currency", m_contract.currency);

    readValue<std::string>(orderStateObject, "StatusString", m_orderState.status);
    readValue<double>(orderStateObject, "Commission", m_orderState.commission);
    readValue<std::string>(orderStateObject, "CompletedTime", m_orderState.completedTime);

    m_orderStatus.fromJson(json["OrderStatus"]);
}

//----------------------------------------------------------------------------------------------------------------------
nlohmann::json ATSIBOrder::toJson() const {
    nlohmann::json jsonObject;
    nlohmann::json orderObject;
    nlohmann::json contractObject;
    nlohmann::json orderStateObject;

    orderObject["OrderId"] = m_order.orderId;
    orderObject["Position"] = m_position;
    orderObject["ExitPrice"] = m_exitPrice;
    orderObject["OrderPermanentId"] = m_order.permId;
    orderObject["ClientId"] = m_order.clientId;
    orderObject["Account"] = m_order.account;
    orderObject["Action"] = m_order.action;
    orderObject["OrderType"] = m_order.orderType;
    orderObject["TotalQuantity"] = m_order.totalQuantity;
    orderObject["CashQantity"] = m_order.cashQty;
    orderObject["LimitPrice"] = m_order.lmtPrice;
    orderObject["AuxPrice"] = m_order.auxPrice;

    contractObject["Symbol"] = m_contract.symbol;
    contractObject["SecurityType"] = m_contract.secType;
    contractObject["Exchange"] = m_contract.exchange;
    contractObject["Currency"] = m_contract.currency;

    orderStateObject["StatusString"] = m_orderState.status;
    orderStateObject["Commission"] = m_orderState.commission;
    orderStateObject["CompletedTime"] = m_orderState.completedTime;

    jsonObject["Order"] = orderObject;
    jsonObject["Contract"] = contractObject;
    jsonObject["OrderState"] = orderStateObject;
    jsonObject["OrderStatus"] = m_orderStatus.toJson();

    return jsonObject;
}

//----------------------------------------------------------------------------------------------------------------------
std::string ATSIBOrderStatus::toJsonString() const {
    return toJson().dump(4);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrderStatus::fromJson(const nlohmann::json &json) {
    readValue<OrderId>(json, "OrderId", m_orderId);
    readValue<std::string>(json, "Status", m_status);
    readValue<double>(json, "Filled", m_filled);
    readValue<double>(json, "Remaining", m_remaining);
    readValue<double>(json, "AverageFillPrice", m_avgFillPrice);
    readValue<int>(json, "PermanentId", m_permId);
    readValue<double>(json, "LastFillPrice", m_lastFillPrice);
    readValue<int>(json, "ClientId", m_clientId);
    readValue<std::string>(json, "WhyHeld", m_whyHeld);
    readValue<double>(json, "MarketCapPrice", m_mktCapPrice);
}

//----------------------------------------------------------------------------------------------------------------------
nlohmann::json ATSIBOrderStatus::toJson() const {
    nlohmann::json jsonObject;

    jsonObject["OrderId"] = m_orderId;
    jsonObject["Status"] = m_status;
    jsonObject["Filled"] = m_filled;
    jsonObject["Remaining"] = m_remaining;
    jsonObject["AverageFillPrice"] = m_avgFillPrice;
    jsonObject["PermanentId"] = m_permId;
    jsonObject["ParentId"] = m_parentId;
    jsonObject["LastFillPrice"] = m_lastFillPrice;
    jsonObject["ClientId"] = m_clientId;
    jsonObject["WhyHeld"] = m_whyHeld;
    jsonObject["MarketCapPrice"] = m_mktCapPrice;

    return jsonObject;
}

//----------------------------------------------------------------------------------------------------------------------
struct ATSIBOrders::P {

    ATSIBClient *m_ibClient = nullptr;
    std::map<std::int64_t, ATSIBOrder> m_orders;
    ATSIBAccountInfo m_accountInfo;
    IBCleaner m_ibCleaner;
    std::atomic<OrderId> m_nextValidOrderId = -1;
    ATSOrderRequestQueue m_orderRequests;
    std::ofstream m_tradesLogFile;

    QTime m_openTime;
    QTime m_closeTime;

    ~P() {
#ifndef SIMULATED_TRADING
        m_tradesLogFile.flush();
        m_tradesLogFile.close();
#endif
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSIBOrders::ATSIBOrders(QObject *parent) : QObject(parent), m_p(spimpl::make_unique_impl<P>()) {
    m_p->m_ibClient = qobject_cast<ATSIBClient *>(parent);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::init() {
    m_p->m_openTime = QTime::fromString(m_p->m_ibClient->configuration().marketOpens().c_str(), "hh:mm:ss");
    m_p->m_closeTime = QTime::fromString(m_p->m_ibClient->configuration().marketCloses().c_str(), "hh:mm:ss");

#ifndef SIMULATED_TRADING
    m_p->m_tradesLogFile.open(
            m_p->m_ibClient->configuration().m_tradesLogBaseFilePath + "/trades_" + timeString() + ".log");
#endif

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::subscribeAllData() {
    m_p->m_orders.clear();
    m_p->m_ibClient->socket()->reqAllOpenOrders();
    m_p->m_ibClient->socket()->reqCompletedOrders(true);
    m_p->m_ibClient->socket()->reqIds(-1);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::unsubscribeAllData() {
    m_p->m_ibClient->socket()->reqGlobalCancel();
    m_p->m_ibCleaner.cleanAll();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onOrderRequest(const ATSOrderRequest &orderRequest) {

#ifndef SIMULATED_TRADING
    auto nowTime = QTime::currentTime();
    auto nowDateTime = QDateTime::currentDateTime();
    auto orderDateTime = QDateTime::fromSecsSinceEpoch(orderRequest.enterTime());
    bool weekend = (QDate::currentDate().day() == 6 || QDate::currentDate().day() == 7);

    //    if(nowTime > m_p->m_closeTime || nowTime < m_p->m_openTime /*|| weekend*/) {
    //        qInfo()<<"Order request outside the trading hours";
    //        return;
    //    }

    if (orderDateTime < nowDateTime.addSecs(-30)) {
        qInfo() << "Order from past cannot be placed";
        return;
    }

    if (m_p->m_nextValidOrderId == -1) {
        qWarning() << "No valid Order Id";
        return;
    }

    Contract contract;
    contract.symbol = orderRequest.symbol();
    contract.currency = "USD";

    if (m_p->m_ibClient->configuration().m_fxTraderEnabled) {
        contract.secType = "CASH";
        contract.exchange = "IDEALPRO";
    } else {
        contract.secType = "CFD";
        contract.exchange = "SMART";
    }

    double position = 0.0;
    double constPosition = 0.0;
    std::int64_t contractId = 0;

    if (m_p->m_accountInfo.positionForSymbol(orderRequest.symbol(), position, contractId)) {
        // Position found - we are in the middle of trading, so keep going on.

        if (position > -1 && position < 1) {


            // Position was already closed by somebody else so we are in the middle of the trading
            // day, but with zero position, so start with max. available position
            if (!m_p->m_ibClient->configuration().m_fxTraderEnabled) {
                double contractFunds = m_p->m_ibClient->configuration().m_buyingPower / static_cast<double>
                (m_p->m_ibClient->configuration().m_contracts.size());

                position = std::floor(contractFunds / orderRequest.enterPrice());
            } else {
                position = 100000;
            }

            constPosition = position;
        } else {
            // Non zero position exists so we have to double the amount to enter the exactly opposite position
            if (m_p->m_ibClient->configuration().m_fxTraderEnabled && (std::abs(position) > 100000)) {
                if (position < 0) {
                    position = -100000;
                } else if (position > 0) {
                    position = 100000;
                }
            }

            constPosition = position;
            position = position * 2;
        }
    } else {
        // Position not found - at the beginning of trading day, so calculate max
        // available position from our buying power

        if (!m_p->m_ibClient->configuration().m_fxTraderEnabled) {
            double contractFunds = m_p->m_ibClient->configuration().m_buyingPower / static_cast<double>
            (m_p->m_ibClient->configuration().m_contracts.size());

            position = std::floor(contractFunds / orderRequest.enterPrice());
        } else {
            position = 100000;
        }
        constPosition = position;

    }

    if (position == 0.0) {
        qWarning() << "Request to trade zero position" << "\n";
        return;
    }

    Order order;
    order.orderType = "MKT";

    if (orderRequest.action() == ATSOrderRequest::Action::Buy) {
        order.action = "BUY";
        constPosition = std::abs(constPosition);
    } else if (orderRequest.action() == ATSOrderRequest::Action::Sell) {
        order.action = "SELL";
        constPosition = -1 * constPosition;
    }

    ATSOrderRequest orderRequestCopy = orderRequest;
    orderRequestCopy.setOrderId(m_p->m_nextValidOrderId);
    orderRequestCopy.setPosition(constPosition);
    m_p->m_orderRequests.push(orderRequestCopy);

    order.totalQuantity = std::abs(position);

    if (!m_p->m_ibClient->configuration().m_tradingHalted) {
        m_p->m_ibClient->socket()->placeOrder(m_p->m_nextValidOrderId, contract, order);
    }

    m_p->m_nextValidOrderId = -1;
    m_p->m_ibClient->socket()->reqIds(-1);
#else

    m_p->m_nextValidOrderId++;

    // Funds available for one Contract

    auto maxQuantity = 0.0;

    if (m_p->m_ibClient->configuration().m_fxTraderEnabled) {
        maxQuantity = 100000; // 1Lot
    } else {
        double contractFunds = m_p->m_ibClient->configuration().m_buyingPower / static_cast<double>
        (m_p->m_ibClient->configuration().m_contracts.size());
        maxQuantity = std::floor(contractFunds / orderRequest.enterPrice());
    }

    ATSIBOrder order;
    order.m_order.orderId = m_p->m_nextValidOrderId;
    order.m_order.permId = static_cast<int>(m_p->m_nextValidOrderId);
    order.m_contract.symbol = orderRequest.symbol();
    order.m_orderStatus.m_avgFillPrice = orderRequest.enterPrice();
    order.m_orderState.commission = m_p->m_ibClient->configuration().m_demoCommission;
    order.m_order.totalQuantity = maxQuantity;
    order.m_position = maxQuantity;

    auto dateTime = QDateTime::fromSecsSinceEpoch(orderRequest.enterTime());
    order.m_orderState.completedTime = dateTime.toString("yyyyMMdd  hh:mm:ss").toStdString();

    if (orderRequest.action() == ATSOrderRequest::Action::Buy) {
        order.m_order.action = "BUY";
    } else {
        order.m_order.action = "SELL";
    }

    emit orderCompleted(order);

#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onExitRequest(double price, std::int64_t time, const std::string &symbol) {
#ifndef SIMULATED_TRADING

    double position = 0.0;
    double constPosition = 0.0;
    std::int64_t contractId = 0;

    if (m_p->m_accountInfo.positionForSymbol(symbol, position, contractId)) {

        // Non zero position exists so we have to double the amount to enter the exactly opposite position
        if (m_p->m_ibClient->configuration().m_fxTraderEnabled && (std::abs(position) > 100000)) {
            if (position < 0) {
                position = -100000;
            } else if (position > 0) {
                position = 100000;
            }
        }

        constPosition = position;
    }

    Order order;
    order.orderType = "MKT";

    if (position > 0) {
        order.action = "SELL";
        constPosition = std::abs(constPosition);
    } else if (position < 0) {
        order.action = "SELL";
        constPosition = -1 * constPosition;
    }

    Contract contract;
    contract.symbol = symbol;
    contract.currency = "USD";

    if (m_p->m_ibClient->configuration().m_fxTraderEnabled) {
        contract.secType = "CASH";
        contract.exchange = "IDEALPRO";
    } else {
        contract.secType = "CFD";
        contract.exchange = "SMART";
    }

    ATSOrderRequest orderRequestCopy;
    orderRequestCopy.setExitTime(time);
    orderRequestCopy.setExitPrice(price);
    orderRequestCopy.setSymbol(symbol);
    orderRequestCopy.setOrderId(m_p->m_nextValidOrderId);
    orderRequestCopy.setPosition(constPosition);
    m_p->m_orderRequests.push(orderRequestCopy);

    order.totalQuantity = std::abs(position);

    if (!m_p->m_ibClient->configuration().m_tradingHalted) {
        m_p->m_ibClient->socket()->placeOrder(m_p->m_nextValidOrderId, contract, order);
    }

    m_p->m_nextValidOrderId = -1;
    m_p->m_ibClient->socket()->reqIds(-1);

#else
    ATSOrderRequest request;
    request.setExitTime(time);
    request.setExitPrice(price);
    request.setSymbol(symbol);
    request.setOrderId(m_p->m_nextValidOrderId);
    emit positionExited(request);

#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onStopTrading(const ATSOrderRequest &orderRequest) {
#ifndef SIMULATED_TRADING
    auto nowTime = QTime::currentTime();
    auto nowDateTime = QDateTime::currentDateTime();
    auto orderDateTime = QDateTime::fromSecsSinceEpoch(orderRequest.enterTime());

    if (nowTime > m_p->m_closeTime || nowTime < m_p->m_openTime) {
        qInfo() << "Order request outside the trading hours";
        return;
    }

    if (orderDateTime < nowDateTime.addSecs(-3)) {
        qInfo() << "Order from past cannot be placed";
        return;
    }

    if (m_p->m_nextValidOrderId == -1) {
        qWarning() << "No valid Order Id";
        return;
    }

    Contract contract;
    contract.symbol = orderRequest.symbol();
    contract.currency = m_p->m_ibClient->configuration().m_baseCurrency;

    if (m_p->m_ibClient->configuration().m_fxTraderEnabled) {
        contract.secType = "CASH";
        contract.exchange = "IDEALPRO";
    } else {
        contract.secType = "CFD";
        contract.exchange = "SMART";
    }

    double position = 0.0;
    std::int64_t contractId = 0;

    if (!m_p->m_accountInfo.positionForSymbol(orderRequest.symbol(), position, contractId)) {
        // Position not found which should not happen
        qWarning() << "Could not find position, nothing to close";
        return;
    }

    Order order;
    order.orderType = "MKT";

    if (position < 0) {
        order.action = "BUY";
    } else {
        order.action = "SELL";
    }

    ATSOrderRequest orderRequestCopy = orderRequest;
    orderRequestCopy.setOrderId(m_p->m_nextValidOrderId);
    orderRequestCopy.setPosition(position);
    m_p->m_orderRequests.push(orderRequestCopy);

    order.totalQuantity = std::abs(position);

    if (!m_p->m_ibClient->configuration().m_tradingHalted) {
        m_p->m_ibClient->socket()->placeOrder(m_p->m_nextValidOrderId, contract, order);
    }

    m_p->m_nextValidOrderId = -1;
    m_p->m_ibClient->socket()->reqIds(-1);

#else
    {
        m_p->m_nextValidOrderId++;

        // Funds available for one Contract
        auto maxQuantity = 0.0;

        if (m_p->m_ibClient->configuration().m_fxTraderEnabled) {
            maxQuantity = 100000; // 1Lot
        } else {

            double contractFunds = m_p->m_ibClient->configuration().m_buyingPower / static_cast<double>
            (m_p->m_ibClient->configuration().m_contracts.size());
            maxQuantity = std::floor(contractFunds / orderRequest.enterPrice());
        }

        ATSIBOrder order;
        order.m_order.orderId = m_p->m_nextValidOrderId;
        order.m_order.permId = static_cast<int>(m_p->m_nextValidOrderId);
        order.m_contract.symbol = orderRequest.symbol();
        order.m_orderStatus.m_avgFillPrice = orderRequest.enterPrice();
        order.m_orderState.commission = m_p->m_ibClient->configuration().m_demoCommission;
        order.m_order.totalQuantity = maxQuantity;
        order.m_position = maxQuantity;

        auto dateTime = QDateTime::fromSecsSinceEpoch(orderRequest.enterTime());
        order.m_orderState.completedTime = dateTime.toString("yyyyMMdd  hh:mm:ss").toStdString();

        if (orderRequest.action() == ATSOrderRequest::Action::Buy) {
            order.m_order.action = "BUY";
        } else if (orderRequest.action() == ATSOrderRequest::Action::Sell) {
            order.m_order.action = "SELL";
        }

        emit orderCompleted(order);
    }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onNextValidOrderId(OrderId orderId) {
    m_p->m_nextValidOrderId = orderId;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onOrderStatus(const ATSIBOrderStatus &status) {
    if (status.m_status == "Filled") {

        ATSOrderRequest orderRequest;
        bool ok = m_p->m_orderRequests.containsRequest(status.m_orderId, orderRequest);

        std::map<std::int64_t, ATSIBOrder>::iterator iter = m_p->m_orders.find(status.m_permId);

        if (iter != m_p->m_orders.end()) {
            iter->second.m_orderStatus = status;

            if (ok) {
                iter->second.m_position = orderRequest.position();
            }

            auto dateTime = QDateTime::currentDateTime();
            iter->second.m_orderState.completedTime = dateTime.toString("yyyyMMdd  hh:mm:ss").toStdString();

#ifndef SIMULATED_TRADING
            m_p->m_tradesLogFile << "OrderStatus - filled, order id: " << std::to_string(status.m_orderId)
                                 << ", completed at: " << dateTime.toString().toStdString()
                                 << std::endl;
#endif
            if (iter->second.m_orderState.commissionCurrency == "CZK") {
                iter->second.m_orderState.commission =
                        iter->second.m_orderState.commission / m_p->m_ibClient->czkToUsdRate();
                iter->second.m_orderState.commissionCurrency = "USD";
            }
            emit orderCompleted(iter->second);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onOpenOrder(const ATSIBOrder &order) {
    m_p->m_orders.insert_or_assign(order.m_order.permId, order);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onOpenOrderEnd() {
    // Nothing to do
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onCompletedOrder(const ATSIBOrder &order) {
    auto iter = m_p->m_orders.find(order.m_order.permId);

    if (iter != m_p->m_orders.end()) {
        // Backup status - could be already set
        ATSIBOrderStatus status = iter->second.m_orderStatus;
        ATSIBOrder newOrder = order;
        newOrder.m_orderStatus = status;

        if (newOrder.m_orderState.commissionCurrency == "CZK") {
            newOrder.m_orderState.commission =
                    iter->second.m_orderState.commission / m_p->m_ibClient->czkToUsdRate();
            newOrder.m_orderState.commissionCurrency = "USD";
        }
        emit orderCompleted(newOrder);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onCompletedOrderEnd() {
    // Nothing to do
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onAccountUpdated(const ATSIBAccountInfo &accountInfo) {
    m_p->m_accountInfo = accountInfo;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onAccountPositionReceived(const ATSIBPosition &position) {
    m_p->m_accountInfo.m_portfolio.m_positions.insert_or_assign(position.m_contract.conId, position);

#ifndef SIMULATED_TRADING
    m_p->m_tradesLogFile << position.toString() << ", received at: "
                         << QDateTime::currentDateTime().toString().toStdString() << std::endl;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::onAccountPositionBriefReceived(const ATSIBPositionBrief &position) {
    m_p->m_accountInfo.updatePositionForSymbol(position.m_contract.symbol, position.m_position,
                                               position.m_contract.conId);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::buy() {
    //    Contract contract;
    //    contract.symbol = "EUR";
    //    contract.currency = "USD";
    //    contract.secType = "CASH";
    //    contract.exchange = "IDEALPRO";

    //    Order order;
    //    order.orderType = "TRAIL";



    //    ATSOrderRequest orderRequestCopy = orderRequest;
    //    orderRequestCopy.setOrderId(m_nextValidOrderId);
    //    orderRequestCopy.setPosition(positionWithSign);
    //    m_orderRequests.push(orderRequestCopy);

    //    order.totalQuantity = std::abs(position);

    //    m_ibClient->socket()->placeOrder(m_nextValidOrderId, contract, order);

    //    m_nextValidOrderId = -1;
    //    m_ibClient->socket()->reqIds(-1);

    QMessageBox::question(nullptr, "Order", "Implement me!", QMessageBox::Ok);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::sell() {
    QMessageBox::question(nullptr, "Order", "Implement me!", QMessageBox::Ok);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBOrders::invert() {
    QMessageBox::question(nullptr, "Order", "Implement me!", QMessageBox::Ok);
}
}
