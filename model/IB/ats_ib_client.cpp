#include "ats_ib_client.hpp"
#include "thirdparty/IB/client/EReaderOSSignal.h"
#include "thirdparty/IB/client/EReader.h"
#include "ats_ib_events.h"
#include "ats_ib_orders.h"
#include <QDebug>
#include <thread>
#include <QStandardPaths>
#include <future>
#include "model/IB/ats_ib_account.h"
#include <QDateTime>
#include <fstream>
#include "model/processing/ats_processor.h"
#include "ats_ib_downloader.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace trader {
struct ATSIBClient::P {

    ATSIBClient *m_parent = nullptr;
    std::shared_ptr<EClientSocket> m_socket;
    std::shared_ptr<ATSIBEvents> m_ibEvents;
    std::shared_ptr<EReader> m_pReader;
    std::shared_ptr<ATSIBOrders> m_orders;
    std::shared_ptr<ATSIBAccount> m_account;
    std::shared_ptr<ATSIBDownloader> m_downloader;
    IBCleaner m_ibCleaner;
    std::vector<Contract> m_contractsToDataRetrieve;
    std::map<TickerId, std::string> m_realTimeDataTickers;
    std::map<std::string, ATSBar> m_aggregatedBar;
    std::map<std::string, std::ofstream> m_realTimeDataFiles;
    std::map<std::string, std::ofstream> m_aggregatedTimeDataFiles;
    std::atomic<bool> m_nextBar = false;
    std::atomic<bool> m_allHistoricBars = false;
    std::condition_variable m_waitForNextBar;
    std::mutex m_waitMutex;
    double m_currentCZKUSDRate = 0.042;
    bool m_exitReadingThread = false;

    IBConfiguration m_ibConfiguration;

    bool m_extraAuth = false;

    EReaderOSSignal m_osSignal = 2000; //2-seconds timeout

    std::thread m_msgThread;
    std::thread m_testThread;

    std::atomic<bool> m_isReadingMsgs = {false};

    static void readMessages(ATSIBClient *owner);

    explicit P(ATSIBClient *parent) {
        m_parent = parent;
        m_ibEvents = std::make_shared<ATSIBEvents>();
        m_socket = std::make_shared<EClientSocket>(m_ibEvents.get(), &m_osSignal);
        m_orders = std::make_shared<ATSIBOrders>(m_parent);
        m_account = std::make_shared<ATSIBAccount>(m_parent);
        m_downloader = std::make_shared<ATSIBDownloader>(m_parent);
    }

    ~P() {
        for (auto &file : m_realTimeDataFiles) {
            file.second.flush();
            file.second.close();
        }

        for (auto &file : m_aggregatedTimeDataFiles) {
            file.second.flush();
            file.second.close();
        }
    }

    void retrieveHistoricData();

    void subscribeLiveMarketData();

    void retrieveHistoricDataForContract(Contract contract);
};

//----------------------------------------------------------------------------------------------------------------------
ATSIBClient::ATSIBClient(QObject *parent) : QObject(parent), m_p(spimpl::make_unique_impl<P>(this)) {
    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::accountInfoReceived, m_p->m_account.get(),
            &ATSIBAccount::onAccountInfo,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::accountInfoEndReceived, m_p->m_account.get(),
            &ATSIBAccount::onAccountInfoEnd,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::accountPositionReceived, m_p->m_account.get(),
            &ATSIBAccount::onAccountPositionReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::accountPositionBriefReceived,
            m_p->m_account.get(),
            &ATSIBAccount::onAccountPositionBriefReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::accountPositionReceived, m_p->m_orders.get(),
            &ATSIBOrders::onAccountPositionReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::accountPositionBriefReceived,
            m_p->m_orders.get(),
            &ATSIBOrders::onAccountPositionBriefReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::historicalDataEndReceived, this,
            &ATSIBClient::onHistoricalDataEnd,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::historicalBarReceived, this,
            &ATSIBClient::onHistoricalBarReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::historicalDataEndReceived,
            m_p->m_downloader.get(),
            &ATSIBDownloader::onHistoricalDataEnd,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::historicalBarReceived, m_p->m_downloader.get(),
            &ATSIBDownloader::onHistoricalBarReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::realTimeBarReceived, this,
            &ATSIBClient::onRealTimeBarReceived,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::openOrderReceived, m_p->m_orders.get(),
            &ATSIBOrders::onOpenOrder,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::openOrderEndReceived, m_p->m_orders.get(),
            &ATSIBOrders::onOpenOrderEnd,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::completedOrderReceived, m_p->m_orders.get(),
            &ATSIBOrders::onCompletedOrder,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::completedOrderEndReceived, m_p->m_orders.get(),
            &ATSIBOrders::onCompletedOrderEnd,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::nextValidOrderId, m_p->m_orders.get(),
            &ATSIBOrders::onNextValidOrderId,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::orderStatus, m_p->m_orders.get(),
            &ATSIBOrders::onOrderStatus,
            Qt::QueuedConnection);

    connect(m_p->m_account.get(), &ATSIBAccount::accountInfoUpdated, m_p->m_orders.get(),
            &ATSIBOrders::onAccountUpdated,
            Qt::QueuedConnection);

    connect(m_p->m_orders.get(), &ATSIBOrders::orderCompleted, m_p->m_account.get(),
            &ATSIBAccount::onOrderCompleted,
            Qt::QueuedConnection);

    connect(m_p->m_ibEvents->observer().get(), &ATSIBEventsObserver::connectionClosed, this,
            &ATSIBClient::clientDisconnected,
            Qt::QueuedConnection);
}

//----------------------------------------------------------------------------------------------------------------------
ATSIBClient::~ATSIBClient() {
    m_p->m_isReadingMsgs = false;
    m_p->m_exitReadingThread = true;
    m_p->m_nextBar = true;
    m_p->m_waitForNextBar.notify_all();

    if (m_p->m_msgThread.joinable())
        m_p->m_msgThread.join();

    if (m_p->m_pReader) {
        m_p->m_pReader.reset();
    }

    if (m_p->m_testThread.joinable()) {
        m_p->m_testThread.join();
    }
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ATSIBEvents> ATSIBClient::events() {
    return m_p->m_ibEvents;
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ATSIBOrders> ATSIBClient::orders() {
    return m_p->m_orders;
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<EClientSocket> ATSIBClient::socket() {
    return m_p->m_socket;
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ATSIBAccount> ATSIBClient::account() {
    return m_p->m_account;
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<ATSIBDownloader> ATSIBClient::downloader() {
    return m_p->m_downloader;
}

//----------------------------------------------------------------------------------------------------------------------
IBConfiguration ATSIBClient::configuration() const {
    return m_p->m_ibConfiguration;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::setConfiguration(const IBConfiguration &ibConfiguration) {
    m_p->m_ibConfiguration = ibConfiguration;
}

//----------------------------------------------------------------------------------------------------------------------
double ATSIBClient::czkToUsdRate() const {
    return m_p->m_currentCZKUSDRate;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSIBClient::connectClient() {
    std::uint16_t apiPort = 0;

    if (m_p->m_ibConfiguration.m_usingRealAccount) {
        apiPort = m_p->m_ibConfiguration.m_apiPortLive;
    } else {
        apiPort = m_p->m_ibConfiguration.m_apiPortDemo;
    }

    qInfo() << "Connecting to " << m_p->m_ibConfiguration.m_apiIP.c_str() << ":" << apiPort << ", clientId: " << 0;

    bool bRes = m_p->m_socket->eConnect(m_p->m_ibConfiguration.m_apiIP.c_str(), apiPort, 0, m_p->m_extraAuth);

    if (bRes) {
        qInfo() << "Connected to " << m_p->m_ibConfiguration.m_apiIP.c_str() << ":" << apiPort << ", clientId: "
                << 0;

        m_p->m_isReadingMsgs = false;

        if (m_p->m_msgThread.joinable())
            m_p->m_msgThread.join();

        if (m_p->m_pReader) {
            m_p->m_pReader.reset();
        }

        m_p->m_pReader = std::make_shared<EReader>(m_p->m_socket.get(), &m_p->m_osSignal);
        m_p->m_account->init();
        m_p->m_orders->init();

        m_p->m_isReadingMsgs = true;
        m_p->m_msgThread = std::thread(P::readMessages, this);

        m_p->m_pReader->start();
        emit clientConnected();

        // Get CZK/USD rate for conversion purpose
        auto *mngr = new QNetworkAccessManager(this);
        connect(mngr, &QNetworkAccessManager::finished, this, [&](QNetworkReply *reply) {
            try {
                QByteArray bts = reply->readAll();
                QString str(bts);
                auto jsonObject = nlohmann::json::parse(str.toStdString());
                m_p->m_currentCZKUSDRate = jsonObject["rates"]["CZK"];
            }
            catch (std::exception &) {
                qCritical() << "Could not parse CZK/USD rate json";
            }
        });

        mngr->get(QNetworkRequest(QUrl("https://api.exchangeratesapi.io/latest?base=USD&symbols=CZK")));

    } else {
        qCritical() << "Cannot connect to " << m_p->m_ibConfiguration.m_apiIP.c_str() << ":" << apiPort
                    << ", clientId: " << 1;
    }
    return bRes;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::unsubscribeAllData() {
    m_p->m_ibCleaner.cleanAll();
    m_p->m_realTimeDataTickers.clear();

    m_p->m_account->unsubscribeAllData();
    m_p->m_orders->unsubscribeAllData();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::disconnectClient() {
    unsubscribeAllData();

    m_p->m_socket->eDisconnect();
    qInfo() << "Client disconnected";
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSIBClient::isClientConnected() const {
    return m_p->m_socket->isConnected();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::setConnectOptions(const std::string &connectOptions) {
    m_p->m_socket->setConnectOptions(connectOptions);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::P::subscribeLiveMarketData() {
#if !defined SIMULATED_TRADING

    for (auto &contract : m_ibConfiguration.m_contracts) {
        {
            auto tickerId = TickerIdGenerator::nextId();
            m_socket->reqRealTimeBars(tickerId, contract, 5, "MIDPOINT", true, TagValueListSPtr());


            m_realTimeDataTickers.insert_or_assign(tickerId, contract.symbol);

            m_ibCleaner.m_cleaners.insert(std::pair<TickerId, std::function<void()>>(tickerId, [=]() {
                m_socket->cancelRealTimeBars(tickerId);
            }));

        }
        {
                        auto tickerId = TickerIdGenerator::nextId();
                        m_socket->reqMktData(tickerId, contract,"",false, false, TagValueListSPtr());

                        m_realTimeDataTickers.insert_or_assign(tickerId, contract.symbol);

                        m_ibCleaner.m_cleaners.insert(std::pair<TickerId, std::function<void()>>(tickerId, [=]() {
                            m_socket->cancelMktData(tickerId);
                        }));
        }

        if (m_ibConfiguration.m_saveRealTimeData) {
            std::string realTimeDataFilePath =
                    m_ibConfiguration.m_realTimeDataBaseFilePath + "/" + contract.symbol + "_realTimeData_" +
                    timeString() + ".csv";
            std::string aggregatedDataFilePath =
                    m_ibConfiguration.m_aggregatedDataBaseFilePath + "/" + contract.symbol + "_aggregatedData_" +
                    timeString() +
                    ".csv";

            std::ofstream realTimeDataFile(realTimeDataFilePath);
            std::ofstream aggregatedDataFile(aggregatedDataFilePath);

            m_realTimeDataFiles.insert_or_assign(contract.symbol, std::move(realTimeDataFile));
            m_aggregatedTimeDataFiles.insert_or_assign(contract.symbol, std::move(aggregatedDataFile));
        }

    }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::P::retrieveHistoricDataForContract(Contract contract) {
#ifndef SIMULATED_TRADING
    std::time_t rawtime;
    std::tm *timeinfo;
    char queryTime[80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);
    std::strftime(queryTime, 80, "%Y%m%d %H:%M:%S", timeinfo);

    std::string whatToShow;

    if (contract.secType == "CASH") {
        whatToShow = "MIDPOINT";
    } else {
        whatToShow = "TRADES";
    }

    auto tickerId = TickerIdGenerator::nextId();
    m_socket->reqHistoricalData(tickerId, contract, queryTime, m_ibConfiguration.m_historicDataLength,
                                m_ibConfiguration.m_historicalBarResolution,
                                whatToShow, 1, 1, false,
                                TagValueListSPtr());

    m_ibCleaner.m_cleaners.insert(std::pair<TickerId, std::function<void()>>(tickerId, [=]() {
        m_socket->cancelHistoricalData(tickerId);
    }));

#else

    m_testThread = std::thread([this]() {
        std::vector<ATSBar> bars;

        auto retVal = loadTimeSeriesFromCSV((QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                                             QString::fromStdString(m_ibConfiguration.m_demoAsset)).toStdString(),
                                            bars);

        if (retVal) {

            int numSend = 0;

            for (auto &bar : bars) {

                std::unique_lock<std::mutex> lock(m_waitMutex);

                m_waitForNextBar.wait(lock, [&] {
                    return m_allHistoricBars | m_nextBar;
                });

                if (m_exitReadingThread) {
                    return;
                }

                if (m_allHistoricBars) {
                    for (std::vector<ATSBar>::iterator i = bars.begin() + numSend; i != bars.end(); i++) {
                        emit m_ibEvents->observer().get()->historicalBarReceived(-1, *i);
                        std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    }

                    break;
                }

                m_nextBar = false;
                emit m_ibEvents->observer().get()->historicalBarReceived(-1, bar);
                numSend++;
            }
        }
    });
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::P::retrieveHistoricData() {
    m_contractsToDataRetrieve.insert(m_contractsToDataRetrieve.begin(), m_ibConfiguration.m_contracts.begin(),
                                     m_ibConfiguration.m_contracts.end());

    if (!m_contractsToDataRetrieve.empty()) {
        retrieveHistoricDataForContract(m_contractsToDataRetrieve.back());
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::onHistoricalDataEnd(TickerId tickerId) {
    if (!m_p->m_contractsToDataRetrieve.empty()) {
        m_p->m_contractsToDataRetrieve.pop_back();
    }

    if (!m_p->m_contractsToDataRetrieve.empty()) {
        m_p->retrieveHistoricDataForContract(m_p->m_contractsToDataRetrieve.back());
    }

    m_p->m_ibCleaner.clean(tickerId);

    if (m_p->m_contractsToDataRetrieve.empty()) {
        m_p->subscribeLiveMarketData();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::subscribeAllData() {
    m_p->retrieveHistoricData();
    m_p->m_orders->subscribeAllData();
    m_p->m_account->subscribeAllData();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::P::readMessages(ATSIBClient *owner) {
    while (owner->m_p->m_isReadingMsgs) {
        owner->m_p->m_osSignal.waitForSignal();
        owner->m_p->m_pReader->processMsgs();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::onHistoricalBarReceived(TickerId tickerId, const ATSBar &bar) {
    if (m_p->m_ibCleaner.exists(tickerId) && !m_p->m_contractsToDataRetrieve.empty()) {
        emit barReceived(bar, m_p->m_contractsToDataRetrieve.back().symbol);
    } else if (tickerId == -1 && !m_p->m_contractsToDataRetrieve.empty()) {
        emit barReceived(bar, m_p->m_contractsToDataRetrieve.back().symbol);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::onRealTimeBarReceived(TickerId tickerId, const ATSBar &bar) {
    for (const auto &dataTicker : m_p->m_realTimeDataTickers) {
        if (dataTicker.first == tickerId) {

#if !defined SIMULATED_TRADING
            if (m_p->m_ibConfiguration.m_saveRealTimeData) {
                auto iter = m_p->m_realTimeDataFiles.find(dataTicker.second);

                if (iter != m_p->m_realTimeDataFiles.end()) {
                    if (iter->second.is_open()) {
                        iter->second << bar.m_unixTime << "," << bar.m_open << "," << bar.m_high << "," << bar.m_low
                                     << "," << bar.m_close << bar.m_volume << "," << std::endl;
                    }
                }
            }
#endif
            emit barReceived(bar, dataTicker.second);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::nextHistoricBar() {
    m_p->m_nextBar = true;
    m_p->m_waitForNextBar.notify_all();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBClient::allHistoricBars() {
    m_p->m_allHistoricBars = true;
    m_p->m_waitForNextBar.notify_all();
}

}
