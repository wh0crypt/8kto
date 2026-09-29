#include "cli.hpp"

#include <cstdlib>
#include <print>

int main(int argc, char *argv[])
{
    const std::span<char *> args{argv, static_cast<std::size_t>(argc)};

    if (argc < 2)
    {
        cli::show_usage();
        return EXIT_FAILURE;
    }

    auto opts = cli::parse_arguments(args);
    if (!opts)
    {
        std::println(stderr, "8kto_emu error: {}", opts.error());
        return EXIT_FAILURE;
    }

    if (opts->show_help)
    {
        cli::show_help();
        return EXIT_SUCCESS;
    }

    auto exec = cli::handle(*opts);
    if (!exec)
    {
        std::println(stderr, "8kto_emu: error: {}", exec.error());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
