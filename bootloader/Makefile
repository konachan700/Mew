DEVICE		= stm32f437vgt6

BINARY		= mew
BOOTPORT	?= /dev/ttyUSB0
BOOTSPEED	?= 115200
LDSCRIPT	= mew.ld
LIBNAME		= opencm3_stm32f4
DEFS		+= -DSTM32F4
# -DEBUG

OBJDIR = ./
INDEPENDENT_HEADERS=

#FP_FLAGS	?= -msoft-float
#ARCH_FLAGS	= -mthumb -mcpu=cortex-m3 $(FP_FLAGS) -mfix-cortex-m3-ldrd -specs=nano.specs -specs=nosys.specs

FP_FLAGS	= -mfloat-abi=hard -mfpu=fpv4-sp-d16
ARCH_FLAGS 	= -mthumb -mcpu=cortex-m4 $(FP_FLAGS) -specs=nano.specs -specs=nosys.specs 


###############################################################################
# Executables
PREFIX		?= arm-none-eabi

RM		:= rm -f
RMDIR		:= rmdir
CC		:= $(PREFIX)-gcc
LD		:= $(PREFIX)-gcc
AR		:= $(PREFIX)-ar
AS		:= $(PREFIX)-as
OBJCOPY		:= $(PREFIX)-objcopy
OBJDUMP		:= $(PREFIX)-objdump
GDB		:= $(PREFIX)-gdb
STFLASH		= $(shell which st-flash)
STBOOT		= $(shell which stm32flash)

###############################################################################
# Source files
LDSCRIPT	?= $(BINARY).ld
#SRC		= $(wildcard *.c) $(wildcard **/*.c)
SRC		= $(shell find . -name "*.c")
ASMSRC		= $(shell find . -name "*.s")
#OBJS		= $(addprefix $(OBJDIR)/, $(SRC:%.c=%.o))
OBJS		= $(addprefix ./, $(SRC:%.c=%.o))
ASMOBJS		= $(addprefix ./, $(ASMSRC:%.s=%.o))

ifeq ($(strip $(OPENCM3_DIR)),)
OPENCM3_DIR := /home/user/Devel/stm32/libopencm3
###/home/misaki/devel/stm32/libopencm3
$(info Using $(OPENCM3_DIR) path to library)
endif

INCLUDE_DIR	= $(OPENCM3_DIR)/include
LIB_DIR		= $(OPENCM3_DIR)/lib
SCRIPT_DIR	= $(OPENCM3_DIR)/scripts

###############################################################################
# AS flags
ASFLAGS		+= -mthumb -mcpu=cortex-m4 $(FP_FLAGS) 
###############################################################################
# C flags
CFLAGS		+= -O2 -g
CFLAGS		+= -Wimplicit-function-declaration
CFLAGS		+= -Wall -Wextra -Wshadow -Wimplicit-function-declaration
CFLAGS		+= -Wredundant-decls -Wstrict-prototypes 
CFLAGS		+= -fno-common -ffunction-sections -fdata-sections
#-Werror -pedantic-errors  -Wmissing-prototypes
CFLAGS  	+= -Wno-sign-compare -Wno-overlength-strings -Wno-unused-function -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-unused-variable
CFLAGS  	+= -Wno-cast-function-type -Wno-type-limits

###############################################################################
# C & C++ preprocessor common flags
CPPFLAGS	+= -MD
CPPFLAGS	+= -Wall -Werror
CPPFLAGS	+= -I. -I$(INCLUDE_DIR) $(DEFS)

###############################################################################
# Linker flags
LDFLAGS		+= --static -nostartfiles -Wl,--start-group -lm -lc -lgcc -lnosys -Wl,--end-group -Wl,-Map=mew.map
LDFLAGS		+= -L$(LIB_DIR)
LDFLAGS		+= -T$(LDSCRIPT)

###############################################################################
# Used libraries
LDLIBS		+= -l$(LIBNAME)
LDLIBS		+= -Wl,--start-group -lm -lc -lgcc -Wl,--end-group

.SUFFIXES: .elf .bin .hex .srec .list .map .images
.SECONDEXPANSION:
.SECONDARY:

ELF		:= $(OBJDIR)/$(BINARY).elf
LIST		:= $(OBJDIR)/$(BINARY).list
BIN		:= $(BINARY).bin
HEX		:= $(BINARY).hex

all: bin

elf: $(ELF)
bin: $(BIN)
hex: $(HEX)
list: $(LIST)

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c
	@printf "  CC      $<\n"
	$(CC) $(CFLAGS) $(CPPFLAGS) $(ARCH_FLAGS) -o $@ -c $<
	
$(OBJDIR)/%.o: %.s
	@printf "  AS      $<\n"
	$(AS) $(ASFLAGS) -o $@ $<

$(SRC) : %.c : %.h $(INDEPENDENT_HEADERS)
	@touch $@
	
$(ASMSRC) : %.s 
	@touch $@

%.h: ;

$(BIN): $(ELF)
	@printf "  OBJCOPY $(BIN)\n"
	$(OBJCOPY) -Obinary $(ELF) $(BIN)
	@echo ""
	@echo ""
	@arm-none-eabi-size mew.elf
	@echo ""
	@echo ""

$(HEX): $(ELF)
	@printf "  OBJCOPY $(HEX)\n"
	$(OBJCOPY) -Oihex $(ELF) $(HEX)

$(LIST): $(ELF)
	@printf "  OBJDUMP $(LIST)\n"
	$(OBJDUMP) -S $(ELF) > $(LIST)

$(ELF): $(OBJDIR) $(OBJS) $(ASMOBJS) $(LDSCRIPT) $(LIB_DIR)/lib$(LIBNAME).a
	@printf "  LD      $(ELF)\n"
	$(LD) $(LDFLAGS) $(ARCH_FLAGS) $(OBJS) $(ASMOBJS) $(LDLIBS) -o $(ELF)

rebuild: clean $(BIN)
	@printf "  REBUILD ALL\n"
	
clean:
	@printf "  CLEAN\n"
	@find . -type f -name '*.o' -delete
	@find . -type f -name '*.d' -delete
	@find . -type f -name '*.elf' -delete
	@find . -type f -name '*.map' -delete
	$(RM) $(BINARY).bin

flash: $(BIN)
	@printf "  FLASH  $(BIN)\n"
	$(STFLASH) write $(BIN) 0x8000000

boot: $(BIN)
	@printf "  LOAD $(BIN) through bootloader\n"
	$(STBOOT) -b$(BOOTSPEED) $(BOOTPORT) -w $(BIN)

.PHONY: clean elf hex list flash boot

#-include $(OBJS:.o=.d)
