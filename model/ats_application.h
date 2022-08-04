#ifndef ATS_APPLICATION_H
#define ATS_APPLICATION_H

#include <thirdparty/spimpl.hpp>
#include <QApplication>
#include <model/ats_configuration.h>
#include "thirdparty/IB/client/CommonDefs.h"
#include "model/processing/ats_processor.h"
#include "model/IB/ats_ib_account.h"
#include <QMessageBox>

namespace trader {
class ATSApplication : public QApplication {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

    bool eventFilter(QObject *obj, QEvent *event) override;

public:
    ATSApplication(int &argc, char **argv);

    ~ATSApplication() override;

    std::string init();

signals:

    void connected(bool);

    void configurationChanged(const ATSConfiguration &config);

    void completedOrdersChanged(const ATSCompletedOrders &completedOrders);

    void barReceived(const ATSBar &bar, const std::string &symbol);

    void barProcessed(const ATSSignals &, const std::string &symbol);

    void orderRequest(const ATSOrderRequest &orderRequest);

    void stopTrading(const ATSOrderRequest &orderRequest);

    void accountInfoUpdated(const ATSIBAccountInfo &accountInfo);

    void accountPositionUpdated(const ATSIBPosition &position);

    void orderCompleted(const ATSIBOrder &order);

    void message(const QMessageBox::Icon &icon, const std::string &message);

    void optimizing(bool running, const std::string &historicalDataFilePath);

    void trendVariables(bool macdLong, bool macdShort, bool aroonLong, bool aroonShort, bool adxFilterOn);

    void positionExited(const ATSOrderRequest &orderRequest);

    void currentProfit(const std::string &symbol, double profit);

public slots:

    void onSetConfiguration(const ATSConfiguration &configuration);

    void onReceiveBar(const ATSBar &bar, const std::string &symbol);

    void onClientConnected();

    void onClientDisconnected();

    void onDownloadTimeSeriesRequest(const std::string &symbol, const std::string &resolution,
                                     const std::string &historicDataLength);

    void onRunOptimizer(const std::string &historicalDataFilePath);

    void onStopOptimizer();

    void onBuy();

    void onSell();

    void onInvert();
};
}

#endif // ATS_APPLICATION_H
