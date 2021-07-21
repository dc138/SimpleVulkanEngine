CXX      := g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -std=c++17
LDFLAGS  := -L/usr/lib -lstdc++ -lglfw -lrt -lm -ldl -lvulkan

FLAGS_RELEASE := -Ofast -flto -Werror -DNDEBUG
FLAGS_DEBUG   := -O0 -g -D_DEBUG

GLSLC := glslc

BUILD_DIR   := ./build
OBJECT_DIR  := $(BUILD_DIR)
BINARY_DIR  := $(BUILD_DIR)/bin
SOURCE_DIR  := src/
INCLUDE_DIR := include/
SHADER_DIR  := shaders/

TARGET   := svke-sample
SRC      := $(shell find $(SOURCE_DIR) $(INCLUDE_DIR) -type f -iname "*.cpp")
OBJECTS  := $(SRC:%.cpp=$(OBJECT_DIR)/%.o)
PCH      := $(shell find $(INCLUDE_DIR) -type f -iwholename "*pch.hpp" | head -n 1)
CPCH     := $(PCH:%.hpp=%.hpp.gch)

FSHADERS  := $(shell find $(SHADER_DIR) -type f -iname "*.frag")
FSPIRV    := $(FSHADERS:%.frag=$(BINARY_DIR)/%.frag.spv)
VSHADERS  := $(shell find $(SHADER_DIR) -type f -iname "*.vert")
VSPIRV    := $(VSHADERS:%.vert=$(BINARY_DIR)/%.vert.spv)

.NOTPARALLEL:
.PHONY: all clean debug release run
all: release

$(OBJECT_DIR)/%.o: %.cpp
	@if [ -d "$(dir $@)" ]; then :; else mkdir -p $(dir $@) \
	  && echo -e "[\033[34mMKDIR\033[0m] $(dir $@)"; fi
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@ $(LDFLAGS) \
	  && echo -e "[\033[32mCXX\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m"

$(BINARY_DIR)/$(TARGET): $(OBJECTS)
	@if [ -d "$(dir $@)" ]; then :; else mkdir -p $(dir $@) \
	  && echo -e "[\033[34mMKDIR\033[0m] $(dir $@)"; fi
	@$(CXX) $(CXXFLAGS) -o $(BINARY_DIR)/$(TARGET) $^ $(LDFLAGS) \
	  && echo -e "[\033[32mLD\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m"

$(BINARY_DIR)/%.frag.spv: %.frag
	@if [ -d "$(dir $@)" ]; then :; else mkdir -p $(dir $@) \
	  && echo -e "[\033[34mMKDIR\033[0m] $(dir $@)"; fi
	@$(GLSLC) $< -o $@ \
	  && echo -e "[\033[32mGLSLC\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m"

$(BINARY_DIR)/%.vert.spv: %.vert
	@if [ -d "$(dir $@)" ]; then :; else mkdir -p $(dir $@) \
	  && echo -e "[\033[34mMKDIR\033[0m] $(dir $@)"; fi
	@$(GLSLC) $< -o $@ \
	  && echo -e "[\033[32mGLSLC\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m"

$(CPCH): $(PCH)
	@$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@ $(LDFLAGS) \
	  && echo -e "[\033[32mCXX\033[0m] \033[1m$^\033[0m -> \033[1m$@\033[0m"

internal_debug_prep:
	@echo -e "[\033[34mINFO\033[0m] Doing a debug build"
	$(eval CXXFLAGS += $(FLAGS_DEBUG))

internal_release_prep:
	@echo -e "[\033[34mINFO\033[0m] Doing a release build"
	$(eval CXXFLAGS += $(FLAGS_RELEASE))

internal_perform_build: $(CPCH) $(BINARY_DIR)/$(TARGET) $(FSPIRV) $(VSPIRV)

release: internal_release_prep internal_perform_build
debug: internal_debug_prep internal_perform_build

run: 
	@echo -e "[\033[34mRUN\033[0m] $(BINARY_DIR)/$(TARGET)"
	@cd $(BINARY_DIR); ./$(TARGET)

clean:
	@echo -e "[\033[34mINFO\033[0m] Cleaning build output"
	-@if [ -d "$(BUILD_DIR)" ]; then rm -rfv $(BUILD_DIR) > /dev/null \
	  && echo -e "[\033[34mRM\033[0m] $(BUILD_DIR)"; fi
