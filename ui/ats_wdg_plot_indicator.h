#ifndef ATS_WDG_PLOT_INDICATOR_H
#define ATS_WDG_PLOT_INDICATOR_H

#include "thirdparty/qcustomplot.h"
#include "thirdparty/spimpl.hpp"

class ATSWdgPlotIndicator : public QCustomPlot {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:

    enum class GraphType : int {
        Line = 0,
        Histogram
    };

    explicit ATSWdgPlotIndicator(QWidget *parent, const std::string &name);

    void clear();

    void addData(const std::vector<double> &samples, std::int64_t time);

    void setRange(double min, double max);

    void addLine(double level, Qt::GlobalColor color);

    void setSignalLabels(const std::vector<std::string> &labels);

    void setGraphTypes(const QVector<GraphType> &graphTypes);

public slots:

    void changeXRange(const QCPRange &range);

    void setZoomEnabled(bool isEnabled);

signals:

    void xRangeChanged(const QCPRange &range);

    void zoomEnabled(bool isEnabled);
};

#endif //ATS_WDG_PLOT_INDICATOR_H
