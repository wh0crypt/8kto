#include "cpu.hpp"

#include <expected>
#include <format>
#include <print>

namespace isa
{

std::expected<Instruction, std::string> Instruction::decode(std::byte raw) noexcept
{
    const auto val = std::to_integer<std::uint8_t>(raw);
    const auto raw_op = static_cast<std::uint8_t>((val >> 4) & 0x0F);

    if (!is_valid_opcode(raw_op))
    {
        return std::unexpected(std::format("illegal opcode 0x{:02X} ({:04b})", raw_op, raw_op));
    }

    return Instruction{
        .opcode = static_cast<OpCode>((val >> 4) & 0x0F),     // bits [7:4]
        .rs = static_cast<Register>((val >> 2) & 0x03),       // bits [3:2]
        .rd = static_cast<Register>(val & 0x03),              // bits [1:0]
        .imm2 = static_cast<std::uint8_t>((val >> 2) & 0x03), // bits [3:2]
        .imm4 = static_cast<std::uint8_t>(val & 0x0F),        // bits [3:0]
        .off = static_cast<std::uint8_t>(val & 0x0F)          // bits [3:0]
    };
}

std::expected<void, std::string> Cpu::load_rom(const Program &p) noexcept
{
    if (static_cast<std::size_t>(p.start_addr()) + p.size() > MAX_MEM_SIZE)
    {
        return std::unexpected(
            std::format(
                "program cannot be loaded at address 0x{:02X} ({:08b}) because it would exceeed "
                "the memory capacity (0x{:02X} + {} > 0x{:02X})",
                p.start_addr(),
                p.start_addr(),
                p.start_addr(),
                p.size(),
                MAX_MEM_SIZE
            )
        );
    }

    this->rom_.fill(std::byte{0});
    std::copy(p.bytes().begin(), p.bytes().end(), this->rom_.begin() + p.start_addr());

    this->reset();
    this->pc_ = p.start_addr();

    return {};
}

void Cpu::write_reg(Register reg, std::byte b) noexcept
{
    if (reg == Register::R0)
    {
        return;
    }

    auto reg_idx = static_cast<std::size_t>(reg);
    this->regs_[reg_idx] = b;
}

std::expected<void, std::string> Cpu::run(bool debug)
{
    while (!this->halted_ && this->pc_ < this->rom_.size())
    {
        if (auto res = this->step(debug); !res)
        {
            return std::unexpected(res.error());
        }
    }

    return {};
}

std::expected<void, std::string> Cpu::step(bool debug)
{
    // fetch
    const auto current_pc = this->pc_;
    std::byte rom_inst = this->rom_[current_pc];
    auto next_pc = static_cast<std::uint8_t>(current_pc + 1);

    // decode
    auto res = Instruction::decode(rom_inst);
    if (!res)
    {
        return std::unexpected(res.error());
    }

    const Instruction inst = *res;
    const auto rs_val = std::to_integer<std::uint8_t>(this->read_reg(inst.rs));
    const auto rd_val = std::to_integer<std::uint8_t>(this->read_reg(inst.rd));

    std::uint8_t result = 0;

    // execute
    switch (inst.opcode)
    {
        case OpCode::ADD:
        {
            result = static_cast<std::uint8_t>(rd_val + rs_val);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::SUB:
        {
            result = static_cast<std::uint8_t>(rd_val - rs_val);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::AND:
        {
            result = static_cast<std::uint8_t>(rd_val & rs_val);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::ORR:
        {
            result = static_cast<std::uint8_t>(rd_val | rs_val);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::XOR:
        {
            result = static_cast<std::uint8_t>(rd_val ^ rs_val);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::NOT:
        {
            result = static_cast<std::uint8_t>(~rs_val);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::ADI:
        {
            const auto delta = sign_extend<2>(inst.imm2);
            result = static_cast<std::uint8_t>(rd_val + delta);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::LUI:
        {
            result = static_cast<std::uint8_t>(inst.imm4 << 4);
            this->write_reg(Register::R3, static_cast<std::byte>(result));
            break;
        }

        case OpCode::LLI:
        {
            const auto r3_val = static_cast<std::uint8_t>(this->read_reg(Register::R3));
            result = static_cast<std::uint8_t>((r3_val & 0xF0) | (inst.imm4 & 0x0F));
            this->write_reg(Register::R3, static_cast<std::byte>(result));
            break;
        }

        case OpCode::LDM:
        {
            this->write_reg(inst.rd, this->read_mem(rs_val));
            break;
        }

        case OpCode::STM:
        {
            this->ram_[static_cast<std::size_t>(rd_val)] = this->read_reg(inst.rs);
            break;
        }

        case OpCode::JMP:
        {
            next_pc = rd_val;
            break;
        }

        case OpCode::JEZ:
        {
            if (rs_val == 0)
            {
                next_pc = rd_val;
            }
            break;
        }

        case OpCode::JNZ:
        {
            if (rs_val != 0)
            {
                next_pc = rd_val;
            }
            break;
        }

        case OpCode::JRE:
        {
            next_pc = static_cast<std::uint8_t>(current_pc + sign_extend<4>(inst.off));
            break;
        }

        case OpCode::HLT:
        {
            this->halted_ = true;
            break;
        }

        default:
        {
            auto opcode_int = static_cast<std::uint8_t>(inst.opcode);
            return std::unexpected(
                std::format("invalid instruction 0x{:01x} ({:04b})", opcode_int, opcode_int)
            );
        }
    }

    if (debug)
    {
        this->debug_inst(inst);
    }

    this->pc_ = next_pc;

    return {};
}

void Cpu::debug_inst(const Instruction &inst) const noexcept
{
    std::print(
        "[DEBUG] $pc=0x{:02X} $r1={:02X} $r2={:02X} $r3={:02X} off=0x{:01X} inst: ",
        this->pc_,
        std::to_integer<std::uint8_t>(this->read_reg(Register::R1)),
        std::to_integer<std::uint8_t>(this->read_reg(Register::R2)),
        std::to_integer<std::uint8_t>(this->read_reg(Register::R3)),
        inst.off
    );

    switch (inst.opcode)
    {
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::AND:
        case OpCode::ORR:
        case OpCode::XOR:
        case OpCode::NOT:
        case OpCode::JEZ:
        case OpCode::JNZ:
            std::println(
                "{} {}, {}",
                to_string(inst.opcode),
                to_string(inst.rs),
                to_string(inst.rd)
            );
            return;
        case OpCode::ADI:
            std::println("ADI {}, {}", sign_extend<2>(inst.imm2), to_string(inst.rd));
            return;
        case OpCode::LUI:
            std::println("LUI 0x{:01X}", inst.imm4);
            return;
        case OpCode::LLI:
            std::println("LLI 0x{:01X}", inst.imm4);
            return;
        case OpCode::LDM:
            std::println("LDM [{}], {}", to_string(inst.rs), to_string(inst.rd));
            return;
        case OpCode::STM:
            std::println("STM {}, [{}]", to_string(inst.rs), to_string(inst.rd));
            return;
        case OpCode::JMP:
            std::println("JMP {}", to_string(inst.rd));
            return;
        case OpCode::JRE:
            std::println("JRE {}", sign_extend<4>(inst.off));
            return;
        case OpCode::HLT:
            std::println("HLT");
            return;
        default:
            std::println("<UNK>");
    }
}

}; // namespace isa
