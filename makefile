TARGET_EXEC ?= program.build

OUT_DIR ?= out
BUILD_DIR ?= build
SRC_DIRS ?= src
LID_DIRS ?= libraries
RSRC_DIRS ?= shaders assets

SRCS := $(shell find $(SRC_DIRS) -name *.cpp)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d) $(shell find $(LID_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := -std=c++17 -stdlib=libc++ -g -O

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ -L$(LID_DIRS) -lglfw3 -lglew -framework Cocoa -framework OpenGL -framework IOKit -lgsl -lgslcblas

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(INC_FLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)/$(SRC_DIRS)

-include $(DEPS)

MKDIR_P ?= mkdir -p
RM ?= rm -f
CXX ?= g++
