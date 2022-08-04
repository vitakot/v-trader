#ifndef ATS_DLG_OPTIMIZER_H
#define ATS_DLG_OPTIMIZER_H

#include <QDialog>
#include "thirdparty/spimpl.hpp"

class ATSDlgOptimizer : public QDialog {
Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSDlgOptimizer(const std::string &pathToHistoricalDataDir, QWidget *parent = nullptr);

public slots:

    void onOptimizing(bool running, const std::string &historicalDataFilePath);

signals:

    void run(const std::string &historicalDataFilePath);

    void stop();
};

#endif // ATS_DLG_OPTIMIZER_H
