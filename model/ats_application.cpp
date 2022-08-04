#include "ats_application.h"
#include "model/IB/ats_ib_client.hpp"
#include <QStandardPaths>
#include "ats_configuration.h"
#include <QPalette>
#include <QStyleFactory>
#include "model/processing/ats_processing_cluster.h"
#include "model/processing/ats_processor.h"
#include "model/ats_optimizer.h"
#include "thirdparty/IB/client/CommonDefs.h"
#include "model/IB/ats_ib_orders.h"
#include "model/IB/ats_ib_downloader.h"
#include "model/ats_db_client.h"
#include <QTimer>
#include <QDateTime>
#include <QDir>
#include <QKeyEvent>
#include <QMessageBox>

namespace trader {

struct ATSApplication::P {

    ATSApplication *m_parent = nullptr;
    std::shared_ptr<ATSIBClient> m_ibClient;
    std::shared_ptr<ATSDBClient> m_dbClient;
    QString m_configPath;
    ATSConfiguration m_config;
    ATSCompletedOrders m_completedOrders;
    QString m_completedOrdersPath;
    std::shared_ptr<ATSProcessingCluster> m_processingCluster;
    std::shared_ptr<ATSOptimizer> m_optimizer;

    P(ATSApplication *parent) {
        m_parent = parent;
    }
};

//---------------------------------------------------------------------------------------------------------------------
ATSApplication::ATSApplication(int &argc, char **argv) : QApplication(argc, argv),
                                                         m_p(spimpl::make_unique_impl<P>(this)) {
    setStyle(QStyleFactory::create("Fusion"));

    setApplicationName("Trader");

    // modify palette to dark
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
    darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

    setPalette(darkPalette);

    qRegisterMetaType<QMessageBox::Icon>("QMessageBox::Icon");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<Contract>("Contract");
    qRegisterMetaType<ATSConfiguration>("ATSConfiguration");
    qRegisterMetaType<ATSCompletedOrders>("ATSCompletedOrders");
    qRegisterMetaType<IBConfiguration>("IBConfiguration");
    qRegisterMetaType<ATSBar>("ATSBar");
    qRegisterMetaType<std::vector<ATSBar>>("std::vector<ATSBar>");
    qRegisterMetaType<TickerId>("TickerId");
    qRegisterMetaType<ATSSignalPack>("ATSSignalPack");
    qRegisterMetaType<ATSSignals>("ATSSignals");
    qRegisterMetaType<ATSOrderRequest>("ATSOrderRequest");
    qRegisterMetaType<ATSIBOrderStatus>("ATSIBOrderStatus");
    qRegisterMetaType<Order>("Order");
    qRegisterMetaType<OrderState>("OrderState");
    qRegisterMetaType<ATSIBOrder>("ATSIBOrder");
    qRegisterMetaType<ATSIBPositionBrief>("ATSIBPositionBrief");
    qRegisterMetaType<ATSIBPosition>("ATSIBPosition");
    qRegisterMetaType<ATSIBAccountInfo>("ATSIBAccountInfo");
    qRegisterMetaType<std::map<std::string, ATSIBPosition>>("std::map<std::string, ATSIBPosition>");
    qRegisterMetaType<std::int64_t>("std::int64_t");

    installEventFilter(this);
}

//---------------------------------------------------------------------------------------------------------------------
ATSApplication::~ATSApplication() {

    if(m_p->m_processingCluster) {
        auto order = m_p->m_processingCluster->activeOrderRequest("EUR", BarResolution::Sec30);

        if (order) {
            m_p->m_config.m_ibConfiguration.m_activeOrder = order;
        }
    }

    m_p->m_config.save(m_p->m_configPath.toStdString());
    m_p->m_completedOrders.save(m_p->m_completedOrdersPath.toStdString());
}

//---------------------------------------------------------------------------------------------------------------------
std::string ATSApplication::init() {
    auto rootDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(rootDir);

    if (!QDir(rootDirPath).exists()) {
        QDir().mkdir(rootDirPath);
    }

    m_p->m_configPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(rootDir) +
                        QString("/config.json");

    m_p->m_completedOrdersPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QString(
            rootDir) + QString("/completed_orders.json");

    if (!m_p->m_config.load(m_p->m_configPath.toStdString())) {
        return "Could not read configuration file";
    }

    if (!m_p->m_completedOrders.load(m_p->m_completedOrdersPath.toStdString())) {
        return "Could not read completed orders file";
    }

    if (!QDir(QString::fromStdString(m_p->m_config.m_ibConfiguration.m_realTimeDataBaseFilePath)).exists()) {
        QDir().mkdir(QString::fromStdString(m_p->m_config.m_ibConfiguration.m_realTimeDataBaseFilePath));
    }

    if (!QDir(QString::fromStdString(m_p->m_config.m_ibConfiguration.m_downloaderBaseFilePath)).exists()) {
        QDir().mkdir(QString::fromStdString(m_p->m_config.m_ibConfiguration.m_downloaderBaseFilePath));
    }

    if (!QDir(QString::fromStdString(m_p->m_config.m_ibConfiguration.m_optimizerBaseFilePath)).exists()) {
        QDir().mkdir(QString::fromStdString(m_p->m_config.m_ibConfiguration.m_optimizerBaseFilePath));
    }

#ifdef SIMULATED_TRADING
    m_p->m_completedOrders.m_orders.clear();
#else
    auto it = m_p->m_completedOrders.m_orders.begin();

    for (; it != m_p->m_completedOrders.m_orders.end();) {
        auto dateTime = QDateTime::fromString(QString::fromStdString(it->second.m_orderState.completedTime),
                                              "yyyyMMdd  hh:mm:ss");
        auto now = QDateTime::currentDateTime();

        // Erase all orders older then 24 hours
        if (dateTime < now.addSecs(-1 * 60 * 60 * 24)) {
            it = m_p->m_completedOrders.m_orders.erase(it);
        } else {
            ++it;
        }
    }
#endif

    m_p->m_ibClient = std::make_shared<ATSIBClient>(nullptr);
    m_p->m_processingCluster = std::make_shared<ATSProcessingCluster>(nullptr);
    m_p->m_optimizer = std::make_shared<ATSOptimizer>(m_p->m_config.m_ibConfiguration.m_optimizerBaseFilePath,
                                                      this);
    m_p->m_dbClient = std::make_shared<ATSDBClient>();

    if (!m_p->m_processingCluster->init(m_p->m_config)) {
        return "Could not initialize processing";
    }

    if (!m_p->m_dbClient->init()) {
        return "Could not initialize DB client";
    }

#ifndef SIMULATED_TRADING
    if (m_p->m_config.m_ibConfiguration.m_activeOrder) {

        if (QMessageBox::Yes == QMessageBox::question(qApp->activeWindow(), qAppName(),
                                                      "Active Order from last run! Do you want to load it?",
                                                      QMessageBox::Yes | QMessageBox::No)) {
            m_p->m_processingCluster->loadActiveOrderRequest("EUR", BarResolution::Sec30,
                                                             *m_p->m_config.m_ibConfiguration.m_activeOrder);
        }

        if (QMessageBox::Yes == QMessageBox::question(qApp->activeWindow(), qAppName(),
                                                      "Do you want to delete Active Order from last run?",
                                                      QMessageBox::Yes | QMessageBox::No)) {

            m_p->m_config.m_ibConfiguration.m_activeOrder = nullptr;
        }
    }

    if (m_p->m_config.m_ibConfiguration.m_tradingHalted) {
        if (QMessageBox::Yes == QMessageBox::question(qApp->activeWindow(), qAppName(),
                                                      "Trading is halted, orders will no be executed. Enable trading?",
                                                      QMessageBox::Yes | QMessageBox::No)) {
            m_p->m_config.m_ibConfiguration.m_tradingHalted = false;
            m_p->m_config.save(m_p->m_configPath.toStdString());
        }
    }
#endif

    connect(m_p->m_ibClient.get(), &ATSIBClient::clientConnected, this, &ATSApplication::onClientConnected,
            Qt::QueuedConnection);
    connect(m_p->m_ibClient.get(), &ATSIBClient::clientDisconnected, this, &ATSApplication::onClientDisconnected,
            Qt::QueuedConnection);

    connect(m_p->m_ibClient->downloader().get(), &ATSIBDownloader::message, this,
            [&](const QMessageBox::Icon &icon, const std::string &msg) {
                emit message(icon, msg);
            });

    connect(m_p->m_optimizer.get(), &ATSOptimizer::message, this,
            [&](const QMessageBox::Icon &icon, const std::string &msg) {
                emit message(icon, msg);
            });

    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::orderRequest, this,
            &ATSApplication::orderRequest, Qt::QueuedConnection);
    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::stopTrading, this, &ATSApplication::stopTrading,
            Qt::QueuedConnection);
    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::orderRequest, m_p->m_ibClient->orders().get(),
            &ATSIBOrders::onOrderRequest,
            Qt::QueuedConnection);
    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::stopTrading, m_p->m_ibClient->orders().get(),
            &ATSIBOrders::onStopTrading,
            Qt::QueuedConnection);

    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::trendVariables, this,
            &ATSApplication::trendVariables,
            Qt::QueuedConnection);

    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::exitRequest, m_p->m_ibClient->orders().get(),
            &ATSIBOrders::onExitRequest,
            Qt::QueuedConnection);

    connect(m_p->m_processingCluster.get(), &ATSProcessingCluster::currentProfit, this,
            &ATSApplication::currentProfit,
            Qt::QueuedConnection);

    connect(m_p->m_ibClient->account().get(), &ATSIBAccount::accountInfoUpdated, this,
            &ATSApplication::accountInfoUpdated, Qt::QueuedConnection);
    connect(m_p->m_ibClient->account().get(), &ATSIBAccount::accountPositionUpdated, this,
            &ATSApplication::accountPositionUpdated, Qt::QueuedConnection);

    connect(m_p->m_ibClient.get(), &ATSIBClient::barReceived, this, &ATSApplication::onReceiveBar,
            Qt::QueuedConnection);
    connect(m_p->m_optimizer.get(), &ATSOptimizer::optimizing, this, &ATSApplication::optimizing,
            Qt::QueuedConnection);

    connect(m_p->m_ibClient->orders().get(), &ATSIBOrders::orderCompleted, this, [&](const ATSIBOrder &order) {
        m_p->m_completedOrders.m_orders.insert_or_assign(order.m_order.permId, order);
        emit configurationChanged(m_p->m_config);
        emit orderCompleted(order);
    }, Qt::QueuedConnection);

    connect(m_p->m_ibClient->orders().get(), &ATSIBOrders::positionExited, this, &ATSApplication::positionExited,
            Qt::QueuedConnection);

    m_p->m_ibClient->setConfiguration(m_p->m_config.m_ibConfiguration);
    emit configurationChanged(m_p->m_config);
    emit completedOrdersChanged(m_p->m_completedOrders);
    m_p->m_ibClient->connectClient();

#ifdef OFFLINE_MODE
    m_p->m_ibClient->subscribeAllData();
#endif

    return "";
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onClientConnected() {
    emit connected(true);
    m_p->m_ibClient->subscribeAllData();
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onClientDisconnected() {
    emit connected(false);
    m_p->m_ibClient->unsubscribeAllData();
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onSetConfiguration(const ATSConfiguration &configuration) {
    m_p->m_config = configuration;
    m_p->m_ibClient->setConfiguration(configuration.m_ibConfiguration);
    m_p->m_config.save(m_p->m_configPath.toStdString());
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onReceiveBar(const ATSBar &bar, const std::string &symbol) {
    emit barReceived(bar, symbol);
    auto outputs = m_p->m_processingCluster->process(bar, symbol);
    emit barProcessed(outputs, symbol);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onDownloadTimeSeriesRequest(const std::string &symbol, const std::string &resolution,
                                                 const std::string &historicDataLength) {
    m_p->m_ibClient->downloader()->downloadDataForSymbol(symbol, resolution, historicDataLength);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onRunOptimizer(const std::string &historicalDataFilePath) {
    m_p->m_optimizer->run(historicalDataFilePath);
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onStopOptimizer() {
    m_p->m_optimizer->stop();
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onBuy() {
    m_p->m_ibClient->orders()->buy();
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onSell() {
    m_p->m_ibClient->orders()->sell();
}

//---------------------------------------------------------------------------------------------------------------------
void ATSApplication::onInvert() {
    m_p->m_ibClient->orders()->invert();
}

//---------------------------------------------------------------------------------------------------------------------
bool ATSApplication::eventFilter(QObject *obj, QEvent *event) {
    // normal event handling
    bool ret = QObject::eventFilter(obj, event);

    if (event->type() == QEvent::KeyPress) {
        auto keyEvent = dynamic_cast<QKeyEvent *>(event);

        if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_Space) {
            m_p->m_ibClient->allHistoricBars();
            return true;
        } else if (keyEvent->key() == Qt::Key_Space) {
            m_p->m_ibClient->nextHistoricBar();
            return true;
        }
    }

    return ret;
}
}
