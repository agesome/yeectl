#include <string_view>
#include <vector>
#include <utility>
#include <optional>

namespace util
{

static auto split_lines(std::string_view view)
{
    std::vector<std::string_view> lines;
    auto newline = view.find_first_of('\n');
    for (size_t offset = 0; newline != view.npos; newline = view.find_first_of('\n', offset))
    {
        lines.emplace_back(view.substr(offset, newline - offset));
        offset = newline + 1;
    }
    return lines;
}

// split key:value and clean any whitespace
static auto split_key_and_value(std::string_view line) -> std::optional<std::tuple<std::string_view, std::string_view>>
{
    const auto sep = line.find_first_of(':');
    if (sep == std::string_view::npos)
    {
        return {};
    }
    auto key = line.substr(0, sep);
    auto vstart = line.find_first_not_of(' ', sep + 1);
    if (vstart == std::string_view::npos)
    {
        return {};
    }
    auto value = line.substr(vstart, line.find_first_of('\r') - vstart);
    if (value.empty())
    {
        return {};
    }
    return std::make_tuple(key, value);
}

}
