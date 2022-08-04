#include "ats_utils.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <QFile>
#include "model/processing/ats_processor.h"
#include <QDateTime>
#include <fstream>

namespace trader {

//----------------------------------------------------------------------------------------------------------------------
std::string timeString() {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d_%m_%Y_%H_%M_%S", timeinfo);
    std::string str(buffer);

    return str;
}

//----------------------------------------------------------------------------------------------------------------------
std::string orderToString(const Order &order) {
    std::string retVal;
    retVal = "IB Order, action: " + order.action + ", total quantity: " + std::to_string(order.totalQuantity);
    return retVal;
}

//----------------------------------------------------------------------------------------------------------------------
std::string historicalDataFileNameTemplate(std::int64_t unixTimeStamp, const std::string &dataLength,
                                           const std::string &dataType,
                                           const std::string &symbol,
                                           const std::string &resolution) {
    std::string dateTime = QDateTime::fromSecsSinceEpoch(unixTimeStamp).toString(
            "dd_MM_yyyy_hh_mm_ss").toStdString();
    std::string retVal = symbol + "_" + dataType + "_" + resolution + "_" + dataLength + "_" + dateTime + ".csv";

    retVal.erase(std::remove(retVal.begin(), retVal.end(), ' '), retVal.end());

    return retVal;
}

//----------------------------------------------------------------------------------------------------------------------
std::atomic<TickerId> TickerIdGenerator::s_currentTickerId(1);

//----------------------------------------------------------------------------------------------------------------------
TickerId TickerIdGenerator::currentId() {
    return s_currentTickerId;
}

//----------------------------------------------------------------------------------------------------------------------
TickerId TickerIdGenerator::nextId() {
    s_currentTickerId.fetch_add(1);
    return s_currentTickerId;
}

//----------------------------------------------------------------------------------------------------------------------
/** Calculate determinant of matrix:
    [a b]
    [c d]
*/
double Det(double a, double b, double c, double d) {
    return a * d - b * c;
}

//----------------------------------------------------------------------------------------------------------------------
///Calculate intersection of two lines.
///\return true if found, false if not found or error
bool lineLineIntersect(double x1, double y1, //Line 1 start
                       double x2, double y2, //Line 1 end
                       double x3, double y3, //Line 2 start
                       double x4, double y4, //Line 2 end
                       double &ixOut, double &iyOut) //Output
{
    double detL1 = Det(x1, y1, x2, y2);
    double detL2 = Det(x3, y3, x4, y4);
    double x1mx2 = x1 - x2;
    double x3mx4 = x3 - x4;
    double y1my2 = y1 - y2;
    double y3my4 = y3 - y4;

    double xnom = Det(detL1, x1mx2, detL2, x3mx4);
    double ynom = Det(detL1, y1my2, detL2, y3my4);
    double denom = Det(x1mx2, y1my2, x3mx4, y3my4);
    if (denom == 0.0) { //Lines don't seem to cross
        ixOut = std::numeric_limits<double>::quiet_NaN();
        iyOut = std::numeric_limits<double>::quiet_NaN();
        return false;
    }

    ixOut = xnom / denom;
    iyOut = ynom / denom;
    if (!std::isfinite(ixOut) || !std::isfinite(iyOut)) //Probably a numerical issue
        return false;

    return true; //All OK
}

//----------------------------------------------------------------------------------------------------------------------
bool lineLineIntersectingInRange(double x1, double y1,
                                 double x2, double y2,
                                 double x3, double y3,
                                 double x4, double y4, double &xi, double &yi) {

    if (lineLineIntersect(x1, y1, x2, y2, x3, y3, x4, y4, xi, yi)) {

        bool xIsInRange = (xi < x2 && xi > x1);
        bool yUpIsInRange = isValueInInterval(y3, y4, yi);
        bool yDownIsInRange = isValueInInterval(y1, y2, yi);

        return xIsInRange && yUpIsInRange && yDownIsInRange;
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool isValueInInterval(double y1, double y2, double value) {
    if ((value > y1 && value < y2) || (value < y1 && value > y2)) {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool lineGoesUp(double y1, double y2) {
    return (y2 > y1);
}

//----------------------------------------------------------------------------------------------------------------------
bool lineGoesDown(double y1, double y2) {
    return (y2 < y1);
}

//----------------------------------------------------------------------------------------------------------------------
bool loadTimeSeriesFromCSV(const std::string &path, std::vector<ATSBar> &timeSeries) {
    QFile inFile(QString::fromStdString(path));

    if (inFile.open(QIODevice::ReadOnly)) {

        while (!inFile.atEnd()) {

            QString line = inFile.readLine();
            QStringList values = line.split(',');

            ATSBar bar;

            bool ok = false;
            auto time = values[0].toLongLong(&ok);

            if (ok) {
                bar.m_unixTime = time;
            } else {
                auto dateTime = QDateTime::fromString(values[0], "yyyyMMdd  hh:mm:ss");
                bar.m_unixTime = dateTime.toSecsSinceEpoch();
            }

            bar.m_open = values[1].toDouble();
            bar.m_high = values[2].toDouble();
            bar.m_low = values[3].toDouble();
            bar.m_close = values[4].toDouble();
            bar.m_volume = values[5].toInt();

            timeSeries.push_back(bar);
        }

        inFile.close();
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool saveTimeSeriesToCSV(const std::string &path, const std::vector<ATSBar> &timeSeries) {
    std::ofstream outFile;
    outFile.open(path);

    if (outFile.is_open()) {

        for (auto it = timeSeries.begin(); it != timeSeries.end(); it++) {
            outFile << (*it).m_unixTime << ",";
            outFile << (*it).m_open << ",";
            outFile << (*it).m_high << ",";
            outFile << (*it).m_low << ",";
            outFile << (*it).m_close << ",";
            outFile << (*it).m_volume << std::endl;
        }

        outFile.flush();
        outFile.close();
        return true;
    }

    return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool
groupCandles(const std::vector<double> &highs, const std::vector<double> &opens, const std::vector<double> &closes,
             const std::vector<double> &lows, std::size_t groupingFactor, std::vector<double> &highsOut,
             std::vector<double> &opensOut,
             std::vector<double> &closesOut,
             std::vector<double> &lowsOut) {

    std::size_t N = highs.size();

    if (groupingFactor == 0) {
        return false;
    } else if (groupingFactor == 1) {
        highsOut = highs;
        opensOut = opens;
        closesOut = closes;
        lowsOut = lows;
        return true;
    }

    if (N % groupingFactor > 0) {
        return false;
    }

    std::size_t groupedN = N / groupingFactor;

    std::vector<double> groupedHighs(groupedN);
    std::vector<double> groupedOpens(groupedN);
    std::vector<double> groupedCloses(groupedN);
    std::vector<double> groupedLows(groupedN);

    auto lastOfCurrentGroup = groupingFactor - 1;

    std::size_t k = 0;

    for (std::size_t i = 0; i < N; i += groupingFactor) {  // scan all param candles
        groupedOpens[k] = opens[i];
        groupedCloses[k] = closes[i + lastOfCurrentGroup];

        groupedHighs[k] = highs[i];
        groupedLows[k] = lows[i];

        auto endOfCurrentGroup = i + lastOfCurrentGroup;

        for (auto j = i + 1; j <= endOfCurrentGroup; j++) {
            if (lows[j] < groupedLows[k]) {
                groupedLows[k] = lows[j];
            }
            if (highs[j] > groupedHighs[k]) {
                groupedHighs[k] = highs[j];
            }
        }
        k++;
    }

    highsOut = groupedHighs;
    opensOut = groupedOpens;
    closesOut = groupedCloses;
    lowsOut = groupedLows;

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
double angleBetweenVectors(double x1, double y1, //Line 1 start
                           double x2, double y2, //Line 1 end
                           double x3, double y3, //Line 2 start
                           double x4, double y4) //Line 2 end

{

    auto dot = x1 * x2 + y1 * y2;//      # dot product between [x1, y1] and [x2, y2]
    auto det = x1 * y2 - y1 * x2;//      # determinant
    auto angle = atan2(det, dot);


    //    (x1 * x2) + (x3 * x4)


    //    auto dotProduct = std::inner_product(std::begin(u), std::end(u), std::begin(v), 0.0);

    //    double absu = 0.0;
    //    double absv = 0.0;

    //    for(std::size_t i = 0; i < u.size(); i++) {
    //        absu += u[i] * u[i];
    //        absv += v[i] * v[i];
    //    }

    //    absu = std::sqrt(absu);
    //    absv = std::sqrt(absv);

    //    auto cosalpha = dotProduct / (absu * absv);

    return angle;
}

//----------------------------------------------------------------------------------------------------------------------
bool crossover(const std::vector<double> &series1, const std::vector<double> &series2) {
    if (series1.size() < 3 || series2.size() < 3) {
        return false;
    }

    std::size_t N = series1.size();

    return series1[N - 2] <= series2[N - 2] && series1[N - 1] > series2[N - 1];
}

//----------------------------------------------------------------------------------------------------------------------
bool crossunder(const std::vector<double> &series1, const std::vector<double> &series2) {
    if (series1.size() < 3 || series2.size() < 3) {
        return false;
    }

    std::size_t N = series1.size();

    return series1[N - 1] <= series2[N - 1] && series1[N - 2] > series2[N - 2];
}
}
