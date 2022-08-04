#ifndef ATS_PROCESSOR_H
#define ATS_PROCESSOR_H

#include "thirdparty/IB/client/CommonDefs.h"
#include <map>
#include "thirdparty/spimpl.hpp"
#include "model/ats_configuration.h"
#include "model/interface/ats_iprocessor.h"

namespace trader {

class ATSProcessingCluster;

class ATSProcessor final : public ATSIProcessor {

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSProcessor(ATSProcessingCluster *cluster);

    ~ATSProcessor() override = default;

    bool init(const ATSConfiguration &configuration, const std::string &symbol) override;

    ATSSignalPack process(const ATSBar &bar) override;

    [[nodiscard]] Contract contract() const override;

    [[nodiscard]] std::vector<ATSOrderRequest> orderRequests() const override;

    void loadActiveOrderRequest(const ATSOrderRequest &orderRequest) override;
};
}
#endif // ATS_PROCESSOR_H
