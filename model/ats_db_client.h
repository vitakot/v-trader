#ifndef ATSDBCLIENT_H
#define ATSDBCLIENT_H

#include "thirdparty/spimpl.hpp"

namespace trader {

class ATSIBPosition;

class ATSDBClient {

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    ATSDBClient();

    bool init();

    bool addPosition(const ATSIBPosition &position);
};
}

#endif // ATSDBCLIENT_H
