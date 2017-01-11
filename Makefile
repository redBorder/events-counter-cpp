
include Makefile.config

BIN=events_counter

.PHONY=version.cpp

SRCS_SFLOW_$(WITH_SFLOW) += sflow_collect.c
SRCS= src/main.cpp version.cpp
OBJS= $(SRCS:.cpp=.o)

all: $(BIN)

include mklove/Makefile.base

version.cpp:
	@rm -f $@
	@echo "const char *version=\"`git describe --abbrev=6 --tags HEAD --always`\";" >> $@

install: bin-install

clean: bin-clean
	@echo -e '\033[1;33m[Workdir cleaned]\033[0m\t $<'

-include $(DEPS)
