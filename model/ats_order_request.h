#ifndef ATS_ORDER_REQUEST_H
#define ATS_ORDER_REQUEST_H

#include <cstdint>
#include <string>
#include "thirdparty/nlohmann/nlohmann_fwd.hpp"

namespace trader {

class ATSOrderRequest {

public:

    enum class Action : int {
        Buy,
        Sell
    };

private:
    std::string m_symbol;
    Action m_action = Action::Buy;
    double m_enterPrice = 0;
    double m_exitPrice = 0;
    std::int64_t m_orderId = -1;
    std::int64_t m_enterTime = 0;
    std::int64_t m_exitTime = 0;
    double m_filled = 0.0;
    double m_position = 0.0;
    bool m_isOpen = true;

#ifdef SL_TP
    double m_trailingAmount = 0.300; // in percents
    double m_stopLossValue = 0.0;
    bool m_stopLossActivated =  false;
    double m_lastHigh = 0.0;
#endif

public:

    ATSOrderRequest() = default;

    ATSOrderRequest(Action action, const std::string &symbol, double enterPrice, std::int64_t enterTime);

    [[nodiscard]] Action action() const;

    void setAction(const Action &action);

    [[nodiscard]] double enterPrice() const;

    [[nodiscard]] double profitRelative() const;

    [[nodiscard]] std::int64_t enterTime() const;

    [[nodiscard]] std::string symbol() const;

    void setSymbol(const std::string &symbol);

    void setExitTime(std::int64_t exitTime);

    [[nodiscard]] std::int64_t exitTime() const;

    void setExitPrice(double exitPrice);

    [[nodiscard]] double exitPrice() const;

    void setOrderId(std::int64_t orderId);

    [[nodiscard]] std::int64_t orderId() const;

    void setFilled(double filled);

    [[nodiscard]] double filled() const;

    void setPosition(double position);

    [[nodiscard]] double position() const;

    [[nodiscard]] std::string toString() const;

    bool loadFromJson(const nlohmann::json &jsonObject);

    bool saveToJson(nlohmann::json &jsonObject) const;

    bool isOpen() const;

    void closeOrder();
};
}
#endif // ATS_ORDER_REQUEST_H
