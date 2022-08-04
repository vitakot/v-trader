#include "ats_wdg_plot_main.h"
#include "ats_axis_ticker_date_time.h"
#include "model/ats_configuration.h"
#include "model/processing/ats_processor.h"
#include "ats_ui_utils.h"
#include "ats_dlg_magnifier.h"
#include <limits>

struct ATSGraphGroup {
    QString m_indicatorName;
    QMap<QString, QCPGraph *> m_indicatorGraphs;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSWdgPlotMain::P {

    ATSWdgPlotMain *m_parent = nullptr;
    Contract m_contract;
    std::map<std::string, trader::ATSIndicator> m_indicators;
    trader::BarResolution m_resolution = trader::BarResolution::Sec30;
    QVector<Qt::GlobalColor> m_plotColors;
    QVector<QCPRange> m_xRanges;
    bool m_zoomEnabled = true;
    QCPFinancial *m_candlesticks = nullptr;
    ATSDlgMagnifier *m_dlgMagnifier = nullptr;

    bool m_candlesEnabled = true;
    std::pair<double, double> m_minMax = {std::numeric_limits<double>::max(), std::numeric_limits<double>::min()};

    QMap<QString, std::shared_ptr<ATSGraphGroup>> m_mainGraphs;

    explicit P(ATSWdgPlotMain *parent) {
        m_parent = parent;

        m_plotColors.push_back(Qt::green);
        m_plotColors.push_back(Qt::blue);
        m_plotColors.push_back(Qt::red);
        m_plotColors.push_back(Qt::magenta);
        m_plotColors.push_back(Qt::cyan);
        m_plotColors.push_back(Qt::yellow);
        m_plotColors.push_back(Qt::darkMagenta);
        m_plotColors.push_back(Qt::darkCyan);
    }

    void createMainGraphs();
};

//----------------------------------------------------------------------------------------------------------------------
ATSWdgPlotMain::ATSWdgPlotMain(QWidget *parent) : QCustomPlot(parent), m_p(spimpl::make_unique_impl<P>(this)) {
    ATSUiUtils::setCustomPlot(this);

    axisRect()->setRangeDrag(Qt::Horizontal);
    axisRect()->setRangeZoom(Qt::Horizontal);

    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), this, SIGNAL(xRangeChanged(QCPRange)));

    connect(this, &QCustomPlot::mouseWheel, this, [=]() {
        m_p->m_xRanges.push_back(xAxis->range());
        emit zoomEnabled(false);
    });

    connect(this, &QCustomPlot::mouseDoubleClick, this, [=]() {

        if (!m_p->m_xRanges.empty()) {
            xAxis->setRange(m_p->m_xRanges.front());
            replot();
            m_p->m_xRanges.clear();
            emit zoomEnabled(true);
        }
    });

    m_p->m_dlgMagnifier = new ATSDlgMagnifier(this);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::setContract(const Contract &contract) {
    m_p->m_contract = contract;
    clearGraphs();
    m_p->createMainGraphs();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::setIndicators(const std::map<std::string, trader::ATSIndicator> &indicators) {
    m_p->m_indicators = indicators;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::setResolution(const trader::BarResolution &resolution) {
    m_p->m_resolution = resolution;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::changeXRange(const QCPRange &range) {
    blockSignals(true);
    xAxis->setRange(range);
    replot();
    blockSignals(false);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::setZoomEnabled(bool isEnabled) {
    m_p->m_zoomEnabled = isEnabled;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::setCandlesEnabled(bool candlesEnabled) {
    m_p->m_candlesEnabled = candlesEnabled;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::addMarker(double price, std::int64_t time, const std::string &symbol) {
    QCPGraph *currentGraph = graph(3);

    if (!currentGraph) {
        return;
    }

    QCPGraphData traderPoint;
    traderPoint.key = time;
    traderPoint.value = price;

    currentGraph->data()->add(traderPoint);
    replot();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::addIndicatorValues(const trader::ATSSignalGroup &signalGroup, std::int64_t unixTime) {

    for (const auto &signal : signalGroup.m_signals) {

        auto graphsIter = m_p->m_mainGraphs.find(QString::fromStdString(signal.first));

        if (graphsIter == m_p->m_mainGraphs.end()) {
            auto indicator = m_p->m_indicators.find(signal.first);

            if (indicator != m_p->m_indicators.end()) {
                auto graphGroup = std::make_shared<ATSGraphGroup>();
                graphGroup->m_indicatorName = QString::fromStdString(signal.first);

                for (auto i = 0; i < indicator->second.m_signalNames.size(); i++) {
                    auto signalName =
                            QString::fromStdString(m_p->m_contract.symbol) + ": " + graphGroup->m_indicatorName + " " +
                            QString::fromStdString(indicator->second.m_signalNames[i]);

                    QPen pen;
                    pen.setWidthF(1);
                    pen.setColor(m_p->m_plotColors[i]);
                    auto newGraph = addGraph();
                    newGraph->setPen(pen);
                    newGraph->setName(signalName);
                    graphGroup->m_indicatorGraphs.insert(signalName, newGraph);

                    m_p->m_mainGraphs.insert(QString::fromStdString(signal.first), graphGroup);
                }
            }
        } else {
            auto graphGroup = *graphsIter;
            std::size_t i = 0;
            for (auto &graph : graphGroup->m_indicatorGraphs) {
                QCPGraphData newData;
                newData.key = unixTime;
                newData.value = signal.second[i++];
                graph->data()->add(newData);
            }
        }
    }

    replot();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::P::createMainGraphs() {

    m_parent->setCurrentLayer("main");

    {
        QPen pen;
        pen.setWidthF(1);
        pen.setColor(Qt::blue);
        m_parent->addGraph();
        m_parent->graph()->setPen(pen);
        m_parent->graph()->setName(QString::fromStdString(m_contract.symbol) + ": Points");

        m_parent->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
        m_parent->legend->setVisible(true);

        QSharedPointer<ATSAxisTickerDateTime> dateTicker(new ATSAxisTickerDateTime);
        dateTicker->setDateTimeFormat("d.M hh:mm:ss");
        m_parent->xAxis->setTicker(dateTicker);
    }

    if (m_candlesEnabled) {
        m_candlesticks = new QCPFinancial(m_parent->xAxis, m_parent->yAxis);
        m_candlesticks->setName(QString::fromStdString(m_contract.symbol) + ": Candlestick");
        m_candlesticks->setChartStyle(QCPFinancial::csCandlestick);
        m_candlesticks->setWidth(trader::secondsDurationForResolution(m_resolution) * 0.9);
        m_candlesticks->setTwoColored(true);
        m_candlesticks->setBrushPositive(Qt::darkGreen);
        m_candlesticks->setBrushNegative(Qt::darkRed);
        m_candlesticks->setPenPositive(QPen(Qt::darkGreen));
        m_candlesticks->setPenNegative(QPen(Qt::darkRed));
    }

    {
        QPen pen;
        pen.setColor(Qt::green);
        m_parent->addGraph();
        m_parent->graph()->setPen(pen);
        m_parent->graph()->setName(QString::fromStdString(m_contract.symbol + ": Long trades"));
        m_parent->graph()->setLineStyle(QCPGraph::lsNone);
        m_parent->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));

        pen.setColor(Qt::red);
        m_parent->addGraph();
        m_parent->graph()->setPen(pen);
        m_parent->graph()->setName(QString::fromStdString(m_contract.symbol + ": Short trades"));
        m_parent->graph()->setLineStyle(QCPGraph::lsNone);
        m_parent->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 10));

        pen.setColor(Qt::cyan);
        m_parent->addGraph();
        m_parent->graph()->setPen(pen);
        m_parent->graph()->setName(QString::fromStdString(m_contract.symbol + ": Markers"));
        m_parent->graph()->setLineStyle(QCPGraph::lsNone);
        m_parent->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCrossSquare, 10));
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::addBar(const trader::ATSBar &bar) {

    if (m_p->m_minMax.first > bar.m_close) {
        m_p->m_minMax.first = bar.m_close - (bar.m_close / 100.0) * 0.1;
    }

    if (m_p->m_minMax.second < bar.m_close) {
        m_p->m_minMax.second = bar.m_close + (bar.m_close / 100.0) * 0.1;
    }

    setCurrentLayer("main");

    if (graph(0)->data()->size() == 0) {
        xAxis->setRange(bar.m_unixTime, bar.m_unixTime + trader::ATSConfiguration::s_numVisibleSeconds);
    }

    if (m_p->m_candlesEnabled) {
        m_p->m_candlesticks->addData(bar.m_unixTime, bar.m_open, bar.m_high, bar.m_low, bar.m_close);
    }

    QCPGraphData newData;
    newData.key = bar.m_unixTime;
    newData.value = bar.m_close;

    graph(0)->data()->add(newData);
    yAxis->setRange(m_p->m_minMax.first, m_p->m_minMax.second);

    if (m_p->m_zoomEnabled) {
        auto rangeX = xAxis->range();

        if (newData.key + 600 > rangeX.upper) {
            xAxis->setRangeUpper(rangeX.upper + 1200);
            xAxis->setRangeLower(rangeX.lower + 1200);
        }
    }

    replot();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::addOrderMark(const trader::ATSIBOrder &order) {
    QCPGraph *currentGraph = nullptr;

    if (order.m_order.action == "BUY") {
        currentGraph = graph(1);
    } else if (order.m_order.action == "SELL") {
        currentGraph = graph(2);
    } else {
        qDebug() << "unknown order action.";
        return;
    }

    if (!currentGraph) {
        return;
    }

    QCPGraphData traderPoint;

    auto dateTime = QDateTime::fromString(QString::fromStdString(order.m_orderState.completedTime),
                                          "yyyyMMdd  hh:mm:ss");
    traderPoint.key = dateTime.toSecsSinceEpoch();
    traderPoint.value = order.m_orderStatus.m_avgFillPrice;

    currentGraph->data()->add(traderPoint);
    replot();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::addTradingClosedMark(const trader::ATSOrderRequest &orderRequest) {

    QCPGraph *currentGraph = graph(3);

    if (!currentGraph) {
        return;
    }

    QCPGraphData traderPoint;
    traderPoint.key = orderRequest.enterTime();
    traderPoint.value = orderRequest.enterPrice();

    currentGraph->data()->add(traderPoint);
    replot();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotMain::contextMenuEvent(QContextMenuEvent *event) {
    QCustomPlot::contextMenuEvent(event);

    m_p->m_dlgMagnifier->show();
}
