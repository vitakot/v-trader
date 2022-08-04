#include "ats_dlg_optimizer.h"
#include "ui_ats_dlg_optimizer.h"
#include <QDir>
#include <QDirIterator>

struct ATSDlgOptimizer::P {

    ATSDlgOptimizer *m_parent = nullptr;
    Ui::ATSDlgOptimizer *m_ui = nullptr;
    std::string m_pathToHistoricalDataDir;

    P(ATSDlgOptimizer *parent) {
        m_parent = parent;
    }

    ~P() {
        delete m_ui;
    }

    void readHistoricalDataFiles(const std::string &dirPath);
};

//----------------------------------------------------------------------------------------------------------------------
ATSDlgOptimizer::ATSDlgOptimizer(const std::string &pathToHistoricalDataDir, QWidget *parent) : QDialog(parent),
                                                                                                m_p(spimpl::make_unique_impl<P>(
                                                                                                        this)) {
    m_p->m_ui = new Ui::ATSDlgOptimizer;
    m_p->m_ui->setupUi(this);

    m_p->m_pathToHistoricalDataDir = pathToHistoricalDataDir;

    m_p->readHistoricalDataFiles(m_p->m_pathToHistoricalDataDir);

    connect(m_p->m_ui->listWidgetInputs, &QListWidget::itemDoubleClicked, this, [&](QListWidgetItem *item) {
        m_p->m_ui->lineEditSelected->setText(item->text());
        m_p->m_ui->pushButtonStart->setEnabled(true);
    });

    connect(m_p->m_ui->pushButtonStart, &QPushButton::clicked, this, [&]() {
        std::string filePath = m_p->m_pathToHistoricalDataDir + "/" + m_p->m_ui->lineEditSelected->text().toStdString();
        emit run(filePath);
    });

    connect(m_p->m_ui->pushButtonStop, &QPushButton::clicked, this, [&]() {
        emit stop();
    });
}

//----------------------------------------------------------------------------------------------------------------------
void ATSDlgOptimizer::P::readHistoricalDataFiles(const std::string &dirPath) {
    QStringList nameFilter("*.csv");
    QDir directory(QString::fromStdString(dirPath + "/"));
    QStringList files = directory.entryList(nameFilter);
    m_ui->listWidgetInputs->addItems(files);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSDlgOptimizer::onOptimizing(bool running, const std::string &historicalDataFilePath) {
    m_p->m_ui->pushButtonStop->setEnabled(running);

    if (running) {
        m_p->m_ui->progressBar->setRange(0, 0);
    } else {
        m_p->m_ui->progressBar->setRange(0, 1);
        m_p->m_ui->progressBar->setValue(0);
    }
}
