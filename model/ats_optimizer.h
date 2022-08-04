#ifndef ATSOPTIMIZER_H
#define ATSOPTIMIZER_H

#include <QObject>
#include <thirdparty/spimpl.hpp>
#include <QMessageBox>

namespace trader {
class ATSOptimizer : public QObject {

Q_OBJECT

    struct P;
    spimpl::unique_impl_ptr<P> m_p{};

public:
    ATSOptimizer(const std::string &resultsDirPath, QObject *parent);

    ~ATSOptimizer();

public slots:

    void run(const std::string &historicalDataFilePath);

    void stop();

signals:

    void message(const QMessageBox::Icon &icon, const std::string &message);

    void optimizing(bool running, const std::string &historicalDataFilePath);
};
}

#endif // ATSOPTIMIZER_H
