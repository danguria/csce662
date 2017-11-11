CC := gcc
BUILD_DIR ?= ./builds
BIN_DIR ?= ./bin
SRC_ROOT ?= ./src
INCS ?=  -I./include
all: crsd crc

# crsd
SRCS_CRSD := $(shell find $(SRC_ROOT)/crsd -name *.c)
OBJS_CRSD := $(SRCS_CRSD:%.c=$(BUILD_DIR)/%.o)
DEPS_CRSD := $(OBJS_CRSD:.o=.d)

crsd:  $(OBJS_CRSD)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(OBJS_CRSD) $(INCS) -o $(BIN_DIR)/$@ $(LDFLAGS) -pthread

$(BUILD_DIR)/%.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCS) -c $< -o $@


# crc
SRCS_CRC := $(shell find $(SRC_ROOT)/crc -name *.c)
OBJS_CRC := $(SRCS_CRC:%.c=$(BUILD_DIR)/%.o)
DEPS_CRC := $(OBJS_CRC:.o=.d)
crc:  $(OBJS_CRC)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(OBJS_CRC) $(INCS) -o $(BIN_DIR)/$@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(INCS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) -r $(BIN_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p


