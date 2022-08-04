#ifndef ATSDSPALGORITHMS_H
#define ATSDSPALGORITHMS_H

#include <vector>
#include <cstdint>
#include <thirdparty/spimpl.hpp>

namespace trader {

class ATSDSPAlgorithms {

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:

    static void aroon(const std::vector<double> &inHigh, const std::vector<double> &inLow, int inTimePeriod,
                      std::vector<double> &down,
                      std::vector<double> &up);

    static bool donchian(const std::vector<double> &inHigh, const std::vector<double> &inLow, int inTimePeriod,
                         std::vector<double> &upper,
                         std::vector<double> &middle, std::vector<double> &lower);

    static void alma(const std::vector<double> &in, int inTimePeriod, std::vector<double> &out);

    static void mmi(const std::vector<double> &in, int length, std::vector<double> &out);
};
}
#endif // ATSDSPALGORITHMS_H
