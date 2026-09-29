#include "cpu.hpp"

#include <expected>
#include <format>

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
        .opcode = static_cast<OpCode>((val >> 4) & 0x0F),
        .rs = static_cast<Register>((val >> 2) & 0x03),
        .rd = static_cast<Register>(val & 0x03),
        .off = static_cast<std::uint8_t>(val & 0x0F)
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
    this->write_pc(p.start_addr());

    return {};
}

void Cpu::write_reg(Register reg, std::byte b) noexcept
{
    if (reg == Register::R0 || reg == Register::R3)
    {
        return;
    }

    auto reg_idx = static_cast<std::size_t>(reg);
    this->regs_[reg_idx] = b;
}

std::expected<void, std::string> Cpu::run()
{
    while (!this->halted_)
    {
        if (auto res = this->step(); !res)
        {
            return std::unexpected(res.error());
        }
    }

    return {};
}

std::expected<void, std::string> Cpu::step()
{
    // fetch
    const auto current_pc = this->pc();
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

        case OpCode::INC:
        {
            result = static_cast<std::uint8_t>(rs_val + 1);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
            break;
        }

        case OpCode::DEC:
        {
            result = static_cast<std::uint8_t>(rs_val - 1);
            this->write_reg(inst.rd, static_cast<std::byte>(result));
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
            next_pc = static_cast<std::uint8_t>(current_pc + sign_extend_4bit(inst.off));
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

    this->write_pc(next_pc);

    return {};
}

}; // namespace isa
