#include "ats_ma_strategy.h"
#include "model/interface/ats_iprocessor.h"
#include "model/processing/ats_processing_cluster.h"

namespace trader {

struct ATSMAStrategy::P {
    ATSSignalPack m_previousOutput;
    std::vector<ATSOrderRequest> m_orderRequests;
    Contract m_contract;
    bool m_firstRun = true;
    ATSProcessingCluster *m_cluster = nullptr;
    bool m_enabled = false;
    std::list<double> m_ma200;
    std::list<double> m_ma50;

    void updateMAPrevious(const ATSSignalPack &signalPack);

    bool checkIfMA200AbovePrice(const ATSSignalPack &signalPack);
};

//----------------------------------------------------------------------------------------------------------------------
ATSMAStrategy::ATSMAStrategy() : m_p(spimpl::make_unique_impl<P>()) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::setContract(const Contract &contract) {
    m_p->m_contract = contract;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::setCluster(ATSProcessingCluster *cluster) {
    m_p->m_cluster = cluster;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest> ATSMAStrategy::orderRequests() const {
    return m_p->m_orderRequests;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::setEnabled(bool enabled) {
    if (enabled != m_p->m_enabled) {
        m_p->m_enabled = enabled;

        if (!m_p->m_enabled && !m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
            m_p->m_orderRequests.back().closeOrder();

            if (m_p->m_cluster) {
                //emit m_p->m_cluster->stopTrading(m_p->m_orderRequests.back());
                emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
                                                 m_p->m_orderRequests.back().exitTime(), m_p->m_contract.symbol);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::loadActiveOrderRequest(const ATSOrderRequest &orderRequest) {
    m_p->m_orderRequests.clear();
    m_p->m_orderRequests.push_back(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) {
    if (m_p->m_orderRequests.size() > 0) {
        m_p->m_orderRequests.back().setExitTime(unixTime);
        m_p->m_orderRequests.back().setExitPrice(exitPrice);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::P::updateMAPrevious(const ATSSignalPack &signalPack) {
//    m_ma200.push_back(signalPack.m_ma[0]);
//    m_ma50.push_back(signalPack.m_ma[1]);
//
//    if (m_ma50.size() > 3) {
//        m_ma50.erase(m_ma50.begin());
//        m_ma200.erase(m_ma200.begin());
//    }
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSMAStrategy::P::checkIfMA200AbovePrice(const ATSSignalPack &signalPack) {
    //return signalPack.m_ma[1] > signalPack.m_ma[0];
    return false;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMAStrategy::computePositions(const ATSSignalPack &signalPack) {
    ATSSignalPack current;
    current = signalPack;

    m_p->updateMAPrevious(signalPack);

    if (m_p->m_firstRun) {
        m_p->m_previousOutput = current;
        m_p->m_firstRun = false;
        return;
    }

    if (!m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
        m_p->m_orderRequests.back().setExitPrice(current.m_bar.m_close);
        m_p->m_orderRequests.back().setExitTime(current.m_bar.m_unixTime);

        auto profit = m_p->m_orderRequests.back().profitRelative() * 100000;

        if (m_p->m_cluster) {
            emit m_p->m_cluster->currentProfit(m_p->m_contract.symbol, profit);
        }
    }

    if (!m_p->m_enabled) {
        return;
    }

    std::vector<double> ma200;
    std::vector<double> ma50;
    ma200.insert(ma200.begin(), m_p->m_ma200.begin(), m_p->m_ma200.end());
    ma50.insert(ma50.begin(), m_p->m_ma50.begin(), m_p->m_ma50.end());

    bool longSignal = crossunder(ma200, ma50);
    bool shortSignal = crossover(ma200, ma50);

    if (longSignal) {
        if (!m_p->m_orderRequests.empty()) {

            auto lastOrder = m_p->m_orderRequests.back();

            if (lastOrder.action() == ATSOrderRequest::Action::Buy && lastOrder.isOpen()) {
                return;
            } else {
                emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
                                                 m_p->m_orderRequests.back().exitTime(),
                                                 m_p->m_orderRequests.back().symbol());
            }
        }

        ATSOrderRequest newOrder(ATSOrderRequest::Action::Buy, m_p->m_contract.symbol, signalPack.m_bar.m_close,
                                 current.m_bar.m_unixTime);
        m_p->m_orderRequests.push_back(newOrder);

        if (m_p->m_cluster) {
            emit m_p->m_cluster->orderRequest(newOrder);
        }
    }

//    if ((current.m_bar.m_close < current.m_slLines[1]) && !m_p->m_orderRequests.empty()
//        && (m_p->m_orderRequests.back().action() == ATSOrderRequest::Action::Buy)) {
//
//        if (m_p->m_orderRequests.back().isOpen()) {
//            if (m_p->m_cluster) {
//                emit m_p->m_cluster->exitRequest(current.m_bar.m_close, current.m_bar.m_unixTime, m_p->m_contract.symbol);
//            }
//            m_p->m_orderRequests.back().closeOrder();
//        }
//    }

    m_p->m_previousOutput = current;
}
}
