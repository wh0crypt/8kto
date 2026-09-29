#ifndef CLI_HPP
#define CLI_HPP

#include <expected>
#include <filesystem>

namespace cli
{

struct ProgramOptions
{
    bool show_help = false;
    std::filesystem::path prog_path = "";
};

[[nodiscard]] std::expected<ProgramOptions, std::string> parse_arguments(
    std::span<char *> args
) noexcept;
[[nodiscard]] std::expected<void, std::string> handle(const ProgramOptions &opts) noexcept;

void show_usage();
void show_help();

}; // namespace cli

#endif // CLI_HPP
