#ifndef ATSMACDADXSTRATEGY_H
#define ATSMACDADXSTRATEGY_H

#include "model/interface/ats_istrategy.h"
#include "thirdparty/spimpl.hpp"

namespace trader {

class ATSMacdADXStrategy final : public ATSIStrategy {

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:

    ATSMacdADXStrategy();

    void computePositions(const ATSSignalPack &signalPack) override;

    void setContract(const Contract &contract) override;

    void setCluster(ATSProcessingCluster *cluster) override;

    [[nodiscard]] std::vector<ATSOrderRequest> orderRequests() const override;

    void loadActiveOrderRequest(const ATSOrderRequest &orderRequest) override;

    void updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) override;

    void setEnabled(bool enabled) override;
};

}
#endif // ATSMACDADXSTRATEGY_H
