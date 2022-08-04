#include "ats_macd_strategy.h"
#include "model/interface/ats_iprocessor.h"
#include "model/processing/ats_processing_cluster.h"

namespace trader {

struct ATSMacdStrategy::P {
    ATSSignalPack m_previousOutput;
    std::vector<ATSOrderRequest> m_orderRequests;
    Contract m_contract;
    bool m_firstRun = true;
    ATSProcessingCluster *m_cluster = nullptr;
    double m_previousMACDDerivative = 0;
    bool m_enabled = false;
    std::vector<double> m_macdHistory;
    bool m_macdIsLong = false;
    bool m_macdIsShort = false;
};

//----------------------------------------------------------------------------------------------------------------------
ATSMacdStrategy::ATSMacdStrategy() : m_p(spimpl::make_unique_impl<P>()) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdStrategy::setContract(const Contract &contract) {
    m_p->m_contract = contract;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdStrategy::setCluster(ATSProcessingCluster *cluster) {
    m_p->m_cluster = cluster;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest> ATSMacdStrategy::orderRequests() const {
    return m_p->m_orderRequests;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdStrategy::setEnabled(bool enabled) {
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

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdStrategy::loadActiveOrderRequest(const ATSOrderRequest &orderRequest) {
    m_p->m_orderRequests.clear();
    m_p->m_orderRequests.push_back(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdStrategy::updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) {
    if (m_p->m_orderRequests.size() > 0) {
        m_p->m_orderRequests.back().setExitTime(unixTime);
        m_p->m_orderRequests.back().setExitPrice(exitPrice);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdStrategy::computePositions(const ATSSignalPack &signalPack) {
    ATSSignalPack current;
    current = signalPack;

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

    double xi;
    double yi;

//    if (lineLineIntersectingInRange(m_p->m_previousOutput.m_bar.m_unixTime, m_p->m_previousOutput.m_macd[0],
//                                    current.m_bar.m_unixTime, current.m_macd[0],
//                                    m_p->m_previousOutput.m_bar.m_unixTime, m_p->m_previousOutput.m_macd[1],
//                                    current.m_bar.m_unixTime, current.m_macd[1], xi, yi)) {
//
//        // MACD and MACD Signal crossed and MACD Histogram is above zero - long signal
//        if (current.m_macd[2] > 0) {
//            m_p->m_macdIsShort = false;
//            m_p->m_macdIsLong = true;
//
//            if (!m_p->m_orderRequests.empty()) {
//
//                auto lastOrder = m_p->m_orderRequests.back();
//
//                if (lastOrder.action() == ATSOrderRequest::Action::Buy && lastOrder.isOpen()) {
//                    return;
//                } else {
//                    emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
//                                                     m_p->m_orderRequests.back().exitTime(),
//                                                     m_p->m_orderRequests.back().symbol());
//                }
//            }
//
//            ATSOrderRequest newOrder(ATSOrderRequest::Action::Buy, m_p->m_contract.symbol, signalPack.m_bar.m_close,
//                                     current.m_bar.m_unixTime);
//            m_p->m_orderRequests.push_back(newOrder);
//
//            if (m_p->m_cluster) {
//                emit m_p->m_cluster->orderRequest(newOrder);
//            }
//        }
//
//            // MACD and MACD Signal crossed and MACD Histogram is below zero - short signal
//        else if (current.m_macd[2] < 0) {
//
//            m_p->m_macdIsShort = true;
//            m_p->m_macdIsLong = false;
//
//            if (!m_p->m_orderRequests.empty()) {
//                auto lastOrder = m_p->m_orderRequests.back();
//
//                if (lastOrder.action() == ATSOrderRequest::Action::Sell && lastOrder.isOpen()) {
//                    return;
//                } else {
//                    emit m_p->m_cluster->exitRequest(m_p->m_orderRequests.back().exitPrice(),
//                                                     m_p->m_orderRequests.back().exitTime(),
//                                                     m_p->m_orderRequests.back().symbol());
//                }
//            }
//
//            ATSOrderRequest newOrder(ATSOrderRequest::Action::Sell, m_p->m_contract.symbol, signalPack.m_bar.m_close,
//                                     current.m_bar.m_unixTime);
//            m_p->m_orderRequests.push_back(newOrder);
//
//            if (m_p->m_cluster) {
//                emit m_p->m_cluster->orderRequest(newOrder);
//            }
//        }
//    }
//
//    // Found inflection point on MACD - close open position
//    if (std::signbit(m_p->m_previousMACDDerivative) !=
//        std::signbit(m_p->m_previousOutput.m_macd[0] - current.m_macd[0])) {
//        m_p->m_macdIsLong = false;
//        m_p->m_macdIsShort = false;
//
//        if (!m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
//            if (m_p->m_cluster) {
//                emit m_p->m_cluster->exitRequest(current.m_bar.m_close, current.m_bar.m_unixTime, m_p->m_contract.symbol);
//            }
//            m_p->m_orderRequests.back().closeOrder();
//        }
//    }
//
//    m_p->m_previousMACDDerivative = m_p->m_previousOutput.m_macd[0] - current.m_macd[0];
    m_p->m_previousOutput = current;
}
}
