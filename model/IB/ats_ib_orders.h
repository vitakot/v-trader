#ifndef ATSIBORDERS_H
#define ATSIBORDERS_H

#include <QObject>
#include "thirdparty/spimpl.hpp"
#include "model/ats_order_request.h"
#include "thirdparty/IB/client/Order.h"
#include "thirdparty/IB/client/OrderState.h"
#include "thirdparty/IB/client/Contract.h"
#include "thirdparty/IB/client/CommonDefs.h"
#include "model/ats_utils.h"
#include "ats_ib_account.h"

namespace trader {

//---------------------------------------------------------------------------------------------------------------------
struct ATSIBOrderStatus {
    OrderId m_orderId = -1;
    std::string m_status;
    double m_filled = 0;
    double m_remaining = 0;
    double m_avgFillPrice = 0;
    int m_permId = -1;
    int m_parentId = -1;
    double m_lastFillPrice = 0;
    int m_clientId = -1;
    std::string m_whyHeld;
    double m_mktCapPrice = 0;

    std::string toJsonString() const;

    nlohmann::json toJson() const;

    void fromJson(const nlohmann::json &json);
};

//---------------------------------------------------------------------------------------------------------------------
struct ATSIBOrder {
    double m_position = 0.0;
    double m_exitPrice = 0.0;
    Order m_order;
    Contract m_contract;
    OrderState m_orderState;
    ATSIBOrderStatus m_orderStatus;

    std::string toJsonString() const;

    nlohmann::json toJson() const;

    void fromJson(const nlohmann::json &json);
};

//---------------------------------------------------------------------------------------------------------------------
class ATSIBOrders final : public QObject {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    ATSIBOrders(QObject *parent);

    ~ATSIBOrders() = default;

    void init();

    void subscribeAllData();

    void unsubscribeAllData();

public slots:

    void onOrderRequest(const ATSOrderRequest &orderRequest);

    void onStopTrading(const ATSOrderRequest &orderRequest);

    void onExitRequest(double price, std::int64_t time, const std::string &symbol);

    void onOpenOrder(const ATSIBOrder &order);

    void onOpenOrderEnd();

    void onCompletedOrder(const ATSIBOrder &order);

    void onCompletedOrderEnd();

    void onNextValidOrderId(OrderId orderId);

    void onOrderStatus(const ATSIBOrderStatus &status);

    void onAccountUpdated(const ATSIBAccountInfo &accountInfo);

    void onAccountPositionReceived(const ATSIBPosition &position);

    void onAccountPositionBriefReceived(const ATSIBPositionBrief &position);

    void buy();

    void sell();

    void invert();

signals:

    void orderCompleted(const ATSIBOrder &order);

    void positionExited(const ATSOrderRequest &orderRequest);
};
}
#endif // ATSIBORDERS_H
