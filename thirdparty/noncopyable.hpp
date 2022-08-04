
#ifndef NONCOPYABLE_HPP
#define NONCOPYABLE_HPP

#if _MSC_FULL_VER < 180020827 // MSVC < 2013
#define NO_CXX11
#endif

namespace trader {

namespace noncopyable_ { // protection from unintended Argument-dependent lookup

class noncopyable {

protected:
#if !defined(NO_CXX11)
    constexpr noncopyable() = default;
    ~noncopyable() = default;
#else
    noncopyable() {}
    ~noncopyable() {}
#endif
#if !defined(NO_CXX11)
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
#else
private:
    noncopyable(const noncopyable &);
    noncopyable &operator=(const noncopyable &);
#endif
};
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace Tescan

#endif  // NONCOPYABLE_HPP
