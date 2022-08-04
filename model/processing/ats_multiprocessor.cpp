#include "ats_multiprocessor.h"
#include <QDateTime>
#include "model/ats_order_request.h"
#include "model/ats_utils.h"
#include <QDebug>
#include "ats_processing_cluster.h"
#include <fstream>
#include "thirdparty/IB/client/Contract.h"
#include "ats_processor.h"

namespace trader {

struct BarValues {
    std::vector<double> m_open;
    std::vector<double> m_high;
    std::vector<double> m_low;
    std::vector<double> m_close;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSMultiProcessor::P {
    ATSMultiProcessor *m_parent = nullptr;
    ATSProcessingCluster *m_cluster = nullptr;
    Contract m_contract;
    std::map<BarResolution, BarValues> m_aggregatedBars;
    std::map<BarResolution, std::shared_ptr<ATSIProcessor>> m_processors;

    P(ATSMultiProcessor *parent, ATSProcessingCluster *cluster) {
        m_parent = parent;
        m_cluster = cluster;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSMultiProcessor::ATSMultiProcessor(ATSProcessingCluster *cluster) : m_p(
        spimpl::make_unique_impl<P>(this, cluster)) {

}

//----------------------------------------------------------------------------------------------------------------------
bool ATSMultiProcessor::init(const ATSConfiguration &configuration, const std::string &symbol) {
    for (auto &contract : configuration.m_ibConfiguration.m_contracts) {
        if (contract.symbol == symbol) {
            m_p->m_contract = contract;
            break;
        }
    }

    for (const auto &resolution : configuration.m_ibConfiguration.m_activeResolutions) {
        auto processor = std::make_shared<ATSProcessor>(m_p->m_cluster);
        processor->init(configuration, symbol);
        m_p->m_processors.insert_or_assign(resolution, processor);
    }


    m_p->m_aggregatedBars.clear();

    for (const auto &resolution : configuration.m_ibConfiguration.m_activeResolutions) {
        m_p->m_aggregatedBars.insert_or_assign(resolution, BarValues());
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
Contract ATSMultiProcessor::contract() const {
    return m_p->m_contract;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest>
ATSMultiProcessor::orderRequestsForResolution(const BarResolution &barResolution) const {
    auto it = m_p->m_processors.find(barResolution);
    if (it != m_p->m_processors.end()) {
        return it->second->orderRequests();
    }

    return std::vector<ATSOrderRequest>();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMultiProcessor::loadOrderRequestsForResolution(const BarResolution &barResolution,
                                                       const ATSOrderRequest &orderRequest) {
    auto it = m_p->m_processors.find(barResolution);

    if (it != m_p->m_processors.end()) {
        it->second->loadActiveOrderRequest(orderRequest);
    }
}

//----------------------------------------------------------------------------------------------------------------------
ATSSignals ATSMultiProcessor::process(const ATSBar &bar) {
    ATSSignals retVal;

    std::vector<std::pair<std::int64_t, double>> atr;

    for (auto &abar : m_p->m_aggregatedBars) {
        abar.second.m_open.push_back(bar.m_open);
        abar.second.m_high.push_back(bar.m_high);
        abar.second.m_low.push_back(bar.m_low);
        abar.second.m_close.push_back(bar.m_close);
    }

    for (auto &resolution : m_p->m_aggregatedBars) {

        auto numBars = static_cast<std::size_t>(realTimeBarsForResolution(resolution.first));

        if (resolution.second.m_close.size() == numBars) {

            std::vector<double> m_openA;
            std::vector<double> m_highA;
            std::vector<double> m_lowA;
            std::vector<double> m_closeA;

            if (groupCandles(resolution.second.m_high, resolution.second.m_open, resolution.second.m_close,
                             resolution.second.m_low, numBars, m_highA, m_openA,
                             m_closeA, m_lowA)) {

                ATSBar aggregatedBar;

                if (m_openA.size() != 1) {
                    return retVal;
                }

                aggregatedBar.m_open = resolution.second.m_open[0];
                aggregatedBar.m_high = resolution.second.m_high[0];
                aggregatedBar.m_low = resolution.second.m_low[0];
                aggregatedBar.m_close = resolution.second.m_close[0];
                aggregatedBar.m_unixTime = bar.m_unixTime;

                resolution.second.m_open.clear();
                resolution.second.m_high.clear();
                resolution.second.m_low.clear();
                resolution.second.m_close.clear();

                auto procIter = m_p->m_processors.find(resolution.first);

                if (procIter != m_p->m_processors.end()) {
                    auto pack = procIter->second->process(aggregatedBar);
                    retVal.m_signalPacks.insert_or_assign(resolution.first, pack);
                }
            }

        }
    }

    return retVal;
}
}
