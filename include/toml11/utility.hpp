#ifndef TOML11_UTILITY_HPP
#define TOML11_UTILITY_HPP

#include "result.hpp"
#include "traits.hpp"

#include <array>
#include <sstream>

#include <cassert>
#include <cctype>
#include <cstring>

namespace toml
{
namespace detail
{

// to output character in an error message.
inline std::string show_char(const int c)
{
    using char_type = unsigned char;
    if(std::isgraph(c))
    {
        return std::string(1, static_cast<char>(c));
    }
    else
    {
        std::array<char, 5> buf;
        buf.fill('\0');
        const auto r = std::snprintf(buf.data(), buf.size(), "0x%02x", c & 0xFF);
        assert(r == static_cast<int>(buf.size()) - 1);
        (void) r; // Unused variable warning
        auto in_hex = std::string(buf.data());
        switch(c)
        {
            case char_type('\0'):   {in_hex += "(NUL)";             break;}
            case char_type(' ') :   {in_hex += "(SPACE)";           break;}
            case char_type('\n'):   {in_hex += "(LINE FEED)";       break;}
            case char_type('\r'):   {in_hex += "(CARRIAGE RETURN)"; break;}
            case char_type('\t'):   {in_hex += "(TAB)";             break;}
            case char_type('\v'):   {in_hex += "(VERTICAL TAB)";    break;}
            case char_type('\f'):   {in_hex += "(FORM FEED)";       break;}
            case char_type('\x1B'): {in_hex += "(ESCAPE)";          break;}
            default: break;
        }
        return in_hex;
    }
}

// ---------------------------------------------------------------------------

template<typename Container>
void try_reserve_impl(Container& container, std::size_t N, std::true_type)
{
    container.reserve(N);
    return;
}
template<typename Container>
void try_reserve_impl(Container&, std::size_t, std::false_type) noexcept
{
    return;
}

template<typename Container>
void try_reserve(Container& container, std::size_t N)
{
    try_reserve_impl(container, N, has_reserve_method<Container>{});
    return;
}

// ---------------------------------------------------------------------------

template<typename T>
result<T, none_t> from_string(const std::string& str)
{
    T v;
    std::istringstream iss(str);
    iss >> v;
    if(iss.fail())
    {
        return err();
    }
    return ok(v);
}

// ---------------------------------------------------------------------------

// helper function to avoid std::string(0, 'c') or std::string(iter, iter)
template<typename Iterator>
std::string make_string(Iterator first, Iterator last)
{
    if(first == last) {return "";}
    return std::string(first, last);
}
inline std::string make_string(std::size_t len, char c)
{
    if(len == 0) {return "";}
    return std::string(len, c);
}

// ---------------------------------------------------------------------------

template<typename Char,  typename Traits, typename Alloc,
         typename Char2, typename Traits2, typename Alloc2>
struct to_string_of_impl
{
    static_assert(sizeof(Char)  == sizeof(char), "");
    static_assert(sizeof(Char2) == sizeof(char), "");

    static std::basic_string<Char, Traits, Alloc> invoke(std::basic_string<Char2, Traits2, Alloc2> s)
    {
        std::basic_string<Char, Traits, Alloc> retval;
        std::transform(s.begin(), s.end(), std::back_inserter(retval),
            [](const Char2 c) {return static_cast<Char>(c);});
        return retval;
    }
    template<std::size_t N>
    static std::basic_string<Char, Traits, Alloc> invoke(const Char2 (&s)[N])
    {
        std::basic_string<Char, Traits, Alloc> retval;
        std::transform(std::begin(s), std::end(s), std::back_inserter(retval),
            [](const char c) {return static_cast<Char>(c);});
        return retval;
    }
};

template<typename Char,  typename Traits, typename Alloc>
struct to_string_of_impl<Char, Traits, Alloc, Char, Traits, Alloc>
{
    static_assert(sizeof(Char) == sizeof(char), "");

    static std::basic_string<Char, Traits, Alloc> invoke(std::basic_string<Char, Traits, Alloc> s)
    {
        return s;
    }
    template<std::size_t N>
    static std::basic_string<Char, Traits, Alloc> invoke(const Char (&s)[N])
    {
        return std::basic_string<Char, Traits, Alloc>(s);
    }
};

template<typename Char,
         typename Traits = std::char_traits<Char>,
         typename Alloc = std::allocator<Char>,
         typename Char2, typename Traits2, typename Alloc2>
std::basic_string<Char, Traits, Alloc>
to_string_of(std::basic_string<Char2, Traits2, Alloc2> s)
{
    return to_string_of_impl<Char, Traits, Alloc, Char2, Traits2, Alloc2>::invoke(std::move(s));
}
template<typename Char,
         typename Traits = std::char_traits<Char>,
         typename Alloc = std::allocator<Char>,
         typename Char2, typename Traits2, typename Alloc2, std::size_t N>
std::basic_string<Char, Traits, Alloc> to_string_of(const char (&s)[N])
{
    return to_string_of_impl<Char, Traits, Alloc, Char2, Traits2, Alloc2>::template invoke<N>(s);
}

} // namespace detail
} // namespace toml
#endif // TOML11_UTILITY_HPP
