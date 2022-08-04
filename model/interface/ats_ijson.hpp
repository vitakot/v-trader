//
// Created by Vitezslav Kot on 18.04.20.
//

#ifndef TRADER_ATS_IJSON_HPP
#define TRADER_ATS_IJSON_HPP

#include "thirdparty/nlohmann/nlohmann_fwd.hpp"

namespace trader {

struct ATSIJson {

    virtual ~ATSIJson() = default;

    /**
     * Serialize object to JSON object
     * @throws nlohmann::json::exception
     * @return
     */
    [[nodiscard]] virtual nlohmann::json toJson() const = 0;

    /**
     * Deserialize object from JSON object
     * @param json
     * @return
     */
    virtual bool fromJson(nlohmann::json json) = 0;
};

}
#endif //TRADER_ATS_IJSON_HPP
