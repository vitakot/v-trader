#include "ats_aroon_atr_ma_strategy.h"
#include "model/interface/ats_iprocessor.h"
#include "model/processing/ats_processing_cluster.h"

namespace trader {

struct ATSAroonAtrMaStrategy::P {
    ATSSignalPack m_previousOutput;
    std::vector<ATSOrderRequest> m_orderRequests;
    Contract m_contract;
    bool m_firstRun = true;
    ATSProcessingCluster *m_cluster = nullptr;
    bool m_enabled = false;
    std::list<double> m_aroonPreviousUp;
    std::list<double> m_aroonPreviousDown;

    void updateAroonPrevious(const ATSSignalPack &signalPack);

    bool checkIfMA200AbovePrice(const ATSSignalPack &signalPack);
};

//----------------------------------------------------------------------------------------------------------------------
ATSAroonAtrMaStrategy::ATSAroonAtrMaStrategy() : m_p(spimpl::make_unique_impl<P>()) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSAroonAtrMaStrategy::setContract(const Contract &contract) {
    m_p->m_contract = contract;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSAroonAtrMaStrategy::setCluster(ATSProcessingCluster *cluster) {
    m_p->m_cluster = cluster;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest> ATSAroonAtrMaStrategy::orderRequests() const {
    return m_p->m_orderRequests;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSAroonAtrMaStrategy::setEnabled(bool enabled) {
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
void ATSAroonAtrMaStrategy::loadActiveOrderRequest(const ATSOrderRequest &orderRequest) {
    m_p->m_orderRequests.clear();
    m_p->m_orderRequests.push_back(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSAroonAtrMaStrategy::P::updateAroonPrevious(const ATSSignalPack &signalPack) {
//    m_aroonPreviousDown.push_back(signalPack.m_aroon[0]);
//    m_aroonPreviousUp.push_back(signalPack.m_aroon[1]);
//
//    if (m_aroonPreviousUp.size() > 3) {
//        m_aroonPreviousUp.erase(m_aroonPreviousUp.begin());
//        m_aroonPreviousDown.erase(m_aroonPreviousDown.begin());
//    }
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSAroonAtrMaStrategy::P::checkIfMA200AbovePrice(const ATSSignalPack &signalPack) {
    //return signalPack.m_ma[1] > signalPack.m_ma[0];
    return false;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSAroonAtrMaStrategy::updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) {
    if (m_p->m_orderRequests.size() > 0) {
        m_p->m_orderRequests.back().setExitTime(unixTime);
        m_p->m_orderRequests.back().setExitPrice(exitPrice);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSAroonAtrMaStrategy::computePositions(const ATSSignalPack &signalPack) {
    ATSSignalPack current;
    current = signalPack;

    std::vector<double> currentSamples;
    currentSamples.push_back(current.m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.find(INDICATOR_NAME_AROON)->second[0]);
    currentSamples.push_back(current.m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.find(INDICATOR_NAME_AROON)->second[1]);

    m_p->updateAroonPrevious(signalPack);

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

    std::vector<double> aroonUp;
    std::vector<double> aroonDown;
    aroonUp.insert(aroonUp.begin(), m_p->m_aroonPreviousUp.begin(), m_p->m_aroonPreviousUp.end());
    aroonDown.insert(aroonDown.begin(), m_p->m_aroonPreviousDown.begin(), m_p->m_aroonPreviousDown.end());

    bool longSignal =
            crossover(aroonUp, aroonDown) && m_p->checkIfMA200AbovePrice(signalPack) && currentSamples[1] > 55;
    bool shortSignal =
            crossover(aroonDown, aroonUp) && !m_p->checkIfMA200AbovePrice(signalPack) && currentSamples[0] > 55;

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
    } else if (shortSignal) {
        if (!m_p->m_orderRequests.empty()) {

            auto lastOrder = m_p->m_orderRequests.back();

            if (lastOrder.action() == ATSOrderRequest::Action::Sell && lastOrder.isOpen()) {
                return;
            } else {
                emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
                                                 m_p->m_orderRequests.back().exitTime(),
                                                 m_p->m_orderRequests.back().symbol());
            }
        }

        ATSOrderRequest newOrder(ATSOrderRequest::Action::Sell, m_p->m_contract.symbol, signalPack.m_bar.m_close,
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
//    } else if ((current.m_bar.m_close > current.m_slLines[0]) && !m_p->m_orderRequests.empty()
//               && (m_p->m_orderRequests.back().action() == ATSOrderRequest::Action::Sell)) {
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
