#include "ats_wdg_plot_indicator.h"
#include "ats_axis_ticker_date_time.h"
#include "model/ats_configuration.h"
#include "ats_ui_utils.h"
#include <limits>

struct LevelLine {
    double m_level = 0.0;
    Qt::GlobalColor m_color = Qt::GlobalColor::blue;
    QCPItemLine *m_line = nullptr;
};

//----------------------------------------------------------------------------------------------------------------------
struct ATSWdgPlotIndicator::P {

    ATSWdgPlotIndicator *m_parent = nullptr;
    QCPItemLine *m_lowerLine = nullptr;
    QVector<QCPRange> m_xRanges;
    bool m_zoomEnabled = true;
    QVector<LevelLine> m_levelLines;
    QVector<Qt::GlobalColor> m_plotColors;
    std::vector<std::string> m_plotLabels;
    QVector<GraphType> m_graphTypes;
    QMap<int, QCPBars *> m_histograms;
    QMap<int, QCPGraph *> m_graphs;
    bool m_fixedRange = false;
    std::pair<double, double> m_minMax = {std::numeric_limits<double>::max(), std::numeric_limits<double>::min()};

    P(ATSWdgPlotIndicator *parent) {
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
};

//----------------------------------------------------------------------------------------------------------------------
ATSWdgPlotIndicator::ATSWdgPlotIndicator(QWidget *parent, const std::string &name) : QCustomPlot(parent),
                                                                                 m_p(spimpl::make_unique_impl<P>(
                                                                                         this)) {
    // set some pens, brushes and backgrounds:
    ATSUiUtils::setCustomPlot(this);

    setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);

    plotLayout()->insertRow(0);
    auto plotTitle = new QCPTextElement(this, QString::fromStdString(name), QFont("sans", 10, QFont::Bold));
    plotTitle->setTextColor(Qt::white);
    plotLayout()->addElement(0, 0, plotTitle);

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
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::setRange(double min, double max) {
    m_p->m_fixedRange = true;
    yAxis->setRange(min, max);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::addLine(double level, Qt::GlobalColor color) {
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidthF(0.75);

    LevelLine line;
    line.m_color = color;
    line.m_level = level;
    line.m_line = new QCPItemLine(this);
    line.m_line->setPen(pen);
    line.m_line->start->setCoords(xAxis->range().lower, level);
    line.m_line->end->setCoords(xAxis->range().upper, level);

    m_p->m_levelLines.push_back(line);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::setSignalLabels(const std::vector<std::string> &labels) {
    m_p->m_plotLabels = labels;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::setGraphTypes(const QVector<GraphType> &graphTypes) {
    m_p->m_graphTypes = graphTypes;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::clear() {
    clearGraphs();
    replot();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::changeXRange(const QCPRange &range) {
    blockSignals(true);
    xAxis->setRange(range);
    replot();
    blockSignals(false);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::setZoomEnabled(bool isEnabled) {
    m_p->m_zoomEnabled = isEnabled;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWdgPlotIndicator::addData(const std::vector<double> &samples, std::int64_t time) {
    if (m_p->m_plotLabels.size() != static_cast<int>(samples.size())) {
        return;
    }

    if (m_p->m_plotLabels.size() != static_cast<int>(m_p->m_graphTypes.size())) {
        return;
    }

    for (auto &sample : samples) {
        if (m_p->m_minMax.first > sample) {
            if (sample < 0) {
                m_p->m_minMax.first = sample + (sample / 100.0) * 2;
            } else {
                m_p->m_minMax.first = sample - (sample / 100.0) * 2;
            }
        }

        if (m_p->m_minMax.second < sample) {
            m_p->m_minMax.second = sample + (sample / 100.0) * 2;
        }
    }

    if ((m_p->m_graphs.size() + m_p->m_histograms.size()) != static_cast<int>(samples.size())) {

        for (auto i = 0; i < static_cast<int>(samples.size()); i++) {
            QPen pen;
            pen.setWidthF(1);
            pen.setColor(m_p->m_plotColors[i]);

            if (m_p->m_graphTypes[i] == GraphType::Line) {
                auto newGraph = addGraph();
                m_p->m_graphs.insert(i, newGraph);
                graph()->setPen(pen);
                graph()->setName(QString::fromStdString(m_p->m_plotLabels[i]));
            } else if (m_p->m_graphTypes[i] == GraphType::Histogram) {
                auto histogram = new QCPBars(xAxis, yAxis);
                histogram->setPen(pen);
                histogram->setName("MACD Histogram");
                m_p->m_histograms.insert(i, histogram);
            }
        }

        setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

        xAxis->setRangeUpper(100);
        legend->setVisible(true);

        QSharedPointer <ATSAxisTickerDateTime> dateTicker(new ATSAxisTickerDateTime);
        dateTicker->setDateTimeFormat("d.M hh:mm:ss");
        xAxis->setTicker(dateTicker);
        xAxis->setRange(time, time + trader::ATSConfiguration::s_numVisibleSeconds);
    }

    for (std::size_t i = 0; i < samples.size(); i++) {
        QCPGraphData newIndicatorData;
        newIndicatorData.key = time;
        newIndicatorData.value = samples[i];

        if (m_p->m_graphTypes[static_cast<int>(i)] == GraphType::Line) {
            m_p->m_graphs[static_cast<int>(i)]->data()->add(newIndicatorData);
        } else if (m_p->m_graphTypes[static_cast<int>(i)] == GraphType::Histogram) {
            m_p->m_histograms[static_cast<int>(i)]->addData(time, samples[i]);
        }

        if (m_p->m_zoomEnabled) {
            auto rangeX = xAxis->range();

            if (newIndicatorData.key + 600 > rangeX.upper) {
                xAxis->setRangeUpper(rangeX.upper + 1200);
                xAxis->setRangeLower(rangeX.lower + 1200);
            }
        }
    }

    for (auto &line : m_p->m_levelLines) {
        line.m_line->end->setCoords(xAxis->range().upper, line.m_level);
    }

    if (!m_p->m_fixedRange) {
        yAxis->setRange(m_p->m_minMax.first, m_p->m_minMax.second);
    }

    replot();
}
