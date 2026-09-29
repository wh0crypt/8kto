#ifndef CPU_HPP
#define CPU_HPP

#include "program.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>

namespace isa
{

constexpr std::size_t MAX_MEM_SIZE = 256;
constexpr std::uint8_t REG_NUM = 4;

enum class Register : std::uint8_t
{
    R0 = 0b00,
    R1 = 0b01,
    R2 = 0b10,
    R3 = 0b11
};

enum class OpCode : std::uint8_t
{
    // ALU instructions
    ADD = 0b0000,
    SUB = 0b0001,
    AND = 0b0010,
    ORR = 0b0011,
    XOR = 0b0100,
    NOT = 0b0101,
    INC = 0b0110,
    DEC = 0b0111,

    // memory instructions
    LDM = 0b1000,
    STM = 0b1001,

    // control instructions
    JMP = 0b1010,
    JEZ = 0b1011,
    JNZ = 0b1100,
    JRE = 0b1101,
    HLT = 0b1111
};

struct Instruction
{
    OpCode opcode = OpCode::HLT;
    Register rs = Register::R0;
    Register rd = Register::R0;
    std::uint8_t off = 0b0000;

    [[nodiscard]] static std::expected<Instruction, std::string> decode(std::byte raw) noexcept;
};

class Cpu
{
  public:
    Cpu() : halted_(false), regs_(), rom_(), ram_()
    {
    }

    [[nodiscard]] std::expected<void, std::string> load_rom(const Program &p) noexcept;

    [[nodiscard]] std::uint8_t pc() const noexcept
    {
        return std::to_integer<std::uint8_t>(this->read_reg(Register::R3));
    }

    [[nodiscard]] std::byte read_reg(Register reg) const noexcept
    {
        return this->regs_[static_cast<std::size_t>(reg)];
    }

    [[nodiscard]] std::byte read_mem(std::uint8_t addr) const noexcept
    {
        return this->ram_[static_cast<std::size_t>(addr)];
    }

    void write_reg(Register reg, std::byte b) noexcept;

    [[nodiscard]] std::expected<void, std::string> run();
    [[nodiscard]] std::expected<void, std::string> step();

    void reset()
    {
        this->halted_ = false;
        this->regs_.fill(std::byte{0});
        this->ram_.fill(std::byte{0});
    }

  private:
    bool halted_;
    std::array<std::byte, REG_NUM> regs_;
    std::array<std::byte, MAX_MEM_SIZE> rom_;
    std::array<std::byte, MAX_MEM_SIZE> ram_;

    void write_pc(std::uint8_t pc)
    {
        this->regs_[static_cast<std::size_t>(Register::R3)] = static_cast<std::byte>(pc);
    }
};

[[nodiscard]] constexpr bool is_valid_opcode(std::uint8_t raw_op) noexcept
{
    switch (static_cast<OpCode>(raw_op))
    {
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::AND:
        case OpCode::ORR:
        case OpCode::XOR:
        case OpCode::NOT:
        case OpCode::INC:
        case OpCode::DEC:
        case OpCode::LDM:
        case OpCode::STM:
        case OpCode::JMP:
        case OpCode::JEZ:
        case OpCode::JNZ:
        case OpCode::JRE:
        case OpCode::HLT:
            return true;
        default:
            return false;
    }
}

[[nodiscard]] constexpr std::int8_t sign_extend_4bit(std::uint8_t val_4bit) noexcept
{
    const auto shifted = static_cast<std::int8_t>(val_4bit << 4);
    return static_cast<std::int8_t>(shifted >> 4);
}

}; // namespace isa

#endif // CPU_HPP
