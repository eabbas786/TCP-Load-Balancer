CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pthread

# directories
BUILD_DIR := ./build
SRC_DIR := ./src
TEST_DIR := ./test

# get dependencies
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# object file for server
SER_OBJECTS := $(BUILD_DIR)/src/server.cpp.o

# C++ files that need to be compiled for load_balancer
LB_SRCS = \
    src/load_balancer.cpp \
    src/proxy.cpp \
    src/thread_pool.cpp

# Prepends BUILD_DIR and appends .o to every src file
LB_OBJS := $(LB_SRCS:%=$(BUILD_DIR)/%.o)


# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIR) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))


all: \
	$(BUILD_DIR)/load_balancer \
	$(BUILD_DIR)/server \
	$(BUILD_DIR)/slow_client \
	$(BUILD_DIR)/stress_client \
	$(BUILD_DIR)/concurrent_slow_client \
	$(BUILD_DIR)/large_message


# build step for load_balancer
$(BUILD_DIR)/load_balancer: $(LB_OBJS)
	$(CXX) $^ -o $@

# build step for server
$(BUILD_DIR)/server: $(SER_OBJECTS)
	$(CXX) $^ -o $@

# build each stress test
$(BUILD_DIR)/stress_client: tests/stress_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/slow_client: tests/slow_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/concurrent_slow_client: tests/concurrent_slow_client.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/large_message: tests/large_message.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@



.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

