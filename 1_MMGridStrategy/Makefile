#Variables
INSTANCE_NAME=MMGrid# single word, no special characters
STRATEGY_NAME=$(INSTANCE_NAME)Strategy# should be your .cpp  & .h file names, single word, no special characters
START_DATE=2019-01-30# for your backtest
END_DATE=2019-01-30# for your backtest
SYMBOLS=SPY#symbols to backtest

GIT_REPO_URL=https://gitlab.engr.illinois.edu/fin556_algo_market_micro_fall_2023/fin556_algo_fall_2023_group_07/group_07_project.git
GIT_BRANCH=main
REPO_DIR=test_strategies
STRATEGY_DIR=1_MMGridStrategy

ifdef INTEL
    CC=icc
else
    CC=g++
endif

ifdef DEBUG
    CFLAGS=-c -g -fPIC -fpermissive -std=c++11
else
    CFLAGS=-c -fPIC -fpermissive -O3 -std=c++11
endif

LIBPATH=../../../../libs/x64
INCLUDEPATH=../../../../includes
INCLUDES=-I/usr/include -I$(INCLUDEPATH)
LDFLAGS=$(LIBPATH)/libstrategystudio_analytics.a $(LIBPATH)/libstrategystudio.a $(LIBPATH)/libstrategystudio_transport.a $(LIBPATH)/libstrategystudio_marketmodels.a $(LIBPATH)/libstrategystudio_utilities.a $(LIBPATH)/libstrategystudio_flashprotocol.a

# Define the names of the library, source, and header files.
LIBRARY=$(STRATEGY_NAME).so
SOURCES=$(STRATEGY_NAME).cpp
HEADERS=$(STRATEGY_NAME).h
OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: all make_executable delete_instance clean_dlls move_strategy_dll build_strategy run_backtest

all: clean $(HEADERS) $(LIBRARY)

$(LIBRARY) : $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(LIBRARY).1 -o $(LIBRARY) $(OBJECTS) $(LDFLAGS)
	
.cpp.o: $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf *.o $(LIBRARY)

git_pull:
	@chmod +x ./provision_scripts/git_pull.sh
	./provision_scripts/git_pull.sh "$(REPO_DIR)" "$(GIT_REPO_URL)" "$(GIT_BRANCH)"

make_executable: git_pull
	@chmod +x ./provision_scripts/*.sh
	@chmod +x ./provision_scripts/*.py
	ls -l ./provision_scripts/

delete_instance: make_executable
	pypy3 ./provision_scripts/delete_instance.py "$(INSTANCE_NAME)" "$(STRATEGY_NAME)"

clean_dlls:
	@mkdir -p /home/vagrant/ss/bt/strategies_dlls
	@rm -rf /home/vagrant/ss/bt/strategies_dlls/*

move_strategy_dll: delete_instance clean_dlls all
	@cp $(LIBRARY) /home/vagrant/ss/bt/strategies_dlls/.

build_strategy: move_strategy_dll
	./provision_scripts/build_strategy.sh "$(INSTANCE_NAME)" "$(STRATEGY_NAME)" "$(SYMBOLS)"

run_backtest: build_strategy
	chmod +x ./provision_scripts/run_backtest.sh
	./provision_scripts/run_backtest.sh "$(INSTANCE_NAME)" "$(START_DATE)" "$(END_DATE)" "$(REPO_DIR)" "$(STRATEGY_DIR)"


ifdef INTEL
    CC=icc
else
    CC=g++
endif

ifdef DEBUG
    CFLAGS=-c -g -fPIC -fpermissive -std=c++11
else
    CFLAGS=-c -fPIC -fpermissive -O3 -std=c++11
endif

LIBPATH=../../../../libs/x64
INCLUDEPATH=../../../../includes
INCLUDES=-I/usr/include -I$(INCLUDEPATH)
LDFLAGS=$(LIBPATH)/libstrategystudio_analytics.a $(LIBPATH)/libstrategystudio.a $(LIBPATH)/libstrategystudio_transport.a $(LIBPATH)/libstrategystudio_marketmodels.a $(LIBPATH)/libstrategystudio_utilities.a $(LIBPATH)/libstrategystudio_flashprotocol.a

OBJECTS=$(SOURCES:.cpp=.o)

.PHONY: all make_executable delete_instance clean_dlls move_strategy_dll build_strategy run_backtest

all: clean $(HEADERS) $(LIBRARY)

$(LIBRARY) : $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(LIBRARY).1 -o $(LIBRARY) $(OBJECTS) $(LDFLAGS)
	
.cpp.o: $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	rm -rf *.o $(LIBRARY)

git_pull:
	chmod +x ./provision_scripts/git_pull.sh
	./provision_scripts/git_pull.sh

make_executable: git_pull
	chmod +x ./provision_scripts/*.sh
	chmod +x ./provision_scripts/*.py
	ls -l ./provision_scripts/

delete_instance: make_executable
	pypy3 ./provision_scripts/delete_instance.py "$(INSTANCE_NAME)" "$(STRATEGY_NAME)"

clean_dlls:
	mkdir -p /home/vagrant/ss/bt/strategies_dlls
	rm -rf /home/vagrant/ss/bt/strategies_dlls/*

move_strategy_dll: delete_instance clean_dlls all
	cp $(LIBRARY) /home/vagrant/ss/bt/strategies_dlls/.

build_strategy: move_strategy_dll
	./provision_scripts/build_strategy.sh "$(INSTANCE_NAME)" "$(STRATEGY_NAME)" "$(SYMBOLS)"

run_backtest: build_strategy
	chmod +x ./provision_scripts/run_backtest.sh
	./provision_scripts/run_backtest.sh "$(INSTANCE_NAME)" "$(START_DATE)" "$(END_DATE)" "$(REPO_DIR)" "$(STRATEGY_DIR)"

