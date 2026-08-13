flags := -std=c++20 -g -MMD -MP
includes := -Iinclude -Ibuild/include
libraries := -L./include/enet/lib -L./include/mysql/lib -lssl -lcrypto -lmariadb

binary := main.out
ifeq ($(OS),Windows_NT)
libraries += -lenet_32 -lws2_32 -lwinmm
binary := main.exe
else # linux
libraries += -lenet
endif

all: $(binary)

sources := main.cpp $(shell find include -type f -name '*.cpp' 2>/dev/null)
objects := $(sources:%.cpp=build/%.o)

$(binary): $(objects)
	g++ $(objects) -o $@ $(libraries)

pch_hpp := include/pch.hpp
pch_gch := build/include/pch.hpp.gch

$(pch_gch): $(pch_hpp)
	@mkdir -p $(dir $@)
	g++ $(flags) $(includes) -x c++-header $< -o $@

build/%.o: %.cpp $(pch_gch)
	@mkdir -p $(dir $@)
	g++ $(flags) $(includes) -include $(pch_hpp) -c $< -o $@

-include $(objects:.o=.d)

.PHONY : all clean
clean :
	-rm -rf build $(binary)
