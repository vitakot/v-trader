#ifndef ATS_IB_DOWNLOADER_H
#define ATS_IB_DOWNLOADER_H

#include <QObject>
#include <string>
#include <vector>
#include <thread>
#include "thirdparty/IB/client/ESocket.h"
#include "thirdparty/IB/client/EReader.h"
#include "model/IB/ats_ib_events.h"
#include "model/processing/ats_processor.h"
#include <QMessageBox>

namespace trader {
class ATSIBDownloader final : public QObject {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:

    ATSIBDownloader(QObject *parent);

    ~ATSIBDownloader() = default;

    void downloadDataForSymbol(const std::string &symbol, const std::string &resolution,
                               const std::string &historicDataLength);

public slots:

    void onHistoricalBarReceived(TickerId tickerId, const ATSBar &bar);

    void onHistoricalDataEnd(TickerId tickerId);

signals:

    void message(const QMessageBox::Icon &icon, const std::string &message);
};
}
#endif // ATS_IB_DOWNLOADER_H
