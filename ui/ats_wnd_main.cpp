#include "ats_wnd_main.h"
#include "ui_ats_wnd_main.h"
#include "ats_wdg_plots.h"
#include <QTime>
#include "ats_dlg_downloader.h"
#include "ats_dlg_optimizer.h"
#include "ats_dlg_settings.h"
#include "ats_dlg_portfolio.h"

using namespace trader;

std::vector<trader::ATSIBOrder> OrdersModel::orders() const {
    return m_orders;
}

//----------------------------------------------------------------------------------------------------------------------
void OrdersModel::addOrder(const trader::ATSIBOrder &order) {
    bool found = false;

    // Update or add
    for (auto &item : m_orders) {
        if (item.m_order.permId == order.m_order.permId) {
            item = order;
            found = true;
            break;
        }
    }
    if (!found) {
        m_orders.push_back(order);
        beginInsertRows(QModelIndex(), static_cast<int>(m_orders.size() - 1), static_cast<int>(m_orders.size()));
        endInsertRows();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void OrdersModel::setOrders(const std::vector<trader::ATSIBOrder> &orders) {
    beginRemoveRows(QModelIndex(), 0, static_cast<int>(m_orders.size()) - 1);
    m_orders.clear();
    endRemoveRows();

    m_orders = orders;
    beginInsertRows(QModelIndex(), 0, static_cast<int>(m_orders.size() - 1));
    endInsertRows();
}

//----------------------------------------------------------------------------------------------------------------------
void OrdersModel::updateOrder(OrderId orderId, double price, std::int64_t time) {
    // Update or add
    for (auto &item : m_orders) {
        if (item.m_order.permId == orderId) {
            item.m_exitPrice = price;
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
int OrdersModel::rowCount(const QModelIndex &) const {
    return static_cast<int>(m_orders.size());
}

//----------------------------------------------------------------------------------------------------------------------
int OrdersModel::columnCount(const QModelIndex &) const {
    return 8;
}

//----------------------------------------------------------------------------------------------------------------------
QVariant OrdersModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    if (index.column() == 0) {
        return index.row() + 1;
    } else if (index.column() == 1) {
        return QString::fromStdString(m_orders[static_cast<std::size_t>(index.row())].m_contract.symbol);
    } else if (index.column() == 2) {
        return QString::fromStdString(m_orders[static_cast<std::size_t>(index.row())].m_order.action);
    } else if (index.column() == 3) {
        return m_orders[static_cast<std::size_t>(index.row())].m_order.totalQuantity;
    } else if (index.column() == 4) {
        return m_orders[static_cast<std::size_t>(index.row())].m_position;
    } else if (index.column() == 5) {
        return m_orders[static_cast<std::size_t>(index.row())].m_orderStatus.m_avgFillPrice;
    } else if (index.column() == 6) {
        return m_orders[static_cast<std::size_t>(index.row())].m_exitPrice;
    } else if (index.column() == 7) {
        return m_orders[static_cast<std::size_t>(index.row())].m_orderState.commission;
    } else if (index.column() == 8) {

        auto completedTime = m_orders[static_cast<std::size_t>(index.row())].m_orderState.completedTime;
        auto dateTime = QDateTime::fromString(QString::fromStdString(completedTime), "yyyyMMdd  hh:mm:ss");
        return dateTime.time().toString();
    }

    return QVariant();
}

//----------------------------------------------------------------------------------------------------------------------
QVariant OrdersModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return QString("#");
        } else if (section == 1) {
            return QString("Symbol");
        } else if (section == 2) {
            return QString("Action");
        } else if (section == 3) {
            return QString("Quantity");
        } else if (section == 4) {
            return QString("Position");
        } else if (section == 5) {
            return QString("Enter Price");
        } else if (section == 6) {
            return QString("Exit Price");
        } else if (section == 7) {
            return QString("Commission");
        } else if (section == 8) {
            return QString("Time");
        }
    }
    return QVariant();
}

//----------------------------------------------------------------------------------------------------------------------
void PositionsModel::insertOrUpdatePosition(const trader::ATSIBPosition &position) {
    bool found = false;

    for (auto &item : m_positions) {
        if (item.m_contract.conId == position.m_contract.conId) {
            item = position;
            found = true;
            break;
        }
    }

    if (!found) {
        m_positions.push_back(position);
        beginInsertRows(QModelIndex(), static_cast<int>(m_positions.size() - 1), static_cast<int>(m_positions.size()));
        endInsertRows();
    }
}

//----------------------------------------------------------------------------------------------------------------------
int PositionsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return static_cast<int>(m_positions.size());
}

//----------------------------------------------------------------------------------------------------------------------
int PositionsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 5;
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<trader::ATSIBPosition> PositionsModel::positions() const {
    return m_positions;
}

//----------------------------------------------------------------------------------------------------------------------
QVariant PositionsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    if (index.column() == 0) {
        return index.row() + 1;
    } else if (index.column() == 1) {
        return QString::fromStdString(m_positions[static_cast<std::size_t>(index.row())].m_contract.symbol);
    } else if (index.column() == 2) {
        return m_positions[static_cast<std::size_t>(index.row())].m_position;
    } else if (index.column() == 3) {
        return m_positions[static_cast<std::size_t>(index.row())].m_realizedPNL +
               m_positions[static_cast<std::size_t>(index.row())].m_unrealizedPNL;
    } else if (index.column() == 4) {
        return QString::fromStdString(m_positions[static_cast<std::size_t>(index.row())].m_contract.currency);
    }

    return QVariant();
}

//----------------------------------------------------------------------------------------------------------------------
QVariant PositionsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        if (section == 0) {
            return QString("#");
        } else if (section == 1) {
            return QString("Symbol");
        } else if (section == 2) {
            return QString("Position");
        } else if (section == 3) {
            return QString("Profit & Loss");
        } else if (section == 4) {
            return QString("Currency");
        }
    }
    return QVariant();
}

//----------------------------------------------------------------------------------------------------------------------
struct ATSWndMain::P {
    Ui::ATSWndMain *m_ui{};
    ATSWndMain *m_parent = nullptr;
    std::string m_statusString;
    bool m_isConnected = false;
    std::shared_ptr<trader::ATSConfiguration> m_config;
    ATSCompletedOrders m_completedOrders;
    PositionsModel *m_positionsModel = nullptr;
    OrdersModel *m_ordersModel = nullptr;
    QTimer m_mainTimer;
    QMessageBox *m_connectionMsgBox = nullptr;
    ATSDlgOptimizer *m_dlgOptimizer = nullptr;
    bool m_macdLong = false;
    bool m_macdShort = false;
    bool m_aroonLong = false;
    bool m_aroonShort = false;
    bool m_adxFilterOn = false;

    explicit P(ATSWndMain *parent) {
        m_parent = parent;
        m_mainTimer.start(1000);
    }

    ~P() {
        delete m_ui;
    }

    void updateStatus();

    void computeContinuousProfit(double &netProfit, double &grossProfit, double &commision) const;

    void switchContentOfOrdersTable(const std::string &symbol);

    void checkIfOpen(const QTime &time);
};

//----------------------------------------------------------------------------------------------------------------------
ATSWndMain::ATSWndMain(QWidget *parent) : QMainWindow(parent), m_p(spimpl::make_unique_impl<P>(this)) {
    m_p->m_ui = new Ui::ATSWndMain;
    m_p->m_ui->setupUi(this);

    m_p->m_ui->actionData_Downloader->setEnabled(false);

    {
        m_p->m_positionsModel = new PositionsModel(this);

        m_p->m_ui->tableViewPositions->setModel(m_p->m_positionsModel);
        m_p->m_ui->tableViewPositions->resizeColumnsToContents();

        QHeaderView *verticalHeader = m_p->m_ui->tableViewPositions->verticalHeader();
        verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
        verticalHeader->setDefaultSectionSize(20);

        m_p->m_ui->tableViewPositions->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_p->m_ui->tableViewPositions->setSelectionMode(QAbstractItemView::NoSelection);
    }

    {
        m_p->m_ordersModel = new OrdersModel(this);

        m_p->m_ui->tableViewOrders->setModel(m_p->m_ordersModel);
        m_p->m_ui->tableViewOrders->resizeColumnsToContents();

        QHeaderView *verticalHeader = m_p->m_ui->tableViewOrders->verticalHeader();
        verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
        verticalHeader->setDefaultSectionSize(20);

        m_p->m_ui->tableViewOrders->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_p->m_ui->tableViewOrders->setSelectionMode(QAbstractItemView::NoSelection);
    }

    m_p->m_ui->tabWidgetPlots->clear();

    connect(m_p->m_ui->tabWidgetPlots, &QTabWidget::currentChanged, this, [this](int index) {
        m_p->m_ui->groupBoxOrders->setTitle("Orders: " + m_p->m_ui->tabWidgetPlots->tabText(index));
        m_p->switchContentOfOrdersTable(m_p->m_ui->tabWidgetPlots->tabText(index).toStdString());
    });

    connect(&m_p->m_mainTimer, &QTimer::timeout, this, [this]() {
        auto time = QTime::currentTime();
        m_p->m_ui->labelTime->setText(time.toString());
        m_p->checkIfOpen(time);
    });

    connect(m_p->m_ui->actionData_Downloader, &QAction::triggered, this, [&]() {
        auto dlg = new ATSDlgDownloader(m_p->m_config->m_ibConfiguration.m_historicalBarResolution, this);
        connect(dlg, &ATSDlgDownloader::downloadTimeSeriesRequest, this, &ATSWndMain::downloadTimeSeriesRequest);
        dlg->show();
    });

    connect(m_p->m_ui->actionIndicators_Optimizer, &QAction::triggered, this, [&]() {
        m_p->m_dlgOptimizer->show();
    });

    connect(m_p->m_ui->actionSettings, &QAction::triggered, this, [&]() {
        auto dlg = new ATSDlgSettings(*m_p->m_config, this);
        dlg->show();
    });

    connect(m_p->m_ui->actionPortfolio, &QAction::triggered, this, [&]() {
        auto dlg = new ATSDlgPortfolio(*m_p->m_config, this);
        dlg->show();
    });

    connect(m_p->m_ui->pushButtonBuy, &QPushButton::clicked, this, [&]() {
        emit buy();
    });

    connect(m_p->m_ui->pushButtonSell, &QPushButton::clicked, this, [&]() {
        emit sell();
    });

    connect(m_p->m_ui->pushButtonInvert, &QPushButton::clicked, this, [&]() {
        emit invert();
    });

    connect(m_p->m_ui->actionExit, &QAction::triggered, this, &QMainWindow::close);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::setConnected(bool isConnected) {
    m_p->m_isConnected = isConnected;
    m_p->updateStatus();

    if (!isConnected) {
        m_p->m_connectionMsgBox = new QMessageBox(this);
        m_p->m_connectionMsgBox->setText("IB API Disconnected!");
        m_p->m_connectionMsgBox->setIcon(QMessageBox::Warning);
        m_p->m_connectionMsgBox->setStandardButtons(QMessageBox::Ok);
        m_p->m_connectionMsgBox->show();

    } else {
        if (m_p->m_connectionMsgBox) {
            m_p->m_connectionMsgBox->close();
            delete m_p->m_connectionMsgBox;
            m_p->m_connectionMsgBox = nullptr;
        }

        m_p->m_ui->actionData_Downloader->setEnabled(isConnected);
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::closeEvent(QCloseEvent *event)  // show prompt when user wants to close app
{
    event->ignore();
    if (QMessageBox::Yes == QMessageBox::question(this, qAppName(), "Do you really want to exit the Trader?",
                                                  QMessageBox::Yes | QMessageBox::No)) {
        event->accept();
    }

}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::P::updateStatus() {
    std::string accountType;
    std::int32_t port;

    if (m_config->m_ibConfiguration.m_usingRealAccount) {
        accountType = "REAL";
        port = m_config->m_ibConfiguration.m_apiPortLive;
    } else {
        accountType = "DEMO";
        port = m_config->m_ibConfiguration.m_apiPortDemo;
    }

    if (m_isConnected) {
        m_statusString =
                "Connected to IB API, account type: " + accountType + ", host: " + m_config->m_ibConfiguration.m_apiIP +
                ", port: " + std::to_string(
                        port);
    } else {
        m_statusString = "Not connected to IB API";
    }

    std::string tradeVariablesStatus;

    if (m_macdLong) {
        tradeVariablesStatus = "macd long = true";
    } else {
        tradeVariablesStatus = "macd long = false";
    }

    if (m_macdShort) {
        tradeVariablesStatus = tradeVariablesStatus + ", macd short = true";
    } else {
        tradeVariablesStatus = tradeVariablesStatus + ", macd short = false";
    }

    if (m_aroonLong) {
        tradeVariablesStatus = tradeVariablesStatus + ", aroon long = true";
    } else {
        tradeVariablesStatus = tradeVariablesStatus + ", aroon long = false";
    }

    if (m_aroonShort) {
        tradeVariablesStatus = tradeVariablesStatus + ", aroon short = true";
    } else {
        tradeVariablesStatus = tradeVariablesStatus + ", aroon short = false";
    }

    if (m_adxFilterOn) {
        tradeVariablesStatus = tradeVariablesStatus + ", adx filter on = true";
    } else {
        tradeVariablesStatus = tradeVariablesStatus + ", adx filter on = false";
    }

    m_statusString = m_statusString + ", variables: " + tradeVariablesStatus;

    m_parent->statusBar()->showMessage(QString::fromStdString(m_statusString));
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::P::computeContinuousProfit(double &netProfit, double &grossProfit, double &commision) const {
    double profit = 0;

    for (const auto &order : m_ordersModel->orders()) {
        if (order.m_exitPrice <= 0) {
            break;
        }

        if (order.m_order.action == "BUY") {
            profit += (order.m_exitPrice - order.m_orderStatus.m_avgFillPrice) * std::abs(order.m_position);
        } else {
            profit += (order.m_orderStatus.m_avgFillPrice - order.m_exitPrice) * std::abs(order.m_position);
        }
    }

    for (std::size_t i = 0; i < m_ordersModel->orders().size(); i++) {
        commision += m_ordersModel->orders()[i].m_orderState.commission;
    }

    grossProfit = profit;
    netProfit = grossProfit - commision;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::P::switchContentOfOrdersTable(const std::string &symbol) {
    std::vector<ATSIBOrder> selectedOrders;
    for (const auto &order : m_completedOrders.m_orders) {

        if (order.second.m_contract.symbol == symbol) {
            selectedOrders.push_back(order.second);
        }
    }

    m_ordersModel->setOrders(selectedOrders);

    double netProfit = 0;
    double grossProfit = 0;
    double commision = 0;

    computeContinuousProfit(netProfit, grossProfit, commision);

    m_ui->labelPLTotalSingleSymbolNet->setText(QString::number(netProfit));
    m_ui->labelPLTotalSingleSymbolGross->setText(QString::number(grossProfit));
    m_ui->labelCommisionTotalSingleSymbol->setText(QString::number(commision));
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::P::checkIfOpen(const QTime &time) {
    //    if(!m_config) {
    //        return;
    //    }

    //    bool weekend = (QDate::currentDate().day() == 6 || QDate::currentDate().day() == 7);

    //    auto openTime = QTime::fromString(m_config->m_ibConfiguration.marketOpens().c_str(), "hh:mm:ss");
    //    auto closeTime = QTime::fromString(m_config->m_ibConfiguration.marketCloses().c_str(), "hh:mm:ss");

    //    if(time > openTime && time < closeTime/* && !weekend*/) {
    // m_ui->labelOpenCloseState->setStyleSheet("color: rgb(0, 255, 0);");
    //m_ui->labelOpenCloseState->setText("Now open");
    //    }
    //    else {
    //        m_ui->labelOpenCloseState->setStyleSheet("color: rgb(255, 0, 0);");
    //        m_ui->labelOpenCloseState->setText("Now closed");
    //    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::setCompletedOrders(const trader::ATSCompletedOrders &completedOrders) {
    m_p->m_completedOrders = completedOrders;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::setConfiguration(const trader::ATSConfiguration &configuration) {
    bool reCreateTabs = false;

    std::vector<std::string> currentConfigContracts;
    std::vector<std::string> newConfigContracs;

    if (m_p->m_config) {
        for (auto &contract : m_p->m_config->m_ibConfiguration.m_contracts) {
            currentConfigContracts.push_back(contract.symbol);
        }

        for (auto &contract : configuration.m_ibConfiguration.m_contracts) {
            newConfigContracs.push_back(contract.symbol);
        }

        if (currentConfigContracts != newConfigContracs) {
            reCreateTabs = true;
        }
    } else {
        m_p->m_config = std::make_shared<trader::ATSConfiguration>();
        reCreateTabs = true;
    }

    *m_p->m_config = configuration;
    m_p->updateStatus();

    if (reCreateTabs) {
        m_p->m_ui->tabWidgetPlots->clear();

        for (auto &contract : m_p->m_config->m_ibConfiguration.m_contracts) {
            ATSWDGPlots *plotWdg = new ATSWDGPlots();
            plotWdg->setResolutions(m_p->m_config->m_ibConfiguration.m_activeResolutions);
            plotWdg->setContract(contract);
            plotWdg->setIndicators(*m_p->m_config);
            m_p->m_ui->tabWidgetPlots->addTab(plotWdg, QString::fromStdString(contract.symbol));
        }
    }

    m_p->m_ui->labelOpenHours->setText(
            QString::fromStdString(configuration.m_ibConfiguration.marketOpens()) + " - " + QString::fromStdString(
                    configuration.m_ibConfiguration.marketCloses()));

    delete m_p->m_dlgOptimizer;

    m_p->m_dlgOptimizer = new ATSDlgOptimizer(m_p->m_config->m_ibConfiguration.m_downloaderBaseFilePath, (this));
    connect(m_p->m_dlgOptimizer, &ATSDlgOptimizer::run, this, &ATSWndMain::runOptimizer);
    connect(m_p->m_dlgOptimizer, &ATSDlgOptimizer::stop, this, &ATSWndMain::stopOptimizer);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::addBar(const ATSBar &bar, const std::string &symbol) {
    QDateTime barDateTime = QDateTime::fromSecsSinceEpoch(bar.m_unixTime);

    m_p->m_ui->labelOpenCloseState->setText(barDateTime.time().toString());

    for (auto i = 0; i < m_p->m_ui->tabWidgetPlots->count(); i++) {
        if (m_p->m_ui->tabWidgetPlots->tabText(i).toStdString() == symbol) {
            ATSWDGPlots *plotWdg = qobject_cast<ATSWDGPlots *>(m_p->m_ui->tabWidgetPlots->widget(i));
            plotWdg->addBar(bar);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::addProcessedBar(const ATSSignals &procesedBar, const std::string &symbol) {
    for (auto i = 0; i < m_p->m_ui->tabWidgetPlots->count(); i++) {
        if (m_p->m_ui->tabWidgetPlots->tabText(i).toStdString() == symbol) {
            ATSWDGPlots *plotWdg = qobject_cast<ATSWDGPlots *>(m_p->m_ui->tabWidgetPlots->widget(i));
            plotWdg->addProcessedBar(procesedBar.m_signalPacks);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onOrderCompleted(const trader::ATSIBOrder &order) {
    if (order.m_contract.symbol ==
        m_p->m_ui->tabWidgetPlots->tabText(m_p->m_ui->tabWidgetPlots->currentIndex()).toStdString()) {
        m_p->m_ordersModel->addOrder(order);
        double netProfit = 0;
        double grossProfit = 0;
        double commision = 0;
        m_p->computeContinuousProfit(netProfit, grossProfit, commision);
        m_p->m_ui->labelPLTotalSingleSymbolNet->setText(QString::number(netProfit));
        m_p->m_ui->labelPLTotalSingleSymbolGross->setText(QString::number(grossProfit));
        m_p->m_ui->labelCommisionTotalSingleSymbol->setText(QString::number(commision));
    }

    for (auto i = 0; i < m_p->m_ui->tabWidgetPlots->count(); i++) {
        if (m_p->m_ui->tabWidgetPlots->tabText(i).toStdString() == order.m_contract.symbol) {
            ATSWDGPlots *plotWdg = qobject_cast<ATSWDGPlots *>(m_p->m_ui->tabWidgetPlots->widget(i));
            plotWdg->addOrderMark(order);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onCloseTrading(const trader::ATSOrderRequest &orderRequest) {
    for (auto i = 0; i < m_p->m_ui->tabWidgetPlots->count(); i++) {
        if (m_p->m_ui->tabWidgetPlots->tabText(i).toStdString() == orderRequest.symbol()) {
            ATSWDGPlots *plotWdg = qobject_cast<ATSWDGPlots *>(m_p->m_ui->tabWidgetPlots->widget(i));
            plotWdg->addTradingClosedMark(orderRequest);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onAccountInfoUpdated(const trader::ATSIBAccountInfo &accountInfo) {
    m_p->m_ui->groupBoxAccount->setTitle("Account - " + QString::fromStdString(accountInfo.m_account));
    m_p->m_ui->labelCurrency->setText(QString::fromStdString(accountInfo.m_currency));
    m_p->m_ui->labelAccountTotalCashbalance->setText(QString::number(accountInfo.m_totalCashBalance, 'g', 10));
    m_p->m_ui->labelAccountCashBalance->setText(QString::number(accountInfo.m_cashBalance, 'g', 10));
    m_p->m_ui->labelAccountAccruedCash->setText(QString::number(accountInfo.m_accruedCash, 'g', 10));
    m_p->m_ui->labelAccountSettledCash->setText(QString::number(accountInfo.m_settledCash, 'g', 10));
    m_p->m_ui->labelAccountRealized->setText(QString::number(accountInfo.m_realizedPNL, 'g', 10));
    m_p->m_ui->labelAccountUnrealized->setText(QString::number(accountInfo.m_unrealizedPNL, 'g', 10));
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onAccountPositionUpdated(const trader::ATSIBPosition &position) {
    m_p->m_positionsModel->insertOrUpdatePosition(position);

    double totalRealized = 0.0;
    double totalUnrealized = 0.0;

    std::string currency;
    for (const auto &item : m_p->m_positionsModel->positions()) {
        totalRealized += item.m_realizedPNL;
        totalUnrealized += item.m_unrealizedPNL;
        currency = item.m_contract.currency;
    }

    m_p->m_ui->labelAccountUnrealized->setText(
            QString::number(totalUnrealized) + " " + QString::fromStdString(currency));
    m_p->m_ui->labelAccountRealized->setText(QString::number(totalRealized) + " " + QString::fromStdString(currency));

    if (totalUnrealized > 0) {
        m_p->m_ui->labelAccountUnrealized->setStyleSheet("color: rgb(0, 255, 0);");
    } else if (totalUnrealized < 0) {
        m_p->m_ui->labelAccountUnrealized->setStyleSheet("color: rgb(255, 0, 0);");
    }

    if (totalRealized > 0) {
        m_p->m_ui->labelAccountRealized->setStyleSheet("color: rgb(0, 255, 0);");
    } else if (totalRealized < 0) {
        m_p->m_ui->labelAccountRealized->setStyleSheet("color: rgb(255, 0, 0);");
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::showMessage(const QMessageBox::Icon &icon, const std::string &message) {
    auto msgBox = new QMessageBox(icon, qAppName(), QString::fromStdString(message), QMessageBox::Ok);
    msgBox->show();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onOptimizing(bool running, const std::string &historicalDataFilePath) {
    m_p->m_dlgOptimizer->onOptimizing(running, historicalDataFilePath);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onTrendVariables(bool macdLong, bool macdShort, bool aroonLong, bool aroonShort, bool adxFilterOn) {
    m_p->m_macdLong = macdLong;
    m_p->m_macdShort = macdShort;
    m_p->m_aroonLong = aroonLong;
    m_p->m_aroonShort = aroonShort;
    m_p->m_adxFilterOn = adxFilterOn;

    m_p->updateStatus();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onPositionExited(const trader::ATSOrderRequest &orderRequest) {
    m_p->m_ordersModel->updateOrder(orderRequest.orderId(), orderRequest.exitPrice(), orderRequest.exitTime());

    for (auto i = 0; i < m_p->m_ui->tabWidgetPlots->count(); i++) {
        if (m_p->m_ui->tabWidgetPlots->tabText(i).toStdString() == orderRequest.symbol()) {
            ATSWDGPlots *plotWdg = qobject_cast<ATSWDGPlots *>(m_p->m_ui->tabWidgetPlots->widget(i));
            plotWdg->addMarker(orderRequest.exitPrice(), orderRequest.exitTime(), orderRequest.symbol());
        }
    }

    double netProfit = 0;
    double grossProfit = 0;
    double commision = 0;

    m_p->computeContinuousProfit(netProfit, grossProfit, commision);

    m_p->m_ui->labelPLTotalSingleSymbolNet->setText(QString::number(netProfit));
    m_p->m_ui->labelPLTotalSingleSymbolGross->setText(QString::number(grossProfit));
    m_p->m_ui->labelCommisionTotalSingleSymbol->setText(QString::number(commision));
}

//----------------------------------------------------------------------------------------------------------------------
void ATSWndMain::onCurrentProfit(const std::string &symbol, double profit) {
    m_p->m_ui->labelCurrentOrderPnL->setText(QString::number(profit));
}
