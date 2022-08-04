#include "ats_ui_utils.h"
#include "thirdparty/qcustomplot.h"

//----------------------------------------------------------------------------------------------------------------------
ATSUiUtils::ATSUiUtils() {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSUiUtils::setCustomPlot(QCustomPlot *customPlot) {
    customPlot->xAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->yAxis->setBasePen(QPen(Qt::white, 1));
    customPlot->xAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setTickPen(QPen(Qt::white, 1));
    customPlot->xAxis->setSubTickPen(QPen(Qt::white, 1));
    customPlot->yAxis->setSubTickPen(QPen(Qt::white, 1));

    customPlot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    customPlot->xAxis->grid()->setSubGridVisible(true);
    customPlot->yAxis->grid()->setSubGridVisible(true);

    customPlot->xAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setTickLabelColor(Qt::white);
    customPlot->yAxis->setRange(0, 100);

    customPlot->setBackground(QBrush(QColor(53, 53, 53)));
    customPlot->axisRect()->setBackground(QBrush(Qt::black));
    customPlot->legend->setBrush(QBrush(Qt::black));

    customPlot->legend->setTextColor(QColor(255, 255, 255));
    auto font = customPlot->legend->font();
    font.setPointSize(10);
    customPlot->legend->setFont(font);
}
