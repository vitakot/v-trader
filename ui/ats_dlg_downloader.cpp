#include "ats_dlg_downloader.h"
#include "ui_ats_dlg_downloader.h"
#include <QMessageBox>

struct ATSDlgDownloader::P {

    ATSDlgDownloader *m_parent = nullptr;
    Ui::ATSDlgDownloader *m_ui = nullptr;
    std::string m_resolution;

    P(ATSDlgDownloader *parent) {
        m_parent = parent;
    }

    ~P() {
        delete m_ui;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSDlgDownloader::ATSDlgDownloader(const std::string &resolution, QWidget *parent) : QDialog(parent),
                                                                                     m_p(spimpl::make_unique_impl<P>(
                                                                                             this)) {
    m_p->m_ui = new Ui::ATSDlgDownloader;
    m_p->m_ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    m_p->m_resolution = resolution;

    connect(m_p->m_ui->pushButtonDownload, &QPushButton::clicked, this, [this]() {

        if (m_p->m_ui->lineEditSymbol->text().isEmpty()) {
            QMessageBox::information(this, qAppName(), "No symbol entered.", QMessageBox::Ok);
            return;
        }

        int days = m_p->m_ui->spinBoxHistoryNumDays->value();
        int hours = m_p->m_ui->spinBoxHistoryNumHours->value();

        if (hours == 0 && days == 0) {
            QMessageBox::information(this, qAppName(), "Can not download zero history.", QMessageBox::Ok);
            return;
        }

        if (hours == 0 && days != 0) {
            std::string historicDataLength = std::to_string(days) + " D";
            emit downloadTimeSeriesRequest(m_p->m_ui->lineEditSymbol->text().toStdString(), m_p->m_resolution,
                                           historicDataLength);
            return;
        }

        if (hours != 0 && days == 0) {
            if (hours >= 25) {
                QMessageBox::information(this, qAppName(),
                                         "Can not download history of this length, must be 24 hours or less.",
                                         QMessageBox::Ok);
                return;
            }

            std::string historicDataLength = std::to_string(hours * 3600) + " S";
            emit downloadTimeSeriesRequest(m_p->m_ui->lineEditSymbol->text().toStdString(), m_p->m_resolution,
                                           historicDataLength);
            return;
        }

        if (hours != 0 && days != 0) {
            QMessageBox::information(this, qAppName(),
                                     "Can not download history of this length, must be 24 hours or less, or the whole days only",
                                     QMessageBox::Ok);
            return;

        }
    });
}
