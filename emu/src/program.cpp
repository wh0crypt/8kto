#include "program.hpp"

#include "cpu.hpp"

#include <charconv>
#include <format>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace
{

[[nodiscard]] std::optional<std::byte> parse_binary_word(std::string_view word) noexcept
{
    if (word.size() != 8)
    {
        return std::nullopt;
    }

    std::uint8_t raw = 0;
    auto [ptr, ec] = std::from_chars(word.data(), word.data() + word.size(), raw, 2);

    if (ec != std::errc{} || ptr != word.data() + word.size())
    {
        return std::nullopt;
    }

    return static_cast<std::byte>(raw);
}

} // namespace

std::expected<Program, std::string> Program::from_file(const std::filesystem::path &path) noexcept
{
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
    {
        return std::unexpected(std::format("file '{}' does not exist", path.string()));
    }

    std::ifstream file(path);
    if (!file)
    {
        return std::unexpected(std::format("could not open file '{}' for reading", path.string()));
    }

    std::vector<std::byte> inst;
    inst.reserve(isa::MAX_MEM_SIZE);

    std::string token;
    while (file >> token)
    {
        if (token.starts_with(';'))
        {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        auto byte = parse_binary_word(token);
        if (!byte)
        {
            return std::unexpected(
                std::format("invalid instruction token '{}' in '{}'", token, path.string())
            );
        }

        if (inst.size() >= isa::MAX_MEM_SIZE)
        {
            return std::unexpected(
                std::format(
                    "program in '{}' exceeds max capacity ({} bytes)",
                    path.string(),
                    isa::MAX_MEM_SIZE
                )
            );
        }

        inst.push_back(*byte);
    }

    if (inst.empty())
    {
        return std::unexpected(
            std::format("file '{}' contains no valid instructions", path.string())
        );
    }

    return Program(inst);
}
