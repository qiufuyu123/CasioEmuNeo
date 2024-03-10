# folder names 'bin' and 'obj' are hardcoded

CXX ?= g++
RM  ?= rm -f
CCFLAGS ?= -D_GLIBCXX_DEBUG -O2 -std=c++14 -Wall -Wextra -Werror -pedantic

_dummy := $(shell mkdir -p obj bin)

bin/u8-disas: nX-U8_is.txt bin/main Makefile example.cpp
	@bin/main $< temp.cpp
	@$(CXX) $(CCFLAGS) temp.cpp -o $@
	@$(RM) temp.cpp

bin/u8-disas-split: nX-U8_is_split.txt bin/main Makefile example.cpp
	@bin/main $< temp.cpp
	@$(CXX) $(CCFLAGS) temp.cpp -o $@
	@$(RM) temp.cpp

bin/main: obj/main.o obj/lib.o Makefile
	@$(CXX) obj/main.o obj/lib.o -o $@

obj/%.o: %.cpp lib.h Makefile
	@$(CXX) -c $(CCFLAGS) $< -o $@

clean:
	@$(RM) bin/*
	@$(RM) obj/*
