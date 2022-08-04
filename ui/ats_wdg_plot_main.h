#ifndef ATS_WDG_PLOT_MAIN_H
#define ATS_WDG_PLOT_MAIN_H

#include "thirdparty/qcustomplot.h"
#include "thirdparty/IB/client/bar.h"
#include "thirdparty/IB/client/Contract.h"
#include "thirdparty/spimpl.hpp"
#include "model/ats_order_request.h"
#include "model/IB/ats_ib_orders.h"
#include "model/ats_configuration.h"
#include "model/interface/ats_iprocessor.h"

class ATSWdgPlotMain : public QCustomPlot {
Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

    void contextMenuEvent(QContextMenuEvent *event) override;

public:
    explicit ATSWdgPlotMain(QWidget *parent = nullptr);

    ~ATSWdgPlotMain() override = default;

    void addBar(const trader::ATSBar &bar);

    void addIndicatorValues(const trader::ATSSignalGroup &signalGroup, std::int64_t unixTime);

    void setContract(const Contract &contract);

    void addOrderMark(const trader::ATSIBOrder &order);

    void addTradingClosedMark(const trader::ATSOrderRequest &orderRequest);

    void addMarker(double price, std::int64_t time, const std::string &symbol);

    void setIndicators(const std::map<std::string, trader::ATSIndicator> &indicators);

    void setResolution(const trader::BarResolution &resolution);

    void setCandlesEnabled(bool candlesEnabled);

public slots:

    void changeXRange(const QCPRange &range);

    void setZoomEnabled(bool isEnabled);

signals:

    void xRangeChanged(const QCPRange &range);

    void zoomEnabled(bool isEnabled);
};

#endif // ATS_WDG_PLOT_MAIN_H
