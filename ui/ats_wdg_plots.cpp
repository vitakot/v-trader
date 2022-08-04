#include "ats_wdg_plots.h"
#include "ui_ats_wdg_plots.h"
#include "ats_wdg_plot_indicator.h"

using namespace trader;

struct ATSWDGPlots::P {
    Ui::ATSWDGPlots *m_ui = nullptr;
    ATSWDGPlots *m_parent = nullptr;
    trader::BarResolution m_currentResolution;
    std::map<std::string, ATSIndicator> m_indicators;
    QMap<QString, ATSWdgPlotIndicator *> m_indicatorPlots;
    QCPMarginGroup *m_marginGroup = nullptr;

    void switchResolution(const trader::BarResolution &resolution);

    explicit P(ATSWDGPlots *parent) {
        m_parent = parent;
    }

    ~P() {
        delete m_ui;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSWDGPlots::ATSWDGPlots(QWidget *parent) : QWidget(parent), m_p(spimpl::make_unique_impl<P>(this)) {
    m_p->m_ui = new Ui::ATSWDGPlots;
    m_p->m_ui->setupUi(this);

    m_p->m_marginGroup = new QCPMarginGroup(m_p->m_ui->mainPlot);
    m_p->m_ui->mainPlot->axisRect(0)->setMarginGroup(QCP::msLeft | QCP::msRight, m_p->m_marginGroup);
    m_p->m_ui->horizontalLayoutResolution->setAlignment(Qt::AlignLeft);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::setIndicators(const trader::ATSConfiguration &config) {
    m_p->m_indicators = config.m_indicators;
    m_p->m_ui->mainPlot->setIndicators(config.m_indicators);
    m_p->m_ui->mainPlot->setCandlesEnabled(config.m_showCandles);

    auto indicator = m_p->m_indicators.find(INDICATOR_NAME_MACD);

    if (indicator != m_p->m_indicators.end() && indicator->second.m_enabled) {
        auto plot = new ATSWdgPlotIndicator(this, INDICATOR_NAME_MACD);
        plot->setSignalLabels(indicator->second.m_signalNames);
        plot->setGraphTypes({ATSWdgPlotIndicator::GraphType::Line, ATSWdgPlotIndicator::GraphType::Line,
                             ATSWdgPlotIndicator::GraphType::Histogram});
        m_p->m_indicatorPlots.insert(INDICATOR_NAME_MACD, plot);
        auto *prd = dynamic_cast<QVBoxLayout *>(this->layout());
        prd->addWidget(plot, 1);
    }

    indicator = m_p->m_indicators.find(INDICATOR_NAME_ADX);

    if (indicator != m_p->m_indicators.end() && indicator->second.m_enabled) {
        auto plot = new ATSWdgPlotIndicator(this, INDICATOR_NAME_ADX);
        plot->setSignalLabels(indicator->second.m_signalNames);
        plot->setRange(0, 100);
        plot->addLine(indicator->second.m_parameters.find(INDICATOR_PARAMETER_THRESHOLD)->second, Qt::red);
        plot->setGraphTypes({ATSWdgPlotIndicator::GraphType::Line});
        m_p->m_indicatorPlots.insert(INDICATOR_NAME_ADX, plot);
        auto *prd = dynamic_cast<QVBoxLayout *>(this->layout());
        prd->addWidget(plot, 1);
    }

    indicator = m_p->m_indicators.find(INDICATOR_NAME_ATR);

    if (indicator != m_p->m_indicators.end() && indicator->second.m_enabled) {
        auto plot = new ATSWdgPlotIndicator(this, INDICATOR_NAME_ATR);
        plot->setSignalLabels(indicator->second.m_signalNames);
        plot->setGraphTypes({ATSWdgPlotIndicator::GraphType::Line});
        m_p->m_indicatorPlots.insert(INDICATOR_NAME_ATR, plot);
        auto *prd = dynamic_cast<QVBoxLayout *>(this->layout());
        prd->addWidget(plot, 1);
    }

    indicator = m_p->m_indicators.find(INDICATOR_NAME_AROON);

    if (indicator != m_p->m_indicators.end() && indicator->second.m_enabled) {
        auto plot = new ATSWdgPlotIndicator(this, INDICATOR_NAME_AROON);
        plot->setSignalLabels(indicator->second.m_signalNames);
        plot->setRange(0, 100);
        plot->setGraphTypes({ATSWdgPlotIndicator::GraphType::Line, ATSWdgPlotIndicator::GraphType::Line});
        m_p->m_indicatorPlots.insert(INDICATOR_NAME_AROON, plot);
        auto *prd = dynamic_cast<QVBoxLayout *>(this->layout());
        prd->addWidget(plot, 1);
    }

    for (auto &plot : m_p->m_indicatorPlots) {
        plot->axisRect(0)->setMarginGroup(QCP::msLeft | QCP::msRight, m_p->m_marginGroup);
    }

    for (auto &plot : m_p->m_indicatorPlots) {
        connect(m_p->m_ui->mainPlot, &ATSWdgPlotMain::xRangeChanged, plot, &ATSWdgPlotIndicator::changeXRange);
        connect(plot, &ATSWdgPlotIndicator::xRangeChanged, m_p->m_ui->mainPlot, &ATSWdgPlotMain::changeXRange);
    }

    for (auto &plot : m_p->m_indicatorPlots) {
        for (auto &restPlot : m_p->m_indicatorPlots) {
            if (plot != restPlot) {
                connect(plot, &ATSWdgPlotIndicator::xRangeChanged, restPlot, &ATSWdgPlotIndicator::changeXRange);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::addOrderMark(const trader::ATSIBOrder &order) {
    m_p->m_ui->mainPlot->addOrderMark(order);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::addBar(const trader::ATSBar &bar) {
    m_p->m_ui->mainPlot->addBar(bar);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::addProcessedBar(const std::map<trader::BarResolution, trader::ATSSignalPack> &signalPacks) {
    for (const auto &signalPack : signalPacks) {
        if (signalPack.first == m_p->m_currentResolution) {

            trader::ATSSignalGroup standaloneGroup = signalPack.second.m_signalGroups.find(
                    SignalGroupType::Standalone)->second;

            m_p->m_ui->mainPlot->addIndicatorValues(
                    signalPack.second.m_signalGroups.find(SignalGroupType::Main)->second,
                    signalPack.second.m_bar.m_unixTime);

            {
                auto iter = m_p->m_indicatorPlots.find(INDICATOR_NAME_ADX);
                if (iter != m_p->m_indicatorPlots.end()) {
                    iter.value()->addData(standaloneGroup.m_signals.find(INDICATOR_NAME_ADX)->second,
                                          signalPack.second.m_bar.m_unixTime);
                }
            }
            {
                auto iter = m_p->m_indicatorPlots.find(INDICATOR_NAME_MACD);
                if (iter != m_p->m_indicatorPlots.end()) {
                    iter.value()->addData(standaloneGroup.m_signals.find(INDICATOR_NAME_MACD)->second,
                                          signalPack.second.m_bar.m_unixTime);
                }
            }

            {
                auto iter = m_p->m_indicatorPlots.find(INDICATOR_NAME_ATR);
                if (iter != m_p->m_indicatorPlots.end()) {
                    iter.value()->addData(standaloneGroup.m_signals.find(INDICATOR_NAME_ATR)->second,
                                          signalPack.second.m_bar.m_unixTime);
                }
            }
            {
                auto iter = m_p->m_indicatorPlots.find(INDICATOR_NAME_AROON);
                if (iter != m_p->m_indicatorPlots.end()) {
                    iter.value()->addData(standaloneGroup.m_signals.find(INDICATOR_NAME_AROON)->second,
                                          signalPack.second.m_bar.m_unixTime);
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::addMarker(double price, std::int64_t time, const std::string &symbol) {
    m_p->m_ui->mainPlot->addMarker(price, time, symbol);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::addTradingClosedMark(const trader::ATSOrderRequest &orderRequest) {
    m_p->m_ui->mainPlot->addTradingClosedMark(orderRequest);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::setContract(const Contract &contract) {
    m_p->m_ui->mainPlot->setContract(contract);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::setResolutions(const std::vector<trader::BarResolution> &resolutions) {
    if (resolutions.empty()) {
        return;
    }

    m_p->m_currentResolution = *resolutions.begin();

    QLayoutItem *child;
    while ((child = m_p->m_ui->horizontalLayoutResolution->takeAt(0)) != nullptr) {
        delete child;
    }

    for (const auto &resolution : resolutions) {
        QString buttonTitle = QString::fromStdString(barResolutionToNiceString(resolution));
        auto *rButton = new QRadioButton(buttonTitle, this);

        if (resolution == m_p->m_currentResolution) {
            rButton->setChecked(true);
        }

        connect(rButton, &QRadioButton::clicked, this, [=]() {
            m_p->switchResolution(resolution);
        });

        m_p->m_ui->horizontalLayoutResolution->addWidget(rButton);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWDGPlots::P::switchResolution(const trader::BarResolution &resolution) {
    m_currentResolution = resolution;
    m_ui->mainPlot->setResolution(m_currentResolution);
}

