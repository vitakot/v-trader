#include "ats_db_client.h"
//#include "mongo/client/dbclient.h"
//#include "mongo/bson/bson.h"
#include <QDebug>

//using namespace mongo;

namespace trader {

struct ATSDBClient::P {
//    DBClientConnection m_mongoClient;
};

//----------------------------------------------------------------------------------------------------------------------
ATSDBClient::ATSDBClient() : m_p(spimpl::make_unique_impl<P>()) {
    // BSONObj p = BSONObjBuilder().append("name", "Joe").append("age", 33).obj();
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSDBClient::init() {
//    try {
//        m_p->m_mongoClient.connect("localhost");
//        qInfo() << "Mongo client connected ok" << "\n";
//    }
//    catch (const mongo::DBException &e) {
//        qCritical() << "Mongo client exception: " << e.what();
//        return false;
//    }

    return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool ATSDBClient::addPosition(const ATSIBPosition &position) {

    return false;
}

}
