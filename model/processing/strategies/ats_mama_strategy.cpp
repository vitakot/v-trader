#include "ats_mama_strategy.hpp"
#include "model/interface/ats_iprocessor.h"
#include "model/processing/ats_processing_cluster.h"

namespace trader {

struct ATSMamaStrategy::P {
    ATSSignalPack m_previousOutput;
    std::vector<ATSOrderRequest> m_orderRequests;
    Contract m_contract;
    bool m_firstRun = true;
    ATSProcessingCluster *m_cluster = nullptr;
    bool m_enabled = false;
    std::list<double> m_mamaPrevious;
    std::list<double> m_famaPrevious;

    void updateMamaPrevious(const ATSSignalPack &signalPack);

    bool checkIfMA200AbovePrice(const ATSSignalPack &signalPack);
};

//----------------------------------------------------------------------------------------------------------------------
ATSMamaStrategy::ATSMamaStrategy() : m_p(spimpl::make_unique_impl<P>()) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::setContract(const Contract &contract) {
    m_p->m_contract = contract;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::setCluster(ATSProcessingCluster *cluster) {
    m_p->m_cluster = cluster;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest> ATSMamaStrategy::orderRequests() const {
    return m_p->m_orderRequests;
}

//---------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::setEnabled(bool enabled) {
    if (enabled != m_p->m_enabled) {
        m_p->m_enabled = enabled;

        if (!m_p->m_enabled && !m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
            m_p->m_orderRequests.back().closeOrder();

            if (m_p->m_cluster) {
                // emit m_p->m_cluster->stopTrading(m_p->m_orderRequests.back());
                emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
                                                 m_p->m_orderRequests.back().exitTime(), m_p->m_contract.symbol);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::loadActiveOrderRequest(const ATSOrderRequest &orderRequest) {
    m_p->m_orderRequests.clear();
    m_p->m_orderRequests.push_back(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) {
    if (m_p->m_orderRequests.size() > 0) {
        m_p->m_orderRequests.back().setExitTime(unixTime);
        m_p->m_orderRequests.back().setExitPrice(exitPrice);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::P::updateMamaPrevious(const ATSSignalPack &signalPack) {
//    m_mamaPrevious.push_back(signalPack.m_mama[0]);
//    m_famaPrevious.push_back(signalPack.m_mama[1]);
//
//    if (m_famaPrevious.size() > 3) {
//        m_famaPrevious.erase(m_famaPrevious.begin());
//        m_mamaPrevious.erase(m_mamaPrevious.begin());
//    }
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSMamaStrategy::P::checkIfMA200AbovePrice(const ATSSignalPack &signalPack) {
   // return signalPack.m_ma[1] > signalPack.m_ma[0];
   return false;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMamaStrategy::computePositions(const ATSSignalPack &signalPack) {
//    ATSSignalPack current;
//    current = signalPack;
//
//    m_p->updateMamaPrevious(signalPack);
//
//    if (m_p->m_firstRun) {
//        m_p->m_previousOutput = current;
//        m_p->m_firstRun = false;
//        return;
//    }
//
//    if (!m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
//        m_p->m_orderRequests.back().setExitPrice(current.m_bar.m_close);
//        m_p->m_orderRequests.back().setExitTime(current.m_bar.m_unixTime);
//
//        auto profit = m_p->m_orderRequests.back().profitRelative() * 100000;
//
//        if (m_p->m_cluster) {
//            emit m_p->m_cluster->currentProfit(m_p->m_contract.symbol, profit);
//        }
//    }
//
//    if (!m_p->m_enabled) {
//        return;
//    }
//
//    std::vector<double> mama;
//    std::vector<double> fama;
//    mama.insert(mama.begin(), m_p->m_mamaPrevious.begin(), m_p->m_mamaPrevious.end());
//    fama.insert(fama.begin(), m_p->m_famaPrevious.begin(), m_p->m_famaPrevious.end());
//
//    bool longSignal = crossover(mama, fama);
//    bool shortSignal = crossover(fama, mama);
//
//    if (longSignal) {
//        if (!m_p->m_orderRequests.empty()) {
//
//            auto lastOrder = m_p->m_orderRequests.back();
//
//            if (lastOrder.action() == ATSOrderRequest::Action::Buy && lastOrder.isOpen()) {
//                return;
//            } else {
//                emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
//                                                 m_p->m_orderRequests.back().exitTime(),
//                                                 m_p->m_orderRequests.back().symbol());
//            }
//        }
//
//        ATSOrderRequest newOrder(ATSOrderRequest::Action::Buy, m_p->m_contract.symbol, signalPack.m_bar.m_close,
//                                 current.m_bar.m_unixTime);
//        m_p->m_orderRequests.push_back(newOrder);
//
//        if (m_p->m_cluster) {
//            emit m_p->m_cluster->orderRequest(newOrder);
//        }
//    } else if (shortSignal) {
//        if (!m_p->m_orderRequests.empty()) {
//
//            auto lastOrder = m_p->m_orderRequests.back();
//
//            if (lastOrder.action() == ATSOrderRequest::Action::Sell && lastOrder.isOpen()) {
//                return;
//            } else {
//                emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
//                                                 m_p->m_orderRequests.back().exitTime(),
//                                                 m_p->m_orderRequests.back().symbol());
//            }
//        }
//
//        ATSOrderRequest newOrder(ATSOrderRequest::Action::Sell, m_p->m_contract.symbol, signalPack.m_bar.m_close,
//                                 current.m_bar.m_unixTime);
//        m_p->m_orderRequests.push_back(newOrder);
//
//        if (m_p->m_cluster) {
//            emit m_p->m_cluster->orderRequest(newOrder);
//        }
//    }

 //   m_p->m_previousOutput = current;
}
}
