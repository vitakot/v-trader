#include "ats_ib_downloader.h"
#include <fstream>
#include <QDebug>
#include "thirdparty/IB/client/EClientSocket.h"
#include <QMessageBox>
#include "ats_ib_client.hpp"
#include <QDateTime>

namespace trader {

struct ATSIBDownloader::P {

    ATSIBClient *m_ibClient = nullptr;
    std::vector<ATSBar> m_historicalBars;
    TickerId m_currentTicker = -1;
    Contract m_currentContract;
    std::string m_currentResolution;
    std::int64_t m_currentQueryTimeStamp = 0;
    std::string m_currentDataLength;
    std::string m_currentDataType;
};

//----------------------------------------------------------------------------------------------------------------------
ATSIBDownloader::ATSIBDownloader(QObject *parent) : QObject(parent), m_p(spimpl::make_unique_impl<P>()) {
    m_p->m_ibClient = qobject_cast<ATSIBClient *>(parent);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBDownloader::downloadDataForSymbol(const std::string &symbol, const std::string &resolution,
                                            const std::string &historicDataLength) {
    std::time_t rawtime;
    std::tm *timeinfo;
    char queryTime[80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    std::strftime(queryTime, 80, "%Y%m%d %H:%M:%S", timeinfo);

    m_p->m_currentQueryTimeStamp = QDateTime::fromString(QString::fromStdString(queryTime),
                                                         "yyyyMMdd hh:mm:ss").toSecsSinceEpoch();
    m_p->m_currentResolution = resolution;
    m_p->m_currentDataLength = historicDataLength;

    m_p->m_currentContract.symbol = symbol;
    m_p->m_currentContract.secType = "STK";
    m_p->m_currentContract.currency = m_p->m_ibClient->configuration().m_baseCurrency;
    std::string whatToSee = "TRADES";

    if (symbol == "EUR" || symbol == "GBP") {
        m_p->m_currentContract.secType = "CASH";
        m_p->m_currentContract.exchange = "IDEALPRO";
        whatToSee = "MIDPOINT";
    } else {
        m_p->m_currentContract.secType = "STK";
        m_p->m_currentContract.exchange = "SMART";
    }

    m_p->m_currentDataType = whatToSee;
    m_p->m_historicalBars.clear();

    m_p->m_currentTicker = TickerIdGenerator::nextId();

    m_p->m_ibClient->socket()->reqHistoricalData(m_p->m_currentTicker, m_p->m_currentContract, queryTime,
                                                 m_p->m_currentDataLength,
                                                 m_p->m_currentResolution,
                                                 m_p->m_currentDataType, 1, 1, false,
                                                 TagValueListSPtr());
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBDownloader::onHistoricalBarReceived(TickerId tickerId, const ATSBar &bar) {
    if (m_p->m_currentTicker == tickerId) {
        m_p->m_historicalBars.push_back(bar);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBDownloader::onHistoricalDataEnd(TickerId tickerId) {
    m_p->m_ibClient->socket()->cancelHistoricalData(tickerId);

    if (m_p->m_currentTicker == tickerId) {
        std::string fileName = historicalDataFileNameTemplate(m_p->m_currentQueryTimeStamp,
                                                              m_p->m_currentDataLength, m_p->m_currentDataType,
                                                              m_p->m_currentContract.symbol,
                                                              m_p->m_currentResolution);

        std::string fullPath = m_p->m_ibClient->configuration().m_downloaderBaseFilePath + "/" + fileName;

        auto ok = saveTimeSeriesToCSV(fullPath, m_p->m_historicalBars);

        if (ok) {
            emit message(QMessageBox::Icon::Information,
                         "Historical data for symbol " + m_p->m_currentContract.symbol + " downloaded.");
        } else {
            emit message(QMessageBox::Icon::Information,
                         "Could not save historical data for symbol " + m_p->m_currentContract.symbol + ".");
        }
    }
}

}
