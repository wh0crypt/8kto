#include "cli.hpp"

#include "cpu.hpp"
#include "program.hpp"

#include <expected>
#include <filesystem>
#include <print>
#include <system_error>

namespace cli
{

std::expected<ProgramOptions, std::string> parse_arguments(std::span<char *> args) noexcept
{
    if (args.empty())
    {
        return std::unexpected("argument list cannot be empty");
    }

    ProgramOptions options;

    for (auto it = args.begin() + 1; it != args.end(); ++it)
    {
        if (*it == nullptr)
        {
            continue;
        }

        const std::string_view arg = *it;
        if (arg == "--help" || arg == "-h")
        {
            options.show_help = true;
            return options;
        }

        if (arg == "--file" || arg == "-f")
        {
            if (++it == args.end() || *it == nullptr)
            {
                return std::unexpected("flag '--file' requires a file path");
            }

            const std::filesystem::path path = *it;
            std::error_code ec;

            if (!std::filesystem::exists(path, ec))
            {
                if (ec)
                {
                    return std::unexpected(
                        std::format("cannot access file '{}': {}", *it, ec.message())
                    );
                }

                return std::unexpected(std::format("binary file '{}' does not exist", *it));
            }

            if (!std::filesystem::is_regular_file(path, ec))
            {
                return std::unexpected(std::format("'{}' is not a regular file", *it));
            }

            options.prog_path = path;
            continue;
        }

        return std::unexpected(std::format("unknown argument: '{}'", *it));
    }

    if (!options.show_help && options.prog_path.empty())
    {
        return std::unexpected("missing required argument: -f / --file <path>");
    }

    return options;
}

void show_usage()
{
    std::println(
        "Usage: 8kto_emu [-h|--help] -f <path>\n"
        "Try '8kto_emu --help' for more information."
    );
}

void show_help()
{
    std::println(
        "8kto_emu -- a basic emulator for the custom 8kto architecture.\n\n"
        "Usage: 8kto_emu [-h|--help] -f <path>\n\n"
        "OPTIONS:\n"
        "    -f, --file <path>\tText file with binary program to execute.\n"
        "    -h, --help\t\tPrint this message.\n\n"
        "EXAMPLES:\n"
        "    8kto_emu --help\n"
        "    8kto_emu -f test.txt"
    );
}

std::expected<void, std::string> handle(const ProgramOptions &opts) noexcept
{
    try
    {
        auto res = Program::from_file(opts.prog_path);
        if (!res)
        {
            return std::unexpected(res.error());
        }

        const Program &p = *res;
        isa::Cpu cpu;
        if (auto rom_res = cpu.load_rom(p); !rom_res)
        {
            return std::unexpected(rom_res.error());
        }

        if (auto run_res = cpu.run(); !run_res)
        {
            return std::unexpected(run_res.error());
        }
    }
    catch (const std::exception &e)
    {
        return std::unexpected(
            std::format("exception raised during program handling: {}", e.what())
        );
    }

    return {};
}

} // namespace cli
