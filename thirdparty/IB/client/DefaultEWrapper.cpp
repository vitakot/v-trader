/* Copyright (C) 2019 Interactive Brokers LLC. All rights reserved. This code is subject to the terms
 * and conditions of the IB API Non-Commercial License or the IB API Commercial License, as applicable. */

#include "StdAfx.h"
#include "DefaultEWrapper.h"

void DefaultEWrapper::tickPrice(TickerId, TickType, double, const TickAttrib &) { }
void DefaultEWrapper::tickSize(TickerId, TickType, int) { }
void DefaultEWrapper::tickOptionComputation(TickerId, TickType, double, double,
        double, double, double, double, double, double) { }
void DefaultEWrapper::tickGeneric(TickerId, TickType, double) { }
void DefaultEWrapper::tickString(TickerId, TickType, const std::string &) { }
void DefaultEWrapper::tickEFP(TickerId, TickType, double, const std::string &,
                              double, int, const std::string &, double, double) { }
void DefaultEWrapper::orderStatus(OrderId, const std::string &, double,
                                  double, double, int, int,
                                  double, int, const std::string &, double) { }
void DefaultEWrapper::openOrder(OrderId, const Contract &, const Order &, const OrderState &) { }
void DefaultEWrapper::openOrderEnd() { }
void DefaultEWrapper::winError(const std::string &, int) { }
void DefaultEWrapper::connectionClosed() { }
void DefaultEWrapper::updateAccountValue(const std::string &, const std::string &,
        const std::string &, const std::string &) { }
void DefaultEWrapper::updatePortfolio(const Contract &, double,
                                      double, double, double,
                                      double, double, const std::string &) { }
void DefaultEWrapper::updateAccountTime(const std::string &) { }
void DefaultEWrapper::accountDownloadEnd(const std::string &) { }
void DefaultEWrapper::nextValidId(OrderId) { }
void DefaultEWrapper::contractDetails(int, const ContractDetails &) { }
void DefaultEWrapper::bondContractDetails(int, const ContractDetails &) { }
void DefaultEWrapper::contractDetailsEnd(int) { }
void DefaultEWrapper::execDetails(int, const Contract &, const Execution &) { }
void DefaultEWrapper::execDetailsEnd(int) { }
void DefaultEWrapper::error(int, int, const std::string &) { }
void DefaultEWrapper::updateMktDepth(TickerId, int, int, int,
                                     double, int) { }
void DefaultEWrapper::updateMktDepthL2(TickerId, int, const std::string &, int,
                                       int, double, int, bool) { }
void DefaultEWrapper::updateNewsBulletin(int, int, const std::string &, const std::string &) { }
void DefaultEWrapper::managedAccounts(const std::string &) { }
void DefaultEWrapper::receiveFA(faDataType, const std::string &) { }
void DefaultEWrapper::historicalData(TickerId, const Bar &) { }
void DefaultEWrapper::historicalDataEnd(int, const std::string &, const std::string &) { }
void DefaultEWrapper::scannerParameters(const std::string &) { }
void DefaultEWrapper::scannerData(int, int, const ContractDetails &,
                                  const std::string &, const std::string &, const std::string &,
                                  const std::string &) { }
void DefaultEWrapper::scannerDataEnd(int) { }
void DefaultEWrapper::realtimeBar(TickerId, long, double, double, double, double,
                                  long, double, int) { }
void DefaultEWrapper::currentTime(long) { }
void DefaultEWrapper::fundamentalData(TickerId, const std::string &) { }
void DefaultEWrapper::deltaNeutralValidation(int, const DeltaNeutralContract &) { }
void DefaultEWrapper::tickSnapshotEnd(int) { }
void DefaultEWrapper::marketDataType(TickerId, int) { }
void DefaultEWrapper::commissionReport(const CommissionReport &) { }
void DefaultEWrapper::position(const std::string &, const Contract &, double, double) { }
void DefaultEWrapper::positionEnd() { }
void DefaultEWrapper::accountSummary(int, const std::string &, const std::string &, const std::string &,
                                     const std::string &) { }
void DefaultEWrapper::accountSummaryEnd(int) { }
void DefaultEWrapper::verifyMessageAPI(const std::string &) { }
void DefaultEWrapper::verifyCompleted(bool, const std::string &) { }
void DefaultEWrapper::displayGroupList(int, const std::string &) { }
void DefaultEWrapper::displayGroupUpdated(int, const std::string &) { }
void DefaultEWrapper::verifyAndAuthMessageAPI(const std::string &, const std::string &) { }
void DefaultEWrapper::verifyAndAuthCompleted(bool, const std::string &) { }
void DefaultEWrapper::connectAck() { }
void DefaultEWrapper::positionMulti(int, const std::string &,const std::string &, const Contract &, double,
                                    double) { }
void DefaultEWrapper::positionMultiEnd(int) { }
void DefaultEWrapper::accountUpdateMulti(int, const std::string &, const std::string &, const std::string &,
        const std::string &, const std::string &) { }
void DefaultEWrapper::accountUpdateMultiEnd(int) { }
void DefaultEWrapper::securityDefinitionOptionalParameter(int, const std::string &, int,
        const std::string &,
        const std::string &, const std::set<std::string> &, const std::set<double> &) { }
void DefaultEWrapper::securityDefinitionOptionalParameterEnd(int) { }
void DefaultEWrapper::softDollarTiers(int, const std::vector<SoftDollarTier> &) { }
void DefaultEWrapper::familyCodes(const std::vector<FamilyCode> &) { }
void DefaultEWrapper::symbolSamples(int, const std::vector<ContractDescription> &) { }
void DefaultEWrapper::mktDepthExchanges(const std::vector<DepthMktDataDescription> &) { }
void DefaultEWrapper::tickNews(int, time_t, const std::string &, const std::string &,
                               const std::string &, const std::string &) { }
void DefaultEWrapper::smartComponents(int, const SmartComponentsMap &) { }
void DefaultEWrapper::tickReqParams(int, double, const std::string &, int) { }
void DefaultEWrapper::newsProviders(const std::vector<NewsProvider> &) { }
void DefaultEWrapper::newsArticle(int, int, const std::string &) { }
void DefaultEWrapper::historicalNews(int, const std::string &, const std::string &, const std::string &,
                                     const std::string &) { }
void DefaultEWrapper::historicalNewsEnd(int, bool) { }
void DefaultEWrapper::headTimestamp(int, const std::string &) { }
void DefaultEWrapper::histogramData(int, const HistogramDataVector &) { }
void DefaultEWrapper::historicalDataUpdate(TickerId, const Bar &) { }
void DefaultEWrapper::rerouteMktDataReq(int, int, const std::string &) { }
void DefaultEWrapper::rerouteMktDepthReq(int, int, const std::string &) { }
void DefaultEWrapper::marketRule(int, const std::vector<PriceIncrement> &) { }
void DefaultEWrapper::pnl(int, double, double, double) { }
void DefaultEWrapper::pnlSingle(int, int, double, double, double, double) { }
void DefaultEWrapper::historicalTicks(int, const std::vector<HistoricalTick> &, bool) { }
void DefaultEWrapper::historicalTicksBidAsk(int, const std::vector<HistoricalTickBidAsk> &, bool) { }
void DefaultEWrapper::historicalTicksLast(int, const std::vector<HistoricalTickLast> &, bool) { }
void DefaultEWrapper::tickByTickAllLast(int, int, time_t, double, int, const TickAttribLast &,
                                        const std::string &, const std::string &) { }
void DefaultEWrapper::tickByTickBidAsk(int, time_t, double, double, int, int,
                                       const TickAttribBidAsk &) { }
void DefaultEWrapper::tickByTickMidPoint(int, time_t, double) { }
void DefaultEWrapper::orderBound(long long, int, int) { }
void DefaultEWrapper::completedOrder(const Contract &, const Order &, const OrderState &) { }
void DefaultEWrapper::completedOrdersEnd() { }
