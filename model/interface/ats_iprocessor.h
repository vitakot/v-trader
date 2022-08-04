#ifndef ATS_IPROCESSOR_H
#define ATS_IPROCESSOR_H

#include <string>
#include <vector>
#include <array>
#include <random>
#include "model/ats_configuration.h"

class Contract;

namespace trader {

class ATSConfiguration;

class ATSOrderRequest;

enum class SignalGroupType : std::int32_t {
    Main = 0,
    Standalone
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSBar {
    std::int64_t m_unixTime = 0;
    double m_open = 0.0;
    double m_high = 0.0;
    double m_low = 0.0;
    double m_close = 0.0;
    std::int32_t m_volume = 0;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSOptimizationResult {
    double m_profit = 0.0;
    std::int32_t m_numTrades = 0;
    std::vector<ATSOrderRequest> m_orderRequests;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSSignalGroup {
    std::map<std::string, std::vector<double>> m_signals;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSSignalPack {
    ATSBar m_bar;
    std::map<SignalGroupType, ATSSignalGroup> m_signalGroups;

    ATSSignalPack(){
        m_signalGroups.insert_or_assign(SignalGroupType::Main,ATSSignalGroup());
        m_signalGroups.insert_or_assign(SignalGroupType::Standalone,ATSSignalGroup());
    }
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSSignals {
    std::map<BarResolution, ATSSignalPack> m_signalPacks;
    std::int64_t m_unixTime = 0;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSIProcessor {

    virtual ~ATSIProcessor() = default;

    virtual bool init(const ATSConfiguration &configuration, const std::string &symbol) = 0;

    virtual ATSSignalPack process(const ATSBar &bar) = 0;

    [[nodiscard]] virtual Contract contract() const = 0;

    [[nodiscard]] virtual std::vector<ATSOrderRequest> orderRequests() const = 0;

    virtual void loadActiveOrderRequest(const ATSOrderRequest &orderRequest) = 0;
};

}
#endif // ATS_IPROCESSOR_H
