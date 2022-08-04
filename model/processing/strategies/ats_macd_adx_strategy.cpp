#include "ats_macd_adx_strategy.h"
#include "model/interface/ats_iprocessor.h"
#include "model/processing/ats_processing_cluster.h"

namespace trader {

struct ATSMacdADXStrategy::P {
    ATSSignalPack m_previousOutput;
    std::vector<ATSOrderRequest> m_orderRequests;
    Contract m_contract;
    bool m_firstRun = true;
    ATSProcessingCluster *m_cluster = nullptr;
    bool m_enabled = false;
    std::vector<double> m_macdHistory;
    std::vector<double> m_adxHistory;
    bool m_macdIsLong = false;
    bool m_macdIsShort = false;
    std::size_t m_numIncreasingAdxInRow = 3;
    std::list<double> m_shortTermAverage;

    bool isAdxInTrend();

    bool checkIfMA200AbovePrice(const ATSSignalPack &signalPack);
};

//----------------------------------------------------------------------------------------------------------------------
ATSMacdADXStrategy::ATSMacdADXStrategy() : m_p(spimpl::make_unique_impl<P>()) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdADXStrategy::setContract(const Contract &contract) {
    m_p->m_contract = contract;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdADXStrategy::setCluster(ATSProcessingCluster *cluster) {
    m_p->m_cluster = cluster;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest> ATSMacdADXStrategy::orderRequests() const {
    return m_p->m_orderRequests;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdADXStrategy::setEnabled(bool enabled) {
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
void ATSMacdADXStrategy::loadActiveOrderRequest(const ATSOrderRequest &orderRequest) {
    m_p->m_orderRequests.clear();
    m_p->m_orderRequests.push_back(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdADXStrategy::updatePreviousOrderRequest(std::int64_t unixTime, double exitPrice) {
    if (m_p->m_orderRequests.size() > 0) {
        m_p->m_orderRequests.back().setExitTime(unixTime);
        m_p->m_orderRequests.back().setExitPrice(exitPrice);
    }
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSMacdADXStrategy::P::isAdxInTrend() {
    //    if(m_adxHistory.size() >= m_numIncreasingAdxInRow) {

    //        double previousSample = m_adxHistory[m_adxHistory.size() - m_numIncreasingAdxInRow];

    //        for(std::size_t i = m_adxHistory.size() - m_numIncreasingAdxInRow + 1; i < m_adxHistory.size(); i++) {
    //            if(m_adxHistory[i] <= previousSample) {
    //                return false;
    //            }
    //            previousSample = m_adxHistory[i];
    //        }

    //        if(m_adxHistory.back() > 17) {
    //            return true;
    //        }
    //    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSMacdADXStrategy::P::checkIfMA200AbovePrice(const ATSSignalPack &signalPack) {
   // return signalPack.m_ma[2] > signalPack.m_ma[0];
   return false;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSMacdADXStrategy::computePositions(const ATSSignalPack &signalPack) {
    ATSSignalPack current;
    current = signalPack;

   // m_p->m_adxHistory.push_back(signalPack.m_adx[0]);

    if (m_p->m_firstRun) {
        m_p->m_previousOutput = current;
        m_p->m_firstRun = false;
        return;
    }

    if (!m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
        m_p->m_orderRequests.back().setExitPrice(current.m_bar.m_close);
        m_p->m_orderRequests.back().setExitTime(current.m_bar.m_unixTime);

        auto profit = m_p->m_orderRequests.back().profitRelative() * 10000;

        if (m_p->m_cluster) {
            emit m_p->m_cluster->currentProfit(m_p->m_contract.symbol, profit);
        }
    }

    if (!m_p->m_enabled) {
        return;
    }

    double xi;
    double yi;
//
//    if (lineLineIntersectingInRange(m_p->m_previousOutput.m_bar.m_unixTime, m_p->m_previousOutput.m_macd[0],
//                                    current.m_bar.m_unixTime, current.m_macd[0],
//                                    m_p->m_previousOutput.m_bar.m_unixTime, m_p->m_previousOutput.m_macd[1],
//                                    current.m_bar.m_unixTime, current.m_macd[1], xi, yi)) {
//
//        // MACD and MACD Signal crossed and MACD Histogram is above zero - long signal
//        if (current.m_macd[2] > 0) {
//            m_p->m_macdIsShort = false;
//            m_p->m_macdIsLong = true;
//        }
//
//            // MACD and MACD Signal crossed and MACD Histogram is below zero - short signal
//        else if (current.m_macd[2] < 0) {
//
//            m_p->m_macdIsShort = true;
//            m_p->m_macdIsLong = false;
//        }
//    }

    if (m_p->m_macdIsLong && m_p->isAdxInTrend() && m_p->checkIfMA200AbovePrice(signalPack)) {
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
    } else if (m_p->m_macdIsShort && m_p->isAdxInTrend() && !m_p->checkIfMA200AbovePrice(signalPack)) {
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

//    if (current.m_bar.m_close < current.m_ma[1] && m_p->m_macdIsLong) {
//        m_p->m_macdIsLong = false;
//        if (!m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
//            if (m_p->m_cluster) {
//                emit m_p->m_cluster->exitRequest(current.m_bar.m_close, current.m_bar.m_unixTime, m_p->m_contract.symbol);
//            }
//            m_p->m_orderRequests.back().closeOrder();
//        }
//    } else if (current.m_bar.m_close > current.m_ma[1] && m_p->m_macdIsShort) {
//        m_p->m_macdIsShort = false;
//        if (!m_p->m_orderRequests.empty() && m_p->m_orderRequests.back().isOpen()) {
//            if (m_p->m_cluster) {
//                emit m_p->m_cluster->exitRequest(current.m_bar.m_close, current.m_bar.m_unixTime, m_p->m_contract.symbol);
//            }
//            m_p->m_orderRequests.back().closeOrder();
//        }
//    }

    m_p->m_previousOutput = current;
}
}
