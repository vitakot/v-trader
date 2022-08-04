#ifndef ATS_UTILS_H
#define ATS_UTILS_H

#include <string>
#include "thirdparty/IB/client/CommonDefs.h"
#include <atomic>
#include <list>
#include <mutex>
#include <map>
#include <functional>
#include "thirdparty/nlohmann/nlohmann_fwd.hpp"
#include <queue>
#include "model/ats_order_request.h"

class Order;

namespace trader {

struct ATSBar;

bool loadTimeSeriesFromCSV(const std::string &path, std::vector<ATSBar> &timeSeries);

bool saveTimeSeriesToCSV(const std::string &path, const std::vector<ATSBar> &timeSeries);

std::string timeString();

std::string orderToString(const Order &order);

std::string historicalDataFileNameTemplate(std::int64_t unixTimeStamp, const std::string &dataLength,
                                           const std::string &dataType,
                                           const std::string &symbol,
                                           const std::string &resolution);

bool parseHistoricalDataFIleName(std::int64_t unixTimeStamp, std::string &dataLength, std::string &dataType,
                                 std::string &symbol,
                                 std::string &resolution);

//----------------------------------------------------------------------------------------------------------------------
template<typename ValueType>
bool readValue(const nlohmann::json &json, const std::string &key, ValueType &value) {
    nlohmann::json::const_iterator it;

    it = json.find(key);

    if (it != json.end()) {
        value = it.value();
        return true;

    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------
class TickerIdGenerator {
    static std::atomic<TickerId> s_currentTickerId;
public:

    static TickerId currentId();

    static TickerId nextId();
};

//----------------------------------------------------------------------------------------------------------------------
class IBCleaner {
public:
    std::map<TickerId, std::function<void()>> m_cleaners;

    bool exists(TickerId tickerId) {
        auto iter = m_cleaners.find(tickerId);

        if (iter != m_cleaners.end()) {
            return true;
        }
        return false;
    }

    void clean(TickerId tickerId) {
        auto iter = m_cleaners.find(tickerId);

        if (iter != m_cleaners.end()) {
            iter->second();
            m_cleaners.erase(iter);
        }
    }

    void cleanAll() {
        for (auto &item : m_cleaners) {
            item.second();
        }

        m_cleaners.clear();
    }
};

//----------------------------------------------------------------------------------------------------------------------
template<typename T>
class SafeQueue {

protected:
    std::list<T> m_queue;
    std::recursive_mutex m_mutex;

public:

    void push(const T &value) {
        m_mutex.lock();
        m_queue.push_back(value);
        m_mutex.unlock();
    }

    T pop() {
        m_mutex.lock();
        auto value = m_queue.pop_front();
        m_mutex.unlock();
        return value;
    }

    std::size_t size() {
        m_mutex.lock();
        auto sz = m_queue.size();
        m_mutex.unlock();
        return sz;
    }
};

//----------------------------------------------------------------------------------------------------------------------
class ATSOrderRequestQueue : public SafeQueue<ATSOrderRequest> {

public:
    bool removeRequest(std::int64_t orderId, ATSOrderRequest &orderRequest) {
        m_mutex.lock();

        for (auto it = m_queue.begin(); it != m_queue.end(); it++) {
            if ((*it).orderId() == orderId) {
                orderRequest = *it;
                m_queue.erase(it);
                m_mutex.unlock();
                return true;
            }
        }

        m_mutex.unlock();
        return false;
    }

    bool containsRequest(std::int64_t orderId, ATSOrderRequest &orderRequest) {
        m_mutex.lock();

        for (auto &it : m_queue) {
            if (it.orderId() == orderId) {
                orderRequest = it;
                m_mutex.unlock();
                return true;
            }
        }

        m_mutex.unlock();
        return false;
    }
};

//----------------------------------------------------------------------------------------------------------------------
double Det(double a, double b, double c, double d);

//----------------------------------------------------------------------------------------------------------------------
bool lineLineIntersect(double x1, double y1, //Line 1 start
                       double x2, double y2, //Line 1 end
                       double x3, double y3, //Line 2 start
                       double x4, double y4, //Line 2 end
                       double &ixOut, double &iyOut); //Output

//----------------------------------------------------------------------------------------------------------------------
bool lineLineIntersectingInRange(double x1, double y1,
                                 double x2, double y2,
                                 double x3, double y3,
                                 double x4, double y4,
                                 double &xi, double &yi);

//----------------------------------------------------------------------------------------------------------------------
bool isValueInInterval(double y1, double y2, double value);

bool lineGoesUp(double y1, double y2);

bool lineGoesDown(double y1, double y2);

//----------------------------------------------------------------------------------------------------------------------
bool
groupCandles(const std::vector<double> &highs, const std::vector<double> &opens, const std::vector<double> &closes,
             const std::vector<double> &lows, std::size_t groupingFactor, std::vector<double> &highsOut,
             std::vector<double> &opensOut,
             std::vector<double> &closesOut,
             std::vector<double> &lowsOut);

//----------------------------------------------------------------------------------------------------------------------
double angleBetweenVectors(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

bool crossover(const std::vector<double> &series1, const std::vector<double> &series2);

bool crossunder(const std::vector<double> &series1, const std::vector<double> &series2);
}
#endif // ATS_UTILS_H
