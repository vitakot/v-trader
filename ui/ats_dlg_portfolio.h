#ifndef ATS_DLG_PORTFOLIO_H
#define ATS_DLG_PORTFOLIO_H

#include <QDialog>
#include "thirdparty/spimpl.hpp"

namespace trader {
    class ATSConfiguration;
}

class ATSDlgPortfolio : public QDialog {
Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSDlgPortfolio(const trader::ATSConfiguration &configuration, QWidget *parent = nullptr);

    ~ATSDlgPortfolio() = default;
};

#endif // ATS_DLG_PORTFOLIO_H
