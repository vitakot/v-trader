#ifndef ATS_DLG_DOWNLOADER_H
#define ATS_DLG_DOWNLOADER_H

#include <QDialog>
#include <thirdparty/spimpl.hpp>
#include <model/ats_configuration.h>

class ATSDlgDownloader : public QDialog {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    ATSDlgDownloader(const std::string &resolution, QWidget *parent = nullptr);

    ~ATSDlgDownloader() = default;

signals:

    void downloadTimeSeriesRequest(const std::string symbol, const std::string &resolution,
                                   const std::string &historicDataLength);
};

#endif // ATS_DLG_DOWNLOADER_H
