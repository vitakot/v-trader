#ifndef ATSIBCLIENT_HPP
#define ATSIBCLIENT_HPP

#include "thirdparty/spimpl.hpp"
#include "thirdparty/IB/client/Contract.h"
#include "model/ats_configuration.h"
#include <QObject>
#include "thirdparty/IB/client/CommonDefs.h"
#include "thirdparty/IB/client/EClientSocket.h"

class EClientSocket;

namespace trader {

class ATSIBEvents;

class ATSIBOrders;

class ATSIBAccount;

class ATSIBDownloader;

class ATSIBClient final : public QObject {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSIBClient(QObject *parent);

    ~ATSIBClient() override;

    void setConnectOptions(const std::string &);

    std::shared_ptr<ATSIBEvents> events();

    std::shared_ptr<ATSIBOrders> orders();

    std::shared_ptr<EClientSocket> socket();

    std::shared_ptr<ATSIBAccount> account();

    std::shared_ptr<ATSIBDownloader> downloader();

    [[nodiscard]] IBConfiguration configuration() const;

    void subscribeAllData();

    void unsubscribeAllData();

    [[nodiscard]] double czkToUsdRate() const;

    [[nodiscard]] bool isClientConnected() const;

signals:

    void clientConnected();

    void clientDisconnected();

    void barReceived(const ATSBar &bar, const std::string &symbol);

public slots:

    void setConfiguration(const IBConfiguration &ibConfiguration);

    bool connectClient();

    void disconnectClient();

    void onHistoricalBarReceived(TickerId tickerId, const ATSBar &bar);

    void onRealTimeBarReceived(TickerId tickerId, const ATSBar &bar);

    void onHistoricalDataEnd(TickerId tickerId);

    void nextHistoricBar();

    void allHistoricBars();
};
}
#endif // ATSIBCLIENT_HPP
