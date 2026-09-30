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
    ADI = 0b0110,
    LUI = 0b0111,
    LLI = 0b1000,

    // memory instructions
    LDM = 0b1001,
    STM = 0b1010,

    // control instructions
    JMP = 0b1011,
    JEZ = 0b1100,
    JNZ = 0b1101,
    JRE = 0b1110,
    HLT = 0b1111
};

struct Instruction
{
    OpCode opcode = OpCode::HLT;
    Register rs = Register::R0;
    Register rd = Register::R0;
    std::uint8_t imm2 = 0b00;
    std::uint8_t imm4 = 0b0000;
    std::uint8_t off = 0b0000;

    [[nodiscard]] static std::expected<Instruction, std::string> decode(std::byte raw) noexcept;
};

class Cpu
{
  public:
    Cpu() : halted_(false), pc_(), regs_(), rom_(), ram_()
    {
    }

    [[nodiscard]] std::expected<void, std::string> load_rom(const Program &p) noexcept;

    [[nodiscard]] std::uint8_t pc() const noexcept
    {
        return this->pc_;
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

    [[nodiscard]] std::expected<void, std::string> run(bool debug = false);
    [[nodiscard]] std::expected<void, std::string> step(bool debug = false);

    void reset()
    {
        this->halted_ = false;
        this->regs_.fill(std::byte{0});
        this->ram_.fill(std::byte{0});
    }

  private:
    bool halted_;
    std::uint8_t pc_;
    std::array<std::byte, REG_NUM> regs_;
    std::array<std::byte, MAX_MEM_SIZE> rom_;
    std::array<std::byte, MAX_MEM_SIZE> ram_;

    void debug_inst(const Instruction &inst) const noexcept;
    void write_pc(std::uint8_t pc)
    {
        this->pc_ = pc;
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
        case OpCode::ADI:
        case OpCode::LUI:
        case OpCode::LLI:
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

template <std::size_t Bits>
[[nodiscard]] constexpr std::int8_t sign_extend(std::uint8_t value) noexcept
{
    static_assert(Bits > 0 && Bits <= 8, "bits must be between 1 and 8");
    constexpr std::size_t shift = 8 - Bits;
    const auto shifted = static_cast<std::int8_t>(value << shift);
    return static_cast<std::int8_t>(shifted >> shift);
}

[[nodiscard]] constexpr std::string_view to_string(OpCode op) noexcept
{
    switch (op)
    {
        case OpCode::ADD:
            return "ADD";
        case OpCode::SUB:
            return "SUB";
        case OpCode::AND:
            return "AND";
        case OpCode::ORR:
            return "ORR";
        case OpCode::XOR:
            return "XOR";
        case OpCode::NOT:
            return "NOT";
        case OpCode::ADI:
            return "ADI";
        case OpCode::LUI:
            return "LUI";
        case OpCode::LLI:
            return "LLI";
        case OpCode::LDM:
            return "LDM";
        case OpCode::STM:
            return "STM";
        case OpCode::JMP:
            return "JMP";
        case OpCode::JEZ:
            return "JEZ";
        case OpCode::JNZ:
            return "JNZ";
        case OpCode::JRE:
            return "JRE";
        case OpCode::HLT:
            return "HLT";
        default:
            return "<unknown>";
    }
}

[[nodiscard]] constexpr std::string_view to_string(Register reg) noexcept
{
    switch (reg)
    {
        case Register::R0:
            return "$r0";
        case Register::R1:
            return "$r1";
        case Register::R2:
            return "$r2";
        case Register::R3:
            return "$r3";
        default:
            return "<unknown>";
    }
}

}; // namespace isa

#endif // CPU_HPP
