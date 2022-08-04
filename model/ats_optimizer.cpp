#include "ats_optimizer.h"
#include <fstream>
#include "model/interface/ats_iprocessor.h"
#include <thread>
#include <mutex>
#include "ats_utils.h"
#include <QFileInfo>
#include <QThread>
#include <QTimer>
#include "model/processing/ats_processor.h"

namespace trader {

struct ATSOptimizer::P {
    std::string m_resultsDirPath;
    std::vector<double> m_open;
    std::vector<double> m_high;
    std::vector<double> m_low;
    std::vector<double> m_close;
    std::vector<std::int64_t> m_unixTime;
    std::ofstream m_resultsFile;
    std::recursive_mutex m_mutex;
    std::list<std::size_t> m_usedParameters;
    std::string m_optimizerDataFilePath;
    std::atomic<bool> m_working = false;
    QTimer *m_updater = nullptr;

    const double m_commission = 5;
    const double m_investment = 100000;

    std::vector<std::thread> m_workers;

    static void workerFunction(ATSOptimizer *parent);

    bool init(const std::string &historicalDataFilePath);

//    bool generateParameters(ATSIndicatorParameters &parameters) {
//        const std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        int numAttempts = 0;
//
//        while (numAttempts < 1000) {
//            auto newParamns = ATSIndicatorParameters::test();
//            auto hash = newParamns.hash();
//
//            auto it = std::find(m_usedParameters.begin(), m_usedParameters.end(), hash);
//
//            if (it == m_usedParameters.end()) {
//                m_usedParameters.push_back(hash);
//                parameters = newParamns;
//                return true;
//            } else {
//                numAttempts++;
//            }
//        }
//        return false;
//    }

    void saveResult(const ATSOptimizationResult &result) {
//        const std::lock_guard<std::recursive_mutex> lock(m_mutex);
//        m_resultsFile << result.m_profit << "," << result.m_numTrades << "," << result.m_parameters.toString()
//                      << std::endl;
    }
};

//----------------------------------------------------------------------------------------------------------------------
ATSOptimizer::ATSOptimizer(const std::string &resultsDirPath, QObject *parent) : QObject(parent),
                                                                                 m_p(spimpl::make_unique_impl<P>()) {
    m_p->m_resultsDirPath = resultsDirPath;
    m_p->m_updater = new QTimer(this);

    connect(m_p->m_updater, &QTimer::timeout, this, [&]() {
        emit optimizing(m_p->m_working, m_p->m_optimizerDataFilePath);
    });
}

//----------------------------------------------------------------------------------------------------------------------
ATSOptimizer::~ATSOptimizer() {
    m_p->m_working = false;

    for (auto &thread : m_p->m_workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    m_p->m_resultsFile.close();
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSOptimizer::P::init(const std::string &historicalDataFilePath) {
    std::vector<ATSBar> timeSeries;

    m_optimizerDataFilePath = historicalDataFilePath;
    m_open.clear();
    m_high.clear();
    m_low.clear();
    m_close.clear();

    if (loadTimeSeriesFromCSV(historicalDataFilePath, timeSeries)) {
        for (const auto &bar : timeSeries) {
            m_open.push_back(bar.m_open);
            m_high.push_back(bar.m_high);
            m_low.push_back(bar.m_low);
            m_close.push_back(bar.m_close);
            m_unixTime.push_back(bar.m_unixTime);
        }

        auto file = QFileInfo(QString::fromStdString(historicalDataFilePath)).baseName();
        auto resultsFileName = QString::fromStdString(m_resultsDirPath) + "/" + file + "_results.csv";

        m_resultsFile.open(resultsFileName.toStdString());

        if (m_resultsFile.is_open()) {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOptimizer::P::workerFunction(ATSOptimizer *parent) {
    //    ATSIndicatorParameters parameters;

    //    while(parent->m_p->generateParameters(parameters) && parent->m_p->m_working) {

    //        auto processor = ATSProcessor(nullptr);
    //        processor.init(parameters);

    //        for(std::size_t i = 0; i < parent->m_p->m_close.size(); i++) {
    //            ATSBar bar;
    //            bar.m_open = parent->m_p->m_open[i];
    //            bar.m_high = parent->m_p->m_high[i];
    //            bar.m_low = parent->m_p->m_low[i];
    //            bar.m_close = parent->m_p->m_close[i];
    //            bar.m_unixTime = parent->m_p->m_unixTime[i];

    //            processor.process(bar);
    //        }

    //        ATSOptimizationResult result;
    //        result.m_parameters = parameters;
    //        result.m_numTrades = static_cast<int>(processor.orderRequests().size());
    //        result.m_orderRequests = processor.orderRequests();

    //        double profit = 0;
    //        double commision = 0;

    //        for(std::size_t i = 1; i < result.m_orderRequests.size(); i++) {
    //            auto prevEnterPrice = result.m_orderRequests[i-1].enterPrice();
    //            auto currentEnterPrice = result.m_orderRequests[i].enterPrice();

    //            if(result.m_orderRequests[i-1].action() == ATSOrderRequest::Action::Sell) {
    //                profit += (prevEnterPrice  - currentEnterPrice) * parent->m_p->m_investment;
    //            }
    //            else {
    //                profit += (currentEnterPrice - prevEnterPrice) * parent->m_p->m_investment;
    //            }
    //        }

    //        for(std::size_t i = 0; i < result.m_orderRequests.size(); i++) {
    //            commision += parent->m_p->m_commission;
    //        }

    //        result.m_profit = profit - commision;
    //        parent->m_p->saveResult(result);
    //    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOptimizer::run(const std::string &historicalDataFilePath) {
    bool ok = m_p->init(historicalDataFilePath);

    if (!ok) {
        emit message(QMessageBox::Icon::Critical, "Optimizer could not load historical trading data file.");
        return;
    }

    m_p->m_updater->start(1000);

    m_p->m_working = true;

    emit optimizing(m_p->m_working, m_p->m_optimizerDataFilePath);

    auto numThreads = QThread::idealThreadCount();

    for (auto i = 0; i < numThreads - 1; i++) {
        auto worker = std::thread(P::workerFunction, this);
        m_p->m_workers.push_back(std::move(worker));
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSOptimizer::stop() {
    m_p->m_working = false;
    m_p->m_updater->stop();

    emit optimizing(m_p->m_working, m_p->m_optimizerDataFilePath);

    for (auto &thread : m_p->m_workers) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
}
