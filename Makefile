.PHONY: all emu asm test clean

EMU_DIR   := emu
BUILD_DIR := build
ISA_RULES := isa/8kto_isa.asm
EMU_BIN   := $(BUILD_DIR)/8kto_emu

all: emu

# compiles emulator with CMake
emu:
	@cmake -B $(EMU_DIR)/build -S $(EMU_DIR)
	@cmake --build $(EMU_DIR)/build
	@mkdir -p $(BUILD_DIR)
	@cp $(EMU_DIR)/build/8kto_emu $(EMU_BIN)

# assembles a file (usage: make asm SRC=examples/test.asm NAME=test)
asm:
	@mkdir -p $(BUILD_DIR)
	customasm -f readmemb,width:8 -o $(BUILD_DIR)/$(NAME).txt $(ISA_RULES) $(SRC)

# assembles and runs the test suite
test: emu
	@mkdir -p $(BUILD_DIR)
	@echo "==> Assembling tests/isa_test.asm..."
	@customasm -f readmemb,width:8 -o $(BUILD_DIR)/isa_test.txt $(ISA_RULES) tests/isa_test.asm
	@echo "==> Running test in 8kto_emu..."
	@$(EMU_BIN) -f $(BUILD_DIR)/isa_test.txt

# artifacts cleanup
clean:
	@rm -rf $(EMU_DIR)/build $(BUILD_DIR)