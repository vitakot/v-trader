#include "ats_axis_ticker_date_time.h"

//----------------------------------------------------------------------------------------------------------------------
ATSAxisTickerDateTime::ATSAxisTickerDateTime() {

}

//----------------------------------------------------------------------------------------------------------------------
QVector<double> ATSAxisTickerDateTime::createTickVector(double tickStep, const QCPRange &range) {
    auto result = QCPAxisTickerDateTime::createTickVector(tickStep, range);


    return result;
}
