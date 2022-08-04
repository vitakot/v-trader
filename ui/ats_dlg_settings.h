#ifndef ATS_DLG_SETTINGS_H
#define ATS_DLG_SETTINGS_H

#include <QDialog>
#include "thirdparty/spimpl.hpp"

namespace trader {
    class ATSConfiguration;
}

class ATSDlgSettings : public QDialog {
Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    explicit ATSDlgSettings(const trader::ATSConfiguration &configuration, QWidget *parent = nullptr);

    trader::ATSConfiguration configuration() const;
};

#endif // ATS_DLG_SETTINGS_H
