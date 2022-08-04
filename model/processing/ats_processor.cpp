#include "ats_processor.h"
#include <QDateTime>
#include "model/ats_order_request.h"
#include <QDebug>
#include "ats_processing_cluster.h"
#include "ta-lib/ta_libc.h"
#include <fstream>
#include <model/processing/strategies/ats_mama_strategy.hpp>
#include "thirdparty/IB/client/Contract.h"
#include "ats_dsp_algorithms.h"
#include "strategies/ats_macd_strategy.h"
#include "strategies/ats_aroon_strategy.h"

namespace trader {

//----------------------------------------------------------------------------------------------------------------------
struct ATSProcessor::P {
    ATSProcessor *m_parent = nullptr;
    ATSProcessingCluster *m_cluster = nullptr;
    QTime m_openTime;
    QTime m_closeTime;
    Contract m_contract;
    std::vector<double> m_open;
    std::vector<double> m_high;
    std::vector<double> m_low;
    std::vector<double> m_close;
    std::vector<std::int64_t> m_unixTime;
    bool m_saveRealTimeData = true;
    std::map<std::string, ATSIndicator> m_indicators;

    bool computeIndicators(std::vector<ATSSignalPack> &signalPacks);

    void computePositions(const ATSSignalPack &signalPack) const;

    void updatePreviousOrder(std::int64_t unixTime, double exitPrice) const;

    std::unique_ptr<ATSIStrategy> m_strategy;

    P(ATSProcessor *parent, ATSProcessingCluster *cluster) {
        m_parent = parent;
        m_cluster = cluster;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSProcessor::ATSProcessor(ATSProcessingCluster *cluster) : m_p(spimpl::make_unique_impl<P>(this, cluster)) {

}

//----------------------------------------------------------------------------------------------------------------------
bool ATSProcessor::init(const ATSConfiguration &configuration, const std::string &symbol) {
    m_p->m_open.clear();
    m_p->m_high.clear();
    m_p->m_low.clear();
    m_p->m_close.clear();
    m_p->m_unixTime.clear();

    for (auto &contract : configuration.m_ibConfiguration.m_contracts) {
        if (contract.symbol == symbol) {
            m_p->m_contract = contract;
            break;
        }
    }

    m_p->m_openTime = QTime::fromString(configuration.m_ibConfiguration.tradingDayStart().c_str(), "hh:mm:ss");
    m_p->m_closeTime = QTime::fromString(configuration.m_ibConfiguration.tradingDayEnd().c_str(), "hh:mm:ss");

    m_p->m_indicators = configuration.m_indicators;

    m_p->m_strategy = std::make_unique<ATSMamaStrategy>();
    m_p->m_strategy->setCluster(m_p->m_cluster);
    m_p->m_strategy->setContract(m_p->m_contract);

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
Contract ATSProcessor::contract() const {
    return m_p->m_contract;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<ATSOrderRequest> ATSProcessor::orderRequests() const {
    return m_p->m_strategy->orderRequests();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSProcessor::loadActiveOrderRequest(const ATSOrderRequest &orderRequest) {
    m_p->m_strategy->loadActiveOrderRequest(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSProcessor::P::updatePreviousOrder(std::int64_t unixTime, double exitPrice) const {
    m_strategy->updatePreviousOrderRequest(unixTime, exitPrice);
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSProcessor::P::computeIndicators(std::vector<ATSSignalPack> &signalPacks) {
    int outBeginIndex = 0;
    int outElementsNumber = 0;

    signalPacks.clear();
    signalPacks.resize(m_close.size());

    for (std::size_t i = 0; i < m_close.size(); i++) {
        signalPacks[i].m_bar.m_open = m_open[i];
        signalPacks[i].m_bar.m_high = m_high[i];
        signalPacks[i].m_bar.m_low = m_low[i];
        signalPacks[i].m_bar.m_close = m_close[i];
        signalPacks[i].m_bar.m_unixTime = m_unixTime[i];
    }

    auto indicator = m_indicators.find(INDICATOR_NAME_ATR);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> atrBufferOut(m_close.size());

            TA_RetCode status = TA_ATR(0, static_cast<int>(m_close.size() - 1), m_high.data(), m_low.data(),
                                       m_close.data(),
                                       indicator->second.m_parameters.find(INDICATOR_PARAMETER_TIME_PERIOD)->second,
                                       &outBeginIndex, &outElementsNumber,
                                       atrBufferOut.data());

            if (status != TA_RetCode::TA_SUCCESS)
                return false;

            for (std::size_t i = 0; i < static_cast<std::size_t>(outBeginIndex); i++) {
                std::vector<double> samples = {0};
                signalPacks[i].m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_ATR, samples);
            }

            for (auto i = static_cast<std::size_t>(outBeginIndex); i < m_close.size(); i++) {
                std::vector<double> samples = {atrBufferOut[i - static_cast<std::size_t>(outBeginIndex)]};
                signalPacks[i].m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_ATR, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_ADX);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> adxBufferOut(m_close.size());
            TA_RetCode status = TA_ADX(0, static_cast<int>(m_close.size() - 1), m_high.data(), m_low.data(),
                                       m_close.data(),
                                       indicator->second.m_parameters.find(INDICATOR_PARAMETER_TIME_PERIOD)->second,
                                       &outBeginIndex, &outElementsNumber,
                                       adxBufferOut.data());

            if (status != TA_RetCode::TA_SUCCESS)
                return false;

            for (std::size_t i = 0; i < static_cast<std::size_t>(outBeginIndex); i++) {
                std::vector<double> samples = {0};
                signalPacks[i].m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_ADX, samples);
            }

            for (auto i = static_cast<std::size_t>(outBeginIndex); i < m_close.size(); i++) {
                std::vector<double> samples = {adxBufferOut[i - static_cast<std::size_t>(outBeginIndex)]};
                signalPacks[i].m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_ADX, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_MACD);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> macdMACDBufferOut(m_close.size());
            std::vector<double> macdSignalBufferOut(m_close.size());
            std::vector<double> macdHistogramBufferOut(m_close.size());

            TA_RetCode status = TA_MACDEXT(0, static_cast<int>(m_close.size() - 1), m_close.data(),
                                           indicator->second.m_parameters.find(
                                                   INDICATOR_PARAMETER_TIME_PERIOD_FAST)->second,
                                           TA_MAType::TA_MAType_EMA,
                                           indicator->second.m_parameters.find(
                                                   INDICATOR_PARAMETER_TIME_PERIOD_SLOW)->second,
                                           TA_MAType::TA_MAType_EMA,
                                           indicator->second.m_parameters.find(
                                                   INDICATOR_PARAMETER_TIME_PERIOD_SIGNAL)->second,
                                           TA_MAType::TA_MAType_EMA, &outBeginIndex,
                                           &outElementsNumber, macdMACDBufferOut.data(), macdSignalBufferOut.data(),
                                           macdHistogramBufferOut.data());

            if (status != TA_RetCode::TA_SUCCESS)
                return false;

            for (std::size_t i = 0; i < static_cast<std::size_t>(outElementsNumber); i++) {
                std::vector<double> samples = {macdMACDBufferOut[i], macdSignalBufferOut[i], macdHistogramBufferOut[i]};
                signalPacks[static_cast<std::size_t>(outBeginIndex) + i].m_signalGroups.find(
                        SignalGroupType::Main)->second.m_signals.insert_or_assign(INDICATOR_NAME_MACD, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_AROON);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> aroonUpBufferOut(m_close.size());
            std::vector<double> aroonDownBufferOut(m_close.size());

            ATSDSPAlgorithms::aroon(m_high, m_low,
                                    indicator->second.m_parameters.find(INDICATOR_PARAMETER_TIME_PERIOD)->second,
                                    aroonDownBufferOut, aroonUpBufferOut);

            for (std::size_t i = 0; i < m_close.size(); i++) {
                std::vector<double> samples = {aroonDownBufferOut[i], aroonUpBufferOut[i]};
                signalPacks[i].m_signalGroups.find(SignalGroupType::Standalone)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_AROON, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_BB);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> bbUpperBufferOut(m_close.size());
            std::vector<double> bbLowerBufferOut(m_close.size());
            std::vector<double> bbMiddleBufferOut(m_close.size());

            TA_RetCode status = TA_BBANDS(0, static_cast<int>(m_close.size() - 1), m_close.data(),
                                          indicator->second.m_parameters.find(INDICATOR_PARAMETER_TIME_PERIOD)->second,
                                          indicator->second.m_parameters.find(INDICATOR_PARAMETER_DEVIATION_UP)->second,
                                          indicator->second.m_parameters.find(
                                                  INDICATOR_PARAMETER_DEVIATION_DOWN)->second,
                                          TA_MAType::TA_MAType_EMA, &outBeginIndex,
                                          &outElementsNumber, bbUpperBufferOut.data(), bbMiddleBufferOut.data(),
                                          bbLowerBufferOut.data());

            if (status != TA_RetCode::TA_SUCCESS)
                return false;

            for (std::size_t i = 0; i < static_cast<std::size_t>(outElementsNumber); i++) {
                std::vector<double> samples = {bbUpperBufferOut[i], bbMiddleBufferOut[i], bbLowerBufferOut[i]};
                signalPacks[static_cast<std::size_t>(outBeginIndex) + i].m_signalGroups.find(
                        SignalGroupType::Main)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_BB, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_DONCHIAN);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {

            std::vector<double> donchianUpperBufferOut(m_close.size());
            std::vector<double> donchianMiddleBufferOut(m_close.size());
            std::vector<double> donchianLowerBufferOut(m_close.size());

            ATSDSPAlgorithms::donchian(m_high, m_low,
                                       indicator->second.m_parameters.find(INDICATOR_PARAMETER_TIME_PERIOD)->second,
                                       donchianUpperBufferOut,
                                       donchianMiddleBufferOut, donchianLowerBufferOut);

            for (std::size_t i = 0; i < m_close.size(); i++) {
                std::vector<double> samples = {donchianUpperBufferOut[i], donchianMiddleBufferOut[i],
                                               donchianLowerBufferOut[i]};
                signalPacks[static_cast<std::size_t>(outBeginIndex) + i].m_signalGroups.find(
                        SignalGroupType::Main)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_DONCHIAN, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_MAMA);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> mamaBufferOut(m_close.size());
            std::vector<double> famaBufferOut(m_close.size());
            TA_RetCode status = TA_MAMA(0, static_cast<int>(m_close.size() - 1), m_close.data(),
                                        indicator->second.m_parameters.find(INDICATOR_PARAMETER_FAST_LIMIT)->second,
                                        indicator->second.m_parameters.find(INDICATOR_PARAMETER_SLOW_LIMIT)->second,
                                        &outBeginIndex,
                                        &outElementsNumber,
                                        mamaBufferOut.data(),
                                        famaBufferOut.data());

            if (status != TA_RetCode::TA_SUCCESS)
                return false;

            for (std::size_t i = 0; i < static_cast<std::size_t>(outElementsNumber); i++) {
                std::vector<double> samples = {mamaBufferOut[i], famaBufferOut[i]};

                signalPacks[static_cast<std::size_t>(outBeginIndex) + i].m_signalGroups.find(
                        SignalGroupType::Main)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_MAMA, samples);
            }
        }
    }

    indicator = m_indicators.find(INDICATOR_NAME_ALMA);

    {
        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
            std::vector<double> almaBufferOut(m_close.size());

            ATSDSPAlgorithms::alma(m_close, indicator->second.m_parameters.find(INDICATOR_PARAMETER_TIME_PERIOD)->second, almaBufferOut);

            for (std::size_t i = 0; i < almaBufferOut.size(); i++) {
                std::vector<double> samples = {almaBufferOut[i]};

                signalPacks[i].m_signalGroups.find(
                        SignalGroupType::Main)->second.m_signals.insert_or_assign(
                        INDICATOR_NAME_ALMA, samples);
            }
        }
    }


//    indicator = m_indicators.find(INDICATOR_NAME_MA);
//
//    {
//        if (indicator != m_indicators.end() && indicator->second.m_enabled) {
//        std::vector<double> samples (indicator->second.m_parameters.size());
//
//        for (auto i = 0; i < indicator->second.m_parameters.size(); i++) {
//            std::vector<double> maBufferOut(m_close.size());
//            {
//                TA_RetCode status = TA_EMA(0, static_cast<int>(m_close.size() - 1), m_close.data(), timePeriod.second, &outBeginIndex,
//                                           &outElementsNumber,
//                                           maBufferOut.data());
//
//                if (status != TA_RetCode::TA_SUCCESS)
//                    return false;
//
//                for (std::size_t i = 0; i < static_cast<std::size_t>(outBeginIndex); i++) {
//                    signalPacks[i].m_ma[0] = 0;
//                }
//
//                for (auto i = static_cast<std::size_t>(outBeginIndex); i < m_close.size(); i++) {
//                    signalPacks[i].m_ma[0] = maBufferOut[i - static_cast<std::size_t>(outBeginIndex)];
//                }
//            }
//        }
//        }
//    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSProcessor::P::computePositions(const ATSSignalPack &signalPack) const {
    m_strategy->computePositions(signalPack);
}

//----------------------------------------------------------------------------------------------------------------------
ATSSignalPack ATSProcessor::process(const ATSBar &bar) {
    m_p->m_open.push_back(bar.m_open);
    m_p->m_high.push_back(bar.m_high);
    m_p->m_low.push_back(bar.m_low);
    m_p->m_close.push_back(bar.m_close);
    m_p->m_unixTime.push_back(bar.m_unixTime);

    if (m_p->m_close.size() > 5) {
        m_p->m_strategy->setEnabled(true);
    }

    if (QDateTime::fromSecsSinceEpoch(bar.m_unixTime).time() > m_p->m_closeTime) {
        m_p->m_strategy->setEnabled(false);
    }

    std::vector<ATSSignalPack> signalPacks;

    m_p->computeIndicators(signalPacks);

    auto retVal = signalPacks[signalPacks.size() - 1];
    m_p->computePositions(retVal);
    return retVal;

}
}
