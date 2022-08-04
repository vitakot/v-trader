#ifndef ATSPROCESSING_CLUSTER_H
#define ATSPROCESSING_CLUSTER_H

#include "thirdparty/spimpl.hpp"
#include "model/ats_configuration.h"
#include <QObject>
#include "model/ats_order_request.h"
#include "model/processing/ats_processor.h"

namespace trader {

class ATSProcessingCluster : public QObject {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSProcessingCluster(QObject *parent);

    ~ATSProcessingCluster() override;

    bool init(const ATSConfiguration &configuration);

    ATSSignals process(const ATSBar &bar, const std::string &symbol);

    std::shared_ptr<ATSOrderRequest> activeOrderRequest(const std::string &symbol, const BarResolution &resolution);

    void loadActiveOrderRequest(const std::string &symbol, const BarResolution &resolution,
                                const ATSOrderRequest &orderRequest);

signals:

    void orderRequest(const ATSOrderRequest &orderRequest);

    void stopTrading(const ATSOrderRequest &orderRequest);

    void exitRequest(double price, std::int64_t time, const std::string &symbol);

    void trendVariables(bool macdLong, bool macdShort, bool aroonLong, bool aroonShort, bool adxFilterOn);

    void currentProfit(const std::string &symbol, double profit);
};
}
#endif // ATSPROCESSING_CLUSTER_H
