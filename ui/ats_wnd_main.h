#ifndef ATS_MAIN_WND_H
#define ATS_MAIN_WND_H

#include <QMainWindow>
#include "thirdparty/spimpl.hpp"
#include "model/ats_configuration.h"
#include "thirdparty/IB/client/CommonDefs.h"
#include "model/processing/ats_processing_cluster.h"
#include "model/processing/ats_processor.h"
#include "thirdparty/qcustomplot.h"
#include "model/ats_order_request.h"
#include "model/IB/ats_ib_account.h"

//---------------------------------------------------------------------------------------------------------------------
class OrdersModel : public QAbstractTableModel {
Q_OBJECT
    std::vector<trader::ATSIBOrder> m_orders;

public:
    OrdersModel(QObject *) {}

    std::vector<trader::ATSIBOrder> orders() const;

    void addOrder(const trader::ATSIBOrder &orders);

    void setOrders(const std::vector<trader::ATSIBOrder> &orderRequest);

    void updateOrder(OrderId orderId, double price, std::int64_t time);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};

//---------------------------------------------------------------------------------------------------------------------
class PositionsModel : public QAbstractTableModel {
Q_OBJECT
    std::vector<trader::ATSIBPosition> m_positions;

public:
    PositionsModel(QObject *) {}

    std::vector<trader::ATSIBPosition> positions() const;

    void insertOrUpdatePosition(const trader::ATSIBPosition &position);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
};


//---------------------------------------------------------------------------------------------------------------------
class ATSWndMain : public QMainWindow {
Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    explicit ATSWndMain(QWidget *parent = nullptr);

public slots:

    void setConnected(bool isConnected);

    void setConfiguration(const trader::ATSConfiguration &configuration);

    void setCompletedOrders(const trader::ATSCompletedOrders &completedOrders);

    void addBar(const trader::ATSBar &bar, const std::string &symbol);

    void addProcessedBar(const trader::ATSSignals &, const std::string &symbol);

    void onOrderCompleted(const trader::ATSIBOrder &order);

    void onCloseTrading(const trader::ATSOrderRequest &order);

    void onAccountInfoUpdated(const trader::ATSIBAccountInfo &accountInfo);

    void onAccountPositionUpdated(const trader::ATSIBPosition &position);

    void showMessage(const QMessageBox::Icon &icon, const std::string &message);

    void onOptimizing(bool running, const std::string &historicalDataFilePath);

    void onTrendVariables(bool macdLong, bool macdShort, bool aroonLong, bool aroonShort, bool adxFilterOn);

    void onPositionExited(const trader::ATSOrderRequest &orderRequest);

    void onCurrentProfit(const std::string &symbol, double profit);

signals:

    void configurationChanged(const trader::ATSConfiguration &configuration);

    void downloadTimeSeriesRequest(const std::string &symbol, const std::string &resolution,
                                   const std::string &historicDataLength);

    void runOptimizer(const std::string &historicalDataFilePath);

    void stopOptimizer();

    void buy();

    void sell();

    void invert();
};

#endif // ATS_MAIN_WND_H
