#ifndef ATSPROCESSOR_MULTIRES_H
#define ATSPROCESSOR_MULTIRES_H

#include "thirdparty/IB/client/CommonDefs.h"
#include <map>
#include "thirdparty/spimpl.hpp"
#include "model/ats_configuration.h"
#include "model/interface/ats_iprocessor.h"

namespace trader {

class ATSProcessingCluster;

class ATSMultiProcessor {

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    ATSMultiProcessor(ATSProcessingCluster *cluster);

    bool init(const ATSConfiguration &configuration, const std::string &symbol);

    Contract contract() const;

    ATSSignals process(const ATSBar &bar);

    std::vector<ATSOrderRequest> orderRequestsForResolution(const BarResolution &barResolution) const;

    void loadOrderRequestsForResolution(const BarResolution &barResolution, const ATSOrderRequest &orderRequest);
};
}
#endif // ATSPROCESSOR_MULTIRES_H
