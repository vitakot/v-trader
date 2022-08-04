#ifndef ATSAXISTICKERDATETIME_H
#define ATSAXISTICKERDATETIME_H

#include "thirdparty/qcustomplot.h"

class ATSAxisTickerDateTime : public QCPAxisTickerDateTime {

    virtual QVector<double> createTickVector(double tickStep, const QCPRange &range) Q_DECL_OVERRIDE;

public:
    ATSAxisTickerDateTime();

};

#endif // ATSAXISTICKERDATETIME_H
