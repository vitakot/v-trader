#ifndef ATS_IB_ACCOUNT_H
#define ATS_IB_ACCOUNT_H

#include <thirdparty/spimpl.hpp>
#include <QObject>
#include "model/ats_utils.h"
#include "thirdparty/IB/client/Contract.h"

namespace trader {
class ATSIBOrder;

//----------------------------------------------------------------------------------------------------------------------
struct ATSIBPosition {
    std::string m_account;
    Contract m_contract;
    double m_position = 0.0;
    double m_averageCost = 0.0;
    double m_marketPrice = 0.0;
    double m_marketValue = 0.0;
    double m_unrealizedPNL = 0.0;
    double m_realizedPNL = 0.0;

    [[nodiscard]] std::string toJsonString() const;

    [[nodiscard]] nlohmann::json toJson() const;

    [[nodiscard]] std::string toString() const;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSIBPositionBrief {
    std::string m_account;
    Contract m_contract;
    double m_position = 0.0;
    double m_averageCost = 0.0;

    [[nodiscard]] std::string toJsonString() const;

    [[nodiscard]] nlohmann::json toJson() const;

    [[nodiscard]] std::string toString() const;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSIBPortfolio {

    std::string m_account;
    std::map<std::int64_t, ATSIBPosition> m_positions;

    [[nodiscard]] std::string toJsonString() const;

    [[nodiscard]] nlohmann::json toJson() const;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSIBAccountInfo {
    double m_buyingPower = 0.0;
    double m_leverage = 0.0;
    std::string m_account;
    std::string m_currency;
    double m_availableFunds = 0.0;
    double m_netLiquidation = 0.0;
    double m_totalCashBalance = 0.0;
    double m_cashBalance = 0.0;
    double m_initMarginReq = 0.0;
    double m_maintMarginReq = 0.0;
    double m_cushion = 0.0;
    double m_accruedCash = 0.0;
    double m_settledCash = 0.0;
    double m_unrealizedPNL = 0.0;
    double m_realizedPNL = 0.0;

    ATSIBPortfolio m_portfolio;

    [[nodiscard]] std::string toJsonString() const;

    bool positionForSymbol(const std::string &symbol, double &position, std::int64_t &contractId);

    void updatePositionForSymbol(const std::string &symbol, double position, std::int64_t contractId);

    std::vector<std::int64_t> contractsId();
};

class ATSIBAccount final : public QObject {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSIBAccount(QObject *parent);

    ~ATSIBAccount() override;

    void init();

    [[nodiscard]] ATSIBAccountInfo accountInfo() const;

    void subscribeAllData();

    void unsubscribeAllData();

public slots:

    void
    onAccountInfo(TickerId tickerId, const std::string &account, const std::string &curency, const std::string &tag,
                  const std::string &value);

    void onAccountInfoEnd(TickerId tickerId);

    void onAccountPositionReceived(const ATSIBPosition &position);

    void onAccountPositionBriefReceived(const ATSIBPositionBrief &position);

    void onOrderCompleted(const ATSIBOrder &order);

signals:

    void accountInfoUpdated(const ATSIBAccountInfo &accountInfo);

    void accountPositionUpdated(const ATSIBPosition &portfolio);
};
}
#endif // ATS_IB_ACCOUNT_H
