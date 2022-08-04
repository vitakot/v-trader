#include "ats_order_request.h"
#include "ats_utils.h"

namespace trader {

//----------------------------------------------------------------------------------------------------------------------
ATSOrderRequest::ATSOrderRequest(Action action, const std::string &symbol, double enterPrice,
                                 std::int64_t enterTime) {
    m_action = action;
    m_enterPrice = enterPrice;
    m_exitPrice = enterPrice;
    m_enterTime = enterTime;
    m_symbol = symbol;

#ifdef SL_TP
    m_lastHigh = m_enterPrice;

    if(m_action == Action::Buy) {
        m_stopLossValue = enterPrice - (enterPrice/100.0) * m_trailingAmount;
    }
    else {
        m_stopLossValue = enterPrice + (enterPrice/100.0) * m_trailingAmount;
    }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
ATSOrderRequest::Action ATSOrderRequest::action() const {
    return m_action;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setAction(const Action &action) {
    m_action = action;
}

//----------------------------------------------------------------------------------------------------------------------
double ATSOrderRequest::enterPrice() const {
    return m_enterPrice;
}

//----------------------------------------------------------------------------------------------------------------------
double ATSOrderRequest::profitRelative() const {
    if (m_action == Action::Buy) {
        return ((m_exitPrice / m_enterPrice)) - 1;
    } else {
        return ((m_enterPrice / m_exitPrice)) - 1;
    }
}

//----------------------------------------------------------------------------------------------------------------------
std::int64_t ATSOrderRequest::enterTime() const {
    return m_enterTime;
}

//----------------------------------------------------------------------------------------------------------------------
std::string ATSOrderRequest::symbol() const {
    return m_symbol;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setSymbol(const std::string &symbol) {
    m_symbol = symbol;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setExitTime(std::int64_t exitTime) {
    m_exitTime = exitTime;
}

//----------------------------------------------------------------------------------------------------------------------
std::int64_t ATSOrderRequest::exitTime() const {
    return m_exitTime;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setExitPrice(double exitPrice) {
    m_exitPrice = exitPrice;

#ifdef SL_TP

    if(m_stopLossActivated) {
        return;
    }

    if(m_action == Action::Buy) {

        // price is going up, so shift stop loss up
        if(exitPrice > m_lastHigh) {
            m_stopLossValue = exitPrice - (exitPrice/100.0) * m_trailingAmount;
            m_lastHigh = exitPrice;
        }
        else if(exitPrice < m_stopLossValue) {
            m_isOpen = false;
            m_stopLossActivated = true;
        }
    }
    else {
        // price is going down, so shift stop loss up
        if(exitPrice < m_lastHigh) {
            m_stopLossValue = exitPrice + (exitPrice/100.0) * m_trailingAmount;
            m_lastHigh = exitPrice;
        }
        else if(exitPrice > m_stopLossValue) {
            m_isOpen = false;
            m_stopLossActivated = true;
        }
    }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
double ATSOrderRequest::exitPrice() const {
    return m_exitPrice;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setOrderId(std::int64_t orderId) {
    m_orderId = orderId;
}

//----------------------------------------------------------------------------------------------------------------------
std::int64_t ATSOrderRequest::orderId() const {
    return m_orderId;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setFilled(double filled) {
    m_filled = filled;
}

//----------------------------------------------------------------------------------------------------------------------
double ATSOrderRequest::filled() const {
    return m_filled;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::setPosition(double position) {
    m_position = position;
}

//----------------------------------------------------------------------------------------------------------------------
double ATSOrderRequest::position() const {
    return m_position;
}

//----------------------------------------------------------------------------------------------------------------------
std::string ATSOrderRequest::toString() const {
    std::string retVal;
    std::string action;

    switch (m_action) {
        case Action::Buy:
            action = "buy";
            break;
        case Action::Sell:
            action = "sell";
            break;
    }

    retVal = "ATSOrderRequest, action: " + action + ", symbol: " + m_symbol + ", order id: " +
             std::to_string(m_orderId) + ", enter price: " +
             std::to_string(m_enterPrice) + ", enter time: " + std::to_string(
            m_enterTime) + ", exit price: " + std::to_string(m_exitPrice) + ", exit time: " +
             std::to_string(m_exitTime);

    return retVal;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSOrderRequest::loadFromJson(const nlohmann::json &jsonObject) {
    try {
        readValue<std::string>(jsonObject, "Symbol", m_symbol);
        readValue<Action>(jsonObject, "Action", m_action);
        readValue<double>(jsonObject, "EnterPrice", m_enterPrice);
        readValue<std::int64_t>(jsonObject, "EnterTime", m_enterTime);
        readValue<double>(jsonObject, "ExitPrice", m_exitPrice);
        readValue<std::int64_t>(jsonObject, "OrderId", m_orderId);
    }
    catch (nlohmann::json::exception &) {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSOrderRequest::saveToJson(nlohmann::json &jsonObject) const {
    try {
        jsonObject["Symbol"] = m_symbol;
        jsonObject["Action"] = m_action;
        jsonObject["EnterPrice"] = m_enterPrice;
        jsonObject["EnterTime"] = m_enterTime;
        jsonObject["ExitPrice"] = m_exitPrice;
        jsonObject["OrderId"] = m_orderId;
    }
    catch (nlohmann::json::exception &) {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSOrderRequest::isOpen() const {
    return m_isOpen;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOrderRequest::closeOrder() {
    m_isOpen = false;
}

}
