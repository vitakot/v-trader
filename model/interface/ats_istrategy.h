#ifndef ATS_ISTRATEGY_H
#define ATS_ISTRATEGY_H

#include <vector>
#include <cinttypes>

struct Contract;

namespace trader {

struct ATSSignalPack;

class ATSOrderRequest;

class ATSProcessingCluster;

struct ATSIStrategy {
    virtual ~ATSIStrategy() = default;

    virtual void computePositions(const ATSSignalPack &signalPack) = 0;

    virtual void setContract(const Contract &contract) = 0;

    virtual void setCluster(ATSProcessingCluster *cluster) = 0;

    [[nodiscard]] virtual std::vector<ATSOrderRequest> orderRequests() const = 0;

    virtual void loadActiveOrderRequest(const ATSOrderRequest &orderRequest) = 0;

    virtual void updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) = 0;

    virtual void setEnabled(bool enabled) = 0;
};
}

#endif // ATS_ISTRATEGY_H
