#ifndef NLOHMAN_FWD_H_
#define NLOHMAN_FWD_H_

#include <map>
#include <vector>
#include "json.hpp"

//-----------------------------------------------------------------------------------------------------
/// Just a forward declaration to speed up the compilation
namespace nlohmann {

template<typename, typename> struct adl_serializer;

template <
    template<typename U, typename V, typename... Args> class ObjectType,
    template<typename U, typename... Args> class ArrayType,
    class StringType,
    class BooleanType,
    class NumberIntegerType,
    class NumberUnsignedType,
    class NumberFloatType,
    template<typename U> class AllocatorType,
    template<typename T, typename SFINAE = void> class JSONSerializer
    >
class basic_json;

using json = basic_json<
             std::map,
             std::vector,
             std::string, bool, std::int64_t, std::uint64_t, double,
             std::allocator, adl_serializer>;
}
#endif // TSC_NLOHMAN_FWD_H_
