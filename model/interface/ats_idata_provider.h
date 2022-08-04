#ifndef ATSIDATAPROVIDER_H
#define ATSIDATAPROVIDER_H

#include "model/ats_configuration.h"

namespace trader {

struct ATSIDataProvider {

    virtual ~ATSIDataProvider() = default;

    //subscribeLiveMarketData
    virtual void
    retrieveHistoricData(const Contract &contract, const BarResolution &resolution, std::int64_t lengthInSeconds,
                         std::vector<ATSBar> &bars) const = 0;

};

}
#endif // ATSIDATAPROVIDER_H
