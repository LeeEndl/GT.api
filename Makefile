CXX := g++
CXXFLAGS := -std=c++20 -g -Iinclude -MMD -MP -Ibuild/include
LDLIBS := -L./include/enet/lib -L./include/mysql/lib -lssl -lcrypto -lmariadb

ifeq ($(OS),Windows_NT)
LDLIBS += -lenet_32 -lws2_32 -lwinmm
else # linux
LDLIBS += -lenet
endif

all: main.out

sources := main.cpp $(shell find include -type f -name '*.cpp' 2>/dev/null)
objects := $(sources:%.cpp=build/%.o)

pch_hpp := include/pch.hpp
pch_gch := build/include/pch.hpp.gch

main.out: $(objects)
	$(CXX) $(objects) -o $@ $(LDLIBS)

$(pch_gch): $(pch_hpp)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -x c++-header $< -o $@

build/%.o: %.cpp $(pch_gch)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -include $(pch_hpp) -c $< -o $@

-include $(objects:.o=.d)

.PHONY : all clean
clean :
	-rm -rf build main.out
