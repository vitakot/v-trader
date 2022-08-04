#include "ats_dlg_portfolio.h"
#include "ui_ats_dlg_portfolio.h"
#include "ats_ui_utils.h"
#include "model/ats_configuration.h"
#include "ats_axis_ticker_date_time.h"

using namespace trader;

struct ATSDlgPortfolio::P {
    Ui::ATSDlgPortfolio *m_ui = nullptr;
    ATSDlgPortfolio *m_parent = nullptr;
    ATSConfiguration m_configuration;

    P(ATSDlgPortfolio *parent) {
        m_parent = parent;
    }

    ~P() {
        delete m_ui;
    }

    void createUi();
};

//----------------------------------------------------------------------------------------------------------------------
ATSDlgPortfolio::ATSDlgPortfolio(const ATSConfiguration &configuration, QWidget *parent) : QDialog(parent),
                                                                                           m_p(spimpl::make_unique_impl<P>(
                                                                                                   this)) {
    m_p->m_ui = new Ui::ATSDlgPortfolio;
    m_p->m_ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    m_p->m_configuration = configuration;

    // set some pens, brushes and backgrounds:
    ATSUiUtils::setCustomPlot(m_p->m_ui->portfolioPlot);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSDlgPortfolio::P::createUi() {
    {
        QPen pen;
        pen.setWidthF(1);
        pen.setColor(Qt::blue);
        m_ui->portfolioPlot->addGraph();
        m_ui->portfolioPlot->graph()->setPen(pen);
        m_ui->portfolioPlot->graph()->setName("Realized P & L");

        m_ui->portfolioPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

        m_ui->portfolioPlot->legend->setVisible(true);

        QSharedPointer<ATSAxisTickerDateTime> dateTicker(new ATSAxisTickerDateTime);
        dateTicker->setDateTimeFormat("d.M hh:mm:ss");
        m_ui->portfolioPlot->xAxis->setTicker(dateTicker);
    }

    {
        QPen pen;
        pen.setWidthF(1);
        pen.setColor(Qt::green);
        m_ui->portfolioPlot->addGraph();
        m_ui->portfolioPlot->graph()->setPen(pen);
        m_ui->portfolioPlot->graph()->setName("Unrealized P & L");

        m_ui->portfolioPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

        m_ui->portfolioPlot->legend->setVisible(true);

        QSharedPointer<ATSAxisTickerDateTime> dateTicker(new ATSAxisTickerDateTime);
        dateTicker->setDateTimeFormat("d.M hh:mm:ss");
        m_ui->portfolioPlot->xAxis->setTicker(dateTicker);
    }


    QFile inFile(QString::fromStdString(m_configuration.m_ibConfiguration.m_accountStateBaseFilePath));

    QVector<double> timeStamps;
    QVector<double> realizedPnL;
    QVector<double> unrealizedPnL;

    if (inFile.open(QIODevice::ReadOnly)) {

        while (!inFile.atEnd()) {

            QString line = inFile.readLine();
            QStringList values = line.split(',');

            bool ok = false;
            auto time = values[0].toLongLong(&ok);

            if (ok) {
                timeStamps.push_back(time);
            }

            realizedPnL.push_back(values[1].toDouble());
            unrealizedPnL.push_back(values[2].toDouble());

        }
        inFile.close();
    }

    if ((timeStamps.size() == realizedPnL.size() == unrealizedPnL.size()) && timeStamps.size() > 0) {

        m_ui->portfolioPlot->graph(0)->addData(timeStamps, realizedPnL);
        m_ui->portfolioPlot->graph(1)->addData(timeStamps, unrealizedPnL);
    }
}
