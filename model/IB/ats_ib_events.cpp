#include "ats_ib_events.h"
#include <QDebug>
#include <QDateTime>
#include "model/processing/ats_processor.h"

namespace trader {

ATSIBEventsObserver::ATSIBEventsObserver(QObject *parent) : QObject(parent) {

}

ATSIBEvents::ATSIBEvents() {
    m_observer = std::make_shared<ATSIBEventsObserver>();
}

std::shared_ptr<ATSIBEventsObserver> ATSIBEvents::observer() const {
    return m_observer;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickPrice(TickerId tickerId, TickType field, double price, const TickAttrib &attribs) {

    QString value(
            "Tick Price. Ticker Id: %1, Field: %2, Price: %3, CanAutoExecute: %4, PastLimit: %5, PreOpen: %6 \n");
    //qDebug()<<value.arg(tickerId).arg((int)field).arg(price).arg(attribs.canAutoExecute).arg(attribs.pastLimit).arg(attribs.preOpen);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickSize(TickerId, TickType, int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickOptionComputation(TickerId, TickType, double, double, double, double, double, double, double,
                                        double) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickGeneric(TickerId tickerId, TickType tickType, double value) {
    //printf("Tick Generic. Ticker Id: %ld, Type: %d, Value: %g\n", tickerId, (int)tickType, value);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickString(TickerId tickerId, TickType tickType, const std::string &value) {
    // printf("Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickEFP(TickerId, TickType, double, const std::string &, double, int, const std::string &, double,
                          double) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::orderStatus(OrderId orderId, const std::string &status, double filled,
                              double remaining, double avgFillPrice, int permId, int parentId,
                              double lastFillPrice, int clientId, const std::string &whyHeld, double mktCapPrice) {
    ATSIBOrderStatus orderStatus;
    orderStatus.m_orderId = orderId;
    orderStatus.m_status = status;
    orderStatus.m_filled = filled;
    orderStatus.m_remaining = remaining;
    orderStatus.m_avgFillPrice = avgFillPrice;
    orderStatus.m_permId = permId;
    orderStatus.m_parentId = parentId;
    orderStatus.m_lastFillPrice = lastFillPrice;
    orderStatus.m_clientId = clientId;
    orderStatus.m_whyHeld = whyHeld;
    orderStatus.m_mktCapPrice = mktCapPrice;

    emit m_observer->orderStatus(orderStatus);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::openOrder(OrderId, const Contract &contract, const Order &order, const OrderState &orderState) {
    ATSIBOrder atsOrder;
    atsOrder.m_contract = contract;
    atsOrder.m_orderState = orderState;
    atsOrder.m_order = order;

    emit m_observer->openOrderReceived(atsOrder);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::openOrderEnd() {
    emit m_observer->openOrderEndReceived();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::winError(const std::string &, int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::connectionClosed() {
    emit m_observer->connectionClosed();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::updateAccountValue(const std::string &, const std::string &, const std::string &,
                                     const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::updatePortfolio(const Contract &contract, double position,
                                  double marketPrice, double marketValue, double averageCost,
                                  double unrealizedPNL, double realizedPNL, const std::string &accountName) {

    ATSIBPosition pos;
    pos.m_account = accountName;
    pos.m_contract = contract;
    pos.m_position = position;
    pos.m_marketPrice = marketPrice;
    pos.m_marketValue = marketValue;
    pos.m_averageCost = averageCost;
    pos.m_unrealizedPNL = unrealizedPNL;
    pos.m_realizedPNL = realizedPNL;

    emit m_observer->accountPositionReceived(pos);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::updateAccountTime(const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::accountDownloadEnd(const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::nextValidId(OrderId orderId) {
    emit m_observer->nextValidOrderId(orderId);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::contractDetails(int, const ContractDetails &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::bondContractDetails(int, const ContractDetails &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::contractDetailsEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::execDetails(int, const Contract &, const Execution &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::execDetailsEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::error(int id, int errorCode, const std::string &errorString) {
    qWarning() << "IB Error, Id: " << id << ", code: " << errorCode << ", message: " << errorString.c_str();
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::updateMktDepth(TickerId, int, int, int, double, int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::updateMktDepthL2(TickerId, int, const std::string &, int, int, double, int, bool) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::updateNewsBulletin(int, int, const std::string &, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::managedAccounts(const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::receiveFA(faDataType, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalData(TickerId reqId, const Bar &bar) {
    ATSBar atsBar;

    auto dateTime = QDateTime::fromString(QString::fromStdString(bar.time), "yyyyMMdd  hh:mm:ss");
    auto unixTime = dateTime.toSecsSinceEpoch();

    atsBar.m_unixTime = unixTime;
    atsBar.m_open = bar.open;
    atsBar.m_high = bar.high;
    atsBar.m_low = bar.low;
    atsBar.m_close = bar.close;
    atsBar.m_volume = bar.volume;

    emit m_observer->historicalBarReceived(reqId, atsBar);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalDataEnd(int reqId, const std::string &, const std::string &) {
    emit m_observer->historicalDataEndReceived(reqId);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::scannerParameters(const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::scannerData(int, int, const ContractDetails &, const std::string &, const std::string &,
                              const std::string &, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::scannerDataEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                              long, double, int) {
    ATSBar bar;

    auto dateTime = QDateTime::fromSecsSinceEpoch(time);

    bar.m_unixTime = time;
    bar.m_open = open;
    bar.m_high = high;
    bar.m_low = low;
    bar.m_close = close;

    emit m_observer->realTimeBarReceived(reqId, bar);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::currentTime(long) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::fundamentalData(TickerId, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::deltaNeutralValidation(int, const DeltaNeutralContract &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickSnapshotEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::marketDataType(TickerId, int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::commissionReport(const CommissionReport &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::position(const std::string &account, const Contract &contract, double position, double avgCost) {
    ATSIBPositionBrief positionBrief;
    positionBrief.m_account = account;
    positionBrief.m_contract = contract;
    positionBrief.m_position = position;
    positionBrief.m_averageCost = avgCost;

    emit m_observer->accountPositionBriefReceived(positionBrief);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::positionEnd() {

}

//----------------------------------------------------------------------------------------------------------------------
void
ATSIBEvents::accountSummary(int reqId, const std::string &account, const std::string &tag, const std::string &value,
                            const std::string &curency) {
    emit m_observer->accountInfoReceived(reqId, account, curency, tag, value);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::accountSummaryEnd(int reqId) {
    emit m_observer->accountInfoEndReceived(reqId);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::verifyMessageAPI(const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::verifyCompleted(bool, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::displayGroupList(int, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::displayGroupUpdated(int, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::verifyAndAuthMessageAPI(const std::string &, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::verifyAndAuthCompleted(bool, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::connectAck() {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::positionMulti(int, const std::string &, const std::string &, const Contract &, double, double) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::positionMultiEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::accountUpdateMulti(int, const std::string &, const std::string &, const std::string &,
                                     const std::string &, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::accountUpdateMultiEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::securityDefinitionOptionalParameter(int, const std::string &, int, const std::string &,
                                                      const std::string &, const std::set<std::string> &,
                                                      const std::set<double> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::securityDefinitionOptionalParameterEnd(int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::softDollarTiers(int, const std::vector<SoftDollarTier> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::familyCodes(const std::vector<FamilyCode> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::symbolSamples(int, const std::vector<ContractDescription> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::mktDepthExchanges(const std::vector<DepthMktDataDescription> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickNews(int, time_t, const std::string &, const std::string &, const std::string &,
                           const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::smartComponents(int, const SmartComponentsMap &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickReqParams(int, double, const std::string &, int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::newsProviders(const std::vector<NewsProvider> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::newsArticle(int, int, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalNews(int, const std::string &, const std::string &, const std::string &,
                                 const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalNewsEnd(int, bool) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::headTimestamp(int, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::histogramData(int, const HistogramDataVector &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalDataUpdate(TickerId, const Bar &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::rerouteMktDataReq(int, int, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::rerouteMktDepthReq(int, int, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::marketRule(int, const std::vector<PriceIncrement> &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::pnl(int, double, double, double) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::pnlSingle(int, int, double, double, double, double) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalTicks(int, const std::vector<HistoricalTick> &, bool) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalTicksBidAsk(int, const std::vector<HistoricalTickBidAsk> &, bool) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::historicalTicksLast(int, const std::vector<HistoricalTickLast> &, bool) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickByTickAllLast(int, int, time_t, double, int, const TickAttribLast &,
                                    const std::string &, const std::string &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickByTickBidAsk(int, time_t, double, double, int, int, const TickAttribBidAsk &) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::tickByTickMidPoint(int, time_t, double) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::orderBound(long long, int, int) {

}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::completedOrder(const Contract &contract, const Order &order, const OrderState &orderState) {
    ATSIBOrder atsOrder;
    atsOrder.m_contract = contract;
    atsOrder.m_orderState = orderState;
    atsOrder.m_order = order;

    emit m_observer->completedOrderReceived(atsOrder);
}

//----------------------------------------------------------------------------------------------------------------------
void ATSIBEvents::completedOrdersEnd() {
    emit m_observer->completedOrderEndReceived();
}

}
