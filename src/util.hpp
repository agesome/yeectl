#ifndef YEECTL_UTIL_HPP
#define YEECTL_UTIL_HPP

#include <string_view>
#include <vector>
#include <utility>
#include <optional>
#include <variant>
#include <string>

namespace util
{

std::vector<std::string_view> split_lines(std::string_view view);
// split key:value and clean any whitespace
std::optional<std::tuple<std::string_view, std::string_view>> split_key_and_value(std::string_view line);

template<typename M>
void update_map(M & target, const M & source)
{
    for (const auto & e : source)
    {
        target.insert_or_assign(e.first, e.second);
    }
}

template<class> inline constexpr bool always_false_v = false;

std::string variant_to_string(const auto & v)
{
    return std::visit([](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
        {
            return arg;
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            return std::to_string(arg);
        }
        else
        {
            static_assert(always_false_v<T>, "non-exhaustive visitor!");
        }
    }, v);
}

}

#endif // YEECTL_UTIL_HPP
