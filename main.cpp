#include "model/ats_application.h"
#include <fstream>
#include <iostream>
#include <QStandardPaths>
#include "ui/ats_wnd_main.h"
#include <QMessageBox>
#include <QDebug>

using namespace trader;

//----------------------------------------------------------------------------------------------------------------------
void atsMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                    context.function);
            break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                    context.function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                    context.function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                    context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line,
                    context.function);
            abort();
    }
}

//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    std::ofstream ofs((QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                       QString("/TraderService/log.txt")).toStdString());

    qInstallMessageHandler(atsMessageOutput);
    ATSApplication a(argc, argv);
    ATSWndMain w;
    w.showMaximized();

    QObject::connect(&a, &ATSApplication::configurationChanged, &w, &ATSWndMain::setConfiguration,
                     Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::completedOrdersChanged, &w, &ATSWndMain::setCompletedOrders,
                     Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::connected, &w, &ATSWndMain::setConnected, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::barReceived, &w, &ATSWndMain::addBar, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::barProcessed, &w, &ATSWndMain::addProcessedBar, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::orderCompleted, &w, &ATSWndMain::onOrderCompleted, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::stopTrading, &w, &ATSWndMain::onCloseTrading, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::accountInfoUpdated, &w, &ATSWndMain::onAccountInfoUpdated,
                     Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::accountPositionUpdated, &w, &ATSWndMain::onAccountPositionUpdated,
                     Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::message, &w, &ATSWndMain::showMessage, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::optimizing, &w, &ATSWndMain::onOptimizing, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::trendVariables, &w, &ATSWndMain::onTrendVariables, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::positionExited, &w, &ATSWndMain::onPositionExited, Qt::QueuedConnection);
    QObject::connect(&a, &ATSApplication::currentProfit, &w, &ATSWndMain::onCurrentProfit, Qt::QueuedConnection);

    auto retVal = a.init();

    if (!retVal.empty()) {
        auto msg = "Application initialization failed";
        QMessageBox::critical(&w, qAppName(), msg, QMessageBox::Ok);
        qCritical() << msg;
        return -1;
    }

    QObject::connect(&w, &ATSWndMain::configurationChanged, &a, &ATSApplication::onSetConfiguration,
                     Qt::QueuedConnection);
    QObject::connect(&w, &ATSWndMain::downloadTimeSeriesRequest, &a, &ATSApplication::onDownloadTimeSeriesRequest,
                     Qt::QueuedConnection);
    QObject::connect(&w, &ATSWndMain::runOptimizer, &a, &ATSApplication::onRunOptimizer, Qt::QueuedConnection);
    QObject::connect(&w, &ATSWndMain::stopOptimizer, &a, &ATSApplication::onStopOptimizer, Qt::QueuedConnection);
    QObject::connect(&w, &ATSWndMain::buy, &a, &ATSApplication::onBuy, Qt::QueuedConnection);
    QObject::connect(&w, &ATSWndMain::sell, &a, &ATSApplication::onSell, Qt::QueuedConnection);
    QObject::connect(&w, &ATSWndMain::invert, &a, &ATSApplication::onInvert, Qt::QueuedConnection);

    return a.exec();
}
