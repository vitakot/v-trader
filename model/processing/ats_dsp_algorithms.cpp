#include "ats_dsp_algorithms.h"
#include "ta-lib/ta_libc.h"
#include <cmath>
#include <algorithm>

namespace trader {

struct ATSDSPAlgorithms::P {

    static double medianCore(const double *x, int n) {

        std::vector<double> workBuffer(x, x+n);
        double temp;
        int i, j;

        // the following two loops sort the array x in ascending order
        for (i = 0; i < n - 1; i++) {
            for (j = i + 1; j < n; j++) {
                if (workBuffer[j] < workBuffer[i]) {
                    // swap elements
                    temp = workBuffer[i];
                    workBuffer[i] = workBuffer[j];
                    workBuffer[j] = temp;
                }
            }
        }

        if (n % 2 == 0) {
            // if there is an even number of elements, return mean of the two elements in the middle
            return ((workBuffer[n / 2] + workBuffer[n / 2 - 1]) / 2.0);
        } else {
            // else return the element in the middle
            return workBuffer[n / 2];
        }

    }

//    double lowPassCore(double *Data, int Period)
//    {
//        auto *LP = series(Data[0]);
//        auto a = 2.0/(1+Period);
//        return LP[0] = (a-0.25*a*a)*Data[0]
//                       + 0.5*a*a*Data[1]
//                       - (a-0.75*a*a)*Data[2]
//                       + 2*(1.-a)*LP[1]
//                       - (1.-a)*(1.-a)*LP[2];
//    }
//
    static double MMICore(const double *data, int length) {
        double m = medianCore(data, length);
        int i, nh = 0, nl = 0;
        for (i = 1; i < length; i++) {
            if (data[i] > m && data[i] > data[i - 1]) // mind Data order: Data[0] is newest!
                nl++;
            else if (data[i] < m && data[i] < data[i - 1])
                nh++;
        }
        return 100. * (nl + nh) / (length - 1);
    }

    static double ALMACore(const double *data, int period) {
        auto m = floor(0.85 * (period - 1));
        auto s = period / 6.0;
        auto alma = 0.;
        auto wSum = 0.;

        for (auto i = 0; i < period; i++) {
            auto w = exp(-(i - m) * (i - m) / (2 * s * s));
            alma += data[period - 1 - i] * w;
            wSum += w;
        }
        return alma / wSum;
    }

//    static double laguerreCore(double *Data, double alpha) {
//        auto *L = series(Data[0]);
//        L[0] = alpha*Data[0] + (1-alpha)*L[1];
//        L[2] = -(1-alpha)*L[0] + L[1] + (1-alpha)*L[2+1];
//        L[4] = -(1-alpha)*L[2] + L[2+1] + (1-alpha)*L[4+1];
//        L[6] = -(1-alpha)*L[4] + L[4+1] + (1-alpha)*L[6+1];
//        return (L[0]+2*L[2]+2*L[4]+L[6])/6;
//    }
//
//    static double smooth(double *data, int period)
//    {
//        auto f = (1.414*M_PI) / period;
//        auto a = exp(-f);
//        auto c2 = 2*a*cos(f);
//        auto c3 = -a*a;
//        auto c1 = 1 - c2 - c3;
//        auto *S = series(data[0]);
//        return S[0] = c1 * (data[0] + data[1]) * 0.5 + c2 * S[1] + c3 * S[2];
//    }

//    double HMA(double *Data, int Period)
//    {
//        return WMA(series(2*WMA(Data,Period/2)-WMA(Data,Period),sqrt(Period));
//    }

//    double ZMA(double *Data,int Period)
//    {
//        var *ZMA = series(Data[0]);
//        var a = 2.0/(1+Period);
//        var Ema = EMA(Data,Period);
//        var Error = 1000000;
//        var Gain, GainLimit=5, BestGain=0;
//        for(Gain = -GainLimit; Gain < GainLimit; Gain += 0.1) {
//            ZMA[0] = a*(Ema + Gain*(Data[0]-ZMA[1])) + (1-a)*ZMA[1];
//            var NewError = Data[0] - ZMA[0];
//            if(abs(Error) < newError) {
//                Error = abs(newError);
//                BestGain = Gain;
//            }
//        }
//        return ZMA[0] = a*(Ema + BestGain*(Data[0]-ZMA[1])) + (1-a)*ZMA[1];
//    }
};

//----------------------------------------------------------------------------------------------------------------------
void ATSDSPAlgorithms::aroon(const std::vector<double> &inHigh, const std::vector<double> &inLow, int inTimePeriod,
                             std::vector<double> &down,
                             std::vector<double> &up) {
    std::vector<double> outAroonUp(inHigh.size());
    std::vector<double> outAroonDown(inHigh.size());

    auto startIdx = inTimePeriod;
    auto outIdx = startIdx;

    auto today = startIdx;
    auto trailingIdx = startIdx - inTimePeriod;

    auto lowestIdx = -1;
    auto highestIdx = -1;
    auto lowest = 0.0;
    auto highest = 0.0;
    auto factor = 100.0 / static_cast<double>((inTimePeriod));

    for (auto index = today; today < static_cast<std::int32_t>(inHigh.size()); index++) {
        auto tmp = inLow[static_cast<std::size_t>(today)];
        if (lowestIdx < trailingIdx) {
            lowestIdx = trailingIdx;
            lowest = inLow[static_cast<std::size_t>(lowestIdx)];
            auto i = lowestIdx;
            i++;
            for (auto j = i; j <= today; j++) {
                tmp = inLow[static_cast<std::size_t>(j)];
                if (tmp <= lowest) {
                    lowestIdx = j;
                    lowest = tmp;
                }
            }
        } else if (tmp <= lowest) {
            lowestIdx = today;
            lowest = tmp;
        }
        tmp = inHigh[static_cast<std::size_t>(today)];
        if (highestIdx < trailingIdx) {
            highestIdx = trailingIdx;
            highest = inHigh[static_cast<std::size_t>(highestIdx)];
            auto i = highestIdx;
            i++;
            for (auto j = i; j <= today; j++) {
                tmp = inHigh[static_cast<std::size_t>(j)];
                if (tmp >= highest) {
                    highestIdx = j;
                    highest = tmp;
                }
            }
        } else if (tmp >= highest) {
            highestIdx = today;
            highest = tmp;
        }

        outAroonUp[static_cast<std::size_t>(outIdx)] = factor * (inTimePeriod - (today - highestIdx));
        outAroonDown[static_cast<std::size_t>(outIdx)] = factor * (inTimePeriod - (today - lowestIdx));
        outIdx++;
        trailingIdx++;
        today++;
    }

    down = outAroonDown;
    up = outAroonUp;
}

//----------------------------------------------------------------------------------------------------------------------
bool
ATSDSPAlgorithms::donchian(const std::vector<double> &inHigh, const std::vector<double> &inLow, int inTimePeriod,
                           std::vector<double> &upper,
                           std::vector<double> &middle, std::vector<double> &lower) {

    int outBeginIndex = 0;
    int outElementsNumber = 0;
    std::vector<double> maxOut(inHigh.size());
    std::vector<double> minOut(inLow.size());
    upper.resize(inHigh.size());
    middle.resize(inHigh.size());
    lower.resize(inHigh.size());

    TA_RetCode status = TA_MAX(0, static_cast<int>(inHigh.size() - 1), inHigh.data(), inTimePeriod, &outBeginIndex,
                               &outElementsNumber, maxOut.data());

    if (status != TA_RetCode::TA_SUCCESS) {
        return false;
    }

    for (std::size_t i = 0; i < static_cast<std::size_t>(outBeginIndex); i++) {
        upper[i] = 0;
    }

    for (auto i = static_cast<std::size_t>(outBeginIndex); i < maxOut.size(); i++) {
        upper[i] = maxOut[i - static_cast<std::size_t>(outBeginIndex)];
    }

    status = TA_MIN(0, static_cast<int>(inLow.size() - 1), inLow.data(), inTimePeriod, &outBeginIndex,
                    &outElementsNumber, minOut.data());

    if (status != TA_RetCode::TA_SUCCESS) {
        return false;
    }

    for (std::size_t i = 0; i < static_cast<std::size_t>(outBeginIndex); i++) {
        lower[i] = 0;
    }

    for (auto i = static_cast<std::size_t>(outBeginIndex); i < minOut.size(); i++) {
        lower[i] = minOut[i - static_cast<std::size_t>(outBeginIndex)];
    }

    if (upper.size() != lower.size()) {
        return false;
    }

    for (std::size_t i = 0; i < upper.size(); i++) {
        middle[i] = (upper[i] + lower[i]) / 2.0;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
void ATSDSPAlgorithms::alma(const std::vector<double> &in, int inTimePeriod, std::vector<double> &out) {

    if (in.size() < inTimePeriod) {
        for (auto i = 0; i < in.size(); i++) {
            out[i] = 0;
        }
        return;
    }

    auto *data = const_cast<double *>(in.data());

    for (auto n = inTimePeriod; n < in.size(); n++) {
        out[n] = P::ALMACore(data, inTimePeriod);
        data++;
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ATSDSPAlgorithms::mmi(const std::vector<double> &in, int length, std::vector<double> &out) {

}
}
