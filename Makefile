.PHONY: all asm clean

EMU_DIR   := emu
BUILD_DIR := build
ISA_RULES := isa/8kto_isa.asm

all: asm

# assembles a file (usage: make asm SRC=examples/test.asm NAME=test)
asm:
	@mkdir -p $(BUILD_DIR)
	customasm -f readmemb,width:8 -o $(BUILD_DIR)/$(NAME).txt $(ISA_RULES) $(SRC)

# artifacts cleanup
clean:
	@rm -rf $(BUILD_DIR)