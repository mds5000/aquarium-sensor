APP=sensors
CFG=debug
CHIP=saml21e18b
CHIP_CAP  = $(shell echo $(CHIP) | tr a-z A-Z)

SOURCE_DIRS=src linker rtt
INCLUDES=-I"include/" -I"rtt/"
BUILD_DIR=build
OBJECT_DIR=$(BUILD_DIR)/obj
BINARY_DIR=$(BUILD_DIR)/bin
DEPENDENCY_DIR=$(BUILD_DIR)/dep

LDLIBS=-Wl,--start-group -lm -Wl,--end-group
LDFLAGS=-Wl,--no-wchar-size-warning \
		-mthumb \
		-Wl,-Map="$(BUILD_DIR)/$(APP).map" \
		--specs=nano.specs \
		--specs=nosys.specs \
		-Wl,--gc-sections \
		-mcpu=cortex-m0plus \
		-T"linker/$(CHIP)_flash.ld"

CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++

CXFLAGS=-x c -mthumb -ffunction-sections -mlong-calls -Wall -std=gnu99 -D__$(CHIP_CAP)__ -mcpu=cortex-m0plus #-DDONT_USE_CMSIS_INIT
CXXFLAGS=-x c++ -mthumb -fno-exceptions -ffunction-sections -mlong-calls -Wall -std=c++14 -D__$(CHIP_CAP)__ -mcpu=cortex-m0plus #-DDONT_USE_CMSIS_INIT

CFLAGS=$(CXFLAGS) $(INCLUDES) -c
CPPFLAGS=$(CXXFLAGS) $(INCLUDES) -c

# Add optimization
ifeq ($(CFG),release)
CFLAGS += -O1
CPPFLAGS += -O1
endif

# Add debugging symbols
ifeq ($(CFG),debug)
CFLAGS += -O1 -g -DDEBUG
CPPFLAGS += -O1 -g -DDEBUG
endif

ifneq ($(CFG),debug)
ifneq ($(CFG),release)
$(error Bad build configuration.  Choices are debug, release)
endif
endif

DIR_GUARD=@mkdir -p $(@D)

# Generate object and dependency lists
COBJECTS=$(foreach sdir,$(SOURCE_DIRS),$(patsubst $(sdir)/%.c,$(OBJECT_DIR)/$(sdir)/%.o,$(shell find $(sdir) -type f -name "*.c")))
CPPOBJECTS=$(foreach sdir,$(SOURCE_DIRS),$(patsubst $(sdir)/%.cpp,$(OBJECT_DIR)/$(sdir)/%.o,$(shell find $(sdir) -type f -name "*.cpp")))
OBJECTS=$(COBJECTS) $(CPPOBJECTS)

CDEPENDENCIES=$(foreach sdir,$(SOURCE_DIRS),$(patsubst $(sdir)/%.c,$(DEPENDENCY_DIR)/$(SOURCE_DIR)/%.d,$(shell find $(sdir) -type f -name "*.c")))
CPPDEPENDENCIES=$(foreach sdir,$(SOURCE_DIRS),$(patsubst $(sdir)/%.cpp,$(DEPENDENCY_DIR)/$(SOURCE_DIR)/%.d,$(shell find $(sdir) -type f -name "*.cpp")))
DEPENDENCIES=$(CDEPENDENCIES) $(CPPDEPENDENCIES)

.PHONY: all clean program

all: $(BINARY_DIR)/$(APP).elf

clean:
	rm -rf $(BUILD_DIR)

$(BINARY_DIR)/flash.jlink:
	printf "\nconnect\nloadbin $(BINARY_DIR)/$(APP).bin, 0\nr\ng\nexit\n" > $@

program: $(BINARY_DIR)/flash.jlink
	JLinkExe -device at$(CHIP) -if SWD -speed 4000 $<

$(BINARY_DIR)/$(APP).elf: $(OBJECTS)
	$(DIR_GUARD)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	@echo Finished building target: $@
	"arm-none-eabi-objcopy" -O binary "$(BINARY_DIR)/$(APP).elf" "$(BINARY_DIR)/$(APP).bin"
	"arm-none-eabi-objcopy" -O ihex -R .eeprom -R .fuse -R .lock -R .signature "$(BINARY_DIR)/$(APP).elf" "$(BINARY_DIR)/$(APP).hex"
	"arm-none-eabi-objcopy" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma \
		.eeprom=0 --no-change-warnings -O binary "$(BINARY_DIR)/$(APP).elf" "$(BINARY_DIR)/$(APP).eep" || exit 0
	"arm-none-eabi-objdump" -h -S -l "$(BINARY_DIR)/$(APP).elf" > "$(BINARY_DIR)/$(APP).lss"
	"arm-none-eabi-size" -A "$(BINARY_DIR)/$(APP).elf"

# C Files
$(OBJECT_DIR)/%.o: %.c
	$(DIR_GUARD)
	$(CC) $(CFLAGS) $< -o $@

# CPP files
$(OBJECT_DIR)/%.o: %.cpp
	$(DIR_GUARD)
	$(CXX) $(CPPFLAGS) $< -o $@

# Automatic Dependency Generation
$(DEPENDENCY_DIR)/%.d: %.c
	$(DIR_GUARD)
	$(CC) $(CFLAGS) -MM $< | sed -e 's!\(.*\)\.o:!$@ $(OBJECT_DIR)/$(<D)/\1.o:!' > $@

$(DEPENDENCY_DIR)/%.d: %.cpp
	$(DIR_GUARD)
	$(CXX) $(CPPFLAGS) -MM $< | sed -e 's!\(.*\)\.o:!$@ $(OBJECT_DIR)/$(<D)/\1.o:!' > $@

