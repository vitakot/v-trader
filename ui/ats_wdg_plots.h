#ifndef ATS_WDG_PLOTS_H
#define ATS_WDG_PLOTS_H

#include <QWidget>
#include "thirdparty/spimpl.hpp"
#include "model/ats_order_request.h"
#include "thirdparty/IB/client/bar.h"
#include "model/processing/ats_processor.h"
#include "model/IB/ats_ib_orders.h"

class ATSWDGPlots : public QWidget {
Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSWDGPlots(QWidget *parent = nullptr);

    ~ATSWDGPlots() = default;

    void addOrderMark(const trader::ATSIBOrder &order);

    void addBar(const trader::ATSBar &bar);

    void addProcessedBar(const std::map<trader::BarResolution, trader::ATSSignalPack> &signalPacks);

    void addTradingClosedMark(const trader::ATSOrderRequest &orderRequest);

    void setContract(const Contract &contract);

    void setResolutions(const std::vector<trader::BarResolution> &resolutions);

    void addMarker(double price, std::int64_t time, const std::string &symbol);

    void setIndicators(const trader::ATSConfiguration &config);
};

#endif // ATS_WDG_PLOTS_H
