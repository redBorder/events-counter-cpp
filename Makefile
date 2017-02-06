
include Makefile.config

BIN=events_counter
$(BIN): CC=$(CXX)

SRCS_SFLOW_$(WITH_SFLOW) += sflow_collect.c
SRCS= src/main.cpp \
	version.cpp \
	src/utils/json_zerocopy.cpp \
	src/config/config.cpp \
	src/uuid_consumer/kafka_uuid_consumer.cpp \
	src/uuid_counter/uuid_counter.cpp \
	src/uuid_counters_db/uuid_counters_db.cpp \
	src/formatters/json_kafka_message.cpp \
	src/uuid_counters_monitor/kafka_leadership_monitor.cpp \
	src/monitor_producer/kafka_monitor_producer.cpp
OBJS= $(SRCS:.cpp=.o)

TESTS_C = $(wildcard tests/0*.cpp)
TESTS = $(TESTS_C:.cpp=.test)
TESTS_OBJS = $(TESTS:.test=.o)
TESTS_CHECKS_XML = $(TESTS_C:.cpp=.xml)
TESTS_MEM_XML = $(TESTS_C:.cpp=.mem.xml)
TESTS_HELGRIND_XML = $(TESTS_C:.cpp=.helgrind.xml)
TESTS_DRD_XML = $(TESTS_C:.cpp=.drd.xml)
TESTS_VALGRIND_XML = $(TESTS_MEM_XML) $(TESTS_HELGRIND_XML) $(TESTS_DRD_XML)
COV_FILES = $(foreach ext,gcda gcno, $(SRCS:.cpp=.$(ext)) $(TESTS_C:.cpp=.$(ext)))

VALGRIND ?= valgrind
SUPPRESSIONS_FILE ?= tests/valgrind.suppressions
ifneq ($(wildcard $(SUPPRESSIONS_FILE)),)
SUPPRESSIONS_VALGRIND_ARG = --suppressions=$(SUPPRESSIONS_FILE)
endif

.PHONY: version.cpp tests checks memchecks drdchecks helchecks coverage manuf

all: $(BIN)

include mklove/Makefile.base

version.cpp:
	@rm -f $@
	@echo "const char *version=\"`git describe --abbrev=6 --tags HEAD --always`\";" >> $@

install: bin-install

clean: bin-clean
	@echo -e '\033[1;33m[Workdir cleaned]\033[0m\t $<'

run_tests = tests/run_tests.sh $(1) $(TESTS_C:.cpp=)
run_valgrind = $(VALGRIND) --tool=$(1) $(SUPPRESSIONS_VALGRIND_ARG) --xml=yes \
					--xml-file=$(2) $(3)  >/dev/null 2>&1

tests: $(TESTS)
	@$(call run_tests,-cvdh)

checks: $(TESTS_CHECKS_XML)
	@$(call run_tests,-c)

memchecks: $(TESTS_MEM_XML)
	@$(call run_tests,-v)

drdchecks: $(TESTS_DRD_XML)
	@$(call run_tests,-d)

helchecks: $(TESTS_HELGRIND_XML)
	@$(call run_tests,-h)

tests/%.test: CPPFLAGS := -I. $(CPPFLAGS)
tests/%.test: tests/%.o $(filter-out src/main.o, $(OBJS)) tests/TestUtils.o
	@echo -e '\033[1;32m[Building]\033[0m\t $@'
	@$(CXX) $(CPPFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS) -lgtest -lpthread

tests/%.mem.xml: tests/%.test
	@echo -e '\033[1;34m[Checking memory ]\033[0m\t $<'
	-@$(call run_valgrind,memcheck,"$@","./$<")

tests/%.helgrind.xml: tests/%.test
	@echo -e '\033[1;34m[Checking concurrency with HELGRIND]\033[0m\t $<'
	-@$(call run_valgrind,helgrind,"$@","./$<")

tests/%.drd.xml: tests/%.test
	@echo -e '\033[1;34m[Checking concurrency with DRD]\033[0m\t $<'
	-@$(call run_valgrind,drd,"$@","./$<")

tests/%.xml: tests/%.test
	@echo -e '\033[1;34m[Testing ]\033[0m\t $<'
	@./$< --gtest_output="xml:$@" >/dev/null 2>&1

COVERAGE_INFO ?= coverage.info
COVERAGE_OUTPUT_DIRECTORY ?= coverage.out.html
COV_VALGRIND ?= valgrind
COV_GCOV ?= gcov
COV_LCOV ?= lcov

coverage: $(TESTS) checks
	@$(COV_LCOV) --gcov-tool=$(COV_GCOV) -q \
                --rc lcov_branch_coverage=1 \
								--capture \
                --directory ./ --output-file ${COVERAGE_INFO} >/dev/null 2>&1
	@$(COV_LCOV) --remove coverage.info '/app/tests/*' 'include/*' \
								--rc lcov_branch_coverage=1 \
								--compat-libtool \
								--output-file coverage.info >/dev/null 2>&1
	@$(COV_LCOV) --list --rc lcov_branch_coverage=1 coverage.info

coverage-html: coverage
	genhtml --branch-coverage ${COVERAGE_INFO} --output-directory \
				${COVERAGE_OUTPUT_DIRECTORY} > coverage.out

-include $(DEPS)
