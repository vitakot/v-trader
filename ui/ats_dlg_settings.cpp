#include "ats_dlg_settings.h"
#include "ui_ats_dlg_settings.h"
#include "model/ats_configuration.h"

struct ATSDlgSettings::P {
    Ui::ATSDlgSettings *m_ui = nullptr;
    ATSDlgSettings *m_parent = nullptr;
    trader::ATSConfiguration m_configuration;

    P(ATSDlgSettings *parent) {
        m_parent = parent;
    }

    ~P() {
        delete m_ui;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSDlgSettings::ATSDlgSettings(const trader::ATSConfiguration &configuration, QWidget *parent) : QDialog(parent),
                                                                                                 m_p(spimpl::make_unique_impl<P>(
                                                                                                         this)) {
    m_p->m_ui = new Ui::ATSDlgSettings;
    m_p->m_ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    m_p->m_configuration = configuration;
}

//----------------------------------------------------------------------------------------------------------------------
trader::ATSConfiguration ATSDlgSettings::configuration() const {
    return m_p->m_configuration;
}
