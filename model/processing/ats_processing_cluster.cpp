#include "ats_processing_cluster.h"
#include "model/ats_utils.h"
#include "ta-lib/ta_common.h"
#include <map>
#include <QDebug>
#include "ats_multiprocessor.h"

namespace trader {

struct ATSProcessingCluster::P {
    ATSProcessingCluster *m_parent = nullptr;
    std::vector<std::shared_ptr<ATSMultiProcessor>> m_multiProcessors;

    explicit P(ATSProcessingCluster *parent) {
        m_parent = parent;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSProcessingCluster::ATSProcessingCluster(QObject *parent) : QObject(parent),
                                                              m_p(spimpl::make_unique_impl<P>(this)) {

}

//----------------------------------------------------------------------------------------------------------------------
ATSProcessingCluster::~ATSProcessingCluster() {
    TA_Shutdown();
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSProcessingCluster::init(const ATSConfiguration &configuration) {
    auto retCode = TA_Initialize();

    if (retCode != TA_RetCode::TA_SUCCESS) {
        qCritical() << "TA library not initialized";
        return false;
    } else {
        qInfo() << "TA library initialized";
    }

    for (auto &contract : configuration.m_ibConfiguration.m_contracts) {
        auto mProcessor = std::make_shared<ATSMultiProcessor>(this);
        mProcessor->init(configuration, contract.symbol);
        m_p->m_multiProcessors.push_back(mProcessor);
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ATSOrderRequest>
ATSProcessingCluster::activeOrderRequest(const std::string &symbol, const BarResolution &resolution) {
    for (auto &mProcessor : m_p->m_multiProcessors) {
        if (mProcessor->contract().symbol == symbol) {
            auto orders = mProcessor->orderRequestsForResolution(resolution);

            if (!orders.empty()) {
                auto retVal = std::make_shared<ATSOrderRequest>();
                *retVal = orders.back();
                return retVal;
            }
        }
    }

    return nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSProcessingCluster::loadActiveOrderRequest(const std::string &symbol, const BarResolution &resolution,
                                                  const ATSOrderRequest &orderRequest) {
    for (auto &mProcessor : m_p->m_multiProcessors) {
        if (mProcessor->contract().symbol == symbol) {
            mProcessor->loadOrderRequestsForResolution(resolution, orderRequest);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
ATSSignals ATSProcessingCluster::process(const ATSBar &bar, const std::string &symbol) {
    for (auto &mProcessor : m_p->m_multiProcessors) {
        if (mProcessor->contract().symbol == symbol) {
            return mProcessor->process(bar);
        }
    }

    return ATSSignals();
}
}
