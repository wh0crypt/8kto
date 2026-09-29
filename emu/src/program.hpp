#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
#include <vector>

class Program
{
  public:
    explicit Program(std::span<const std::byte> buf = {}, std::uint8_t start_addr = 0)
        : data_(buf.begin(), buf.end()), start_addr_(start_addr)
    {
    }

    static std::expected<Program, std::string> from_file(
        const std::filesystem::path &path
    ) noexcept;

    [[nodiscard]] std::uint8_t start_addr() const noexcept
    {
        return this->start_addr_;
    }

    [[nodiscard]] std::span<const std::byte> bytes() const noexcept
    {
        return this->data_;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return this->data_.size();
    }

  private:
    std::vector<std::byte> data_;
    std::uint8_t start_addr_;
};

#endif // PROGRAM_HPP
