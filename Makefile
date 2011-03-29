# Default qmake binary. Some systems have qmake-qt4 and qmake-qt3 instead.
QMAKE=qmake
# Default make binary. Some systems have nmake or gmake (some bsds)
MAKE=make

install-message="Nothing to do, run the executable in the bin folder and make sure to edit your ~/.bashrc to add the path to the bin directory to LD_LIBRARY_PATH"

SUBDIRS = Utilities PokemonInfo Client Server

all: client server
	@echo "Read instructions in HowToBuild.txt"
	@echo ${install-message}

.SECONDARY: Makefile
%.cpp: ;
%.o: ;
%.h: ;
DIRE = src/Utilities src/Teambuilder src/Server src/PokemonInfo			\
src/BattleLogs src/veekun_data_extracter src/ChainBreeding src/MoveMachine	\
src/EventCombinations src/level_balance src/UsageStatistics			\
src/StatsExtracter src/Registry src/DOSTest src/PokesIndexConverter

# Instruct make on how to convert any given .pro file to a Makefile
# and then compile that Makefile. This expands to the correct rule for
# whichever directory needs making.
define QMAKE_template
 $(1)/%.pro: $$(wildcard $(1)/*.cpp) $$(wildcard $(1)/*.h)
	$$(QMAKE) -makefile -o ${1}/Makefile $$@
	$${MAKE} -C $${@D}
endef

$(foreach d, ${DIRE}, $(eval $(call QMAKE_template,$(d))))

utilities: src/Utilities/Utilities.pro
	@echo "Compiling the utilities library"

pokemon-info: utilities src/PokemonInfo/PokemonInfo.pro
	@echo "Compiling the pokemon library"

battlelogs: pokemon-info src/BattleLogs/BattleLogs.pro 
	@echo "Compiling the battlelogs plugin"
;
client: pokemon-info src/Teambuilder/Teambuilder.pro
	@echo "Compiling the client"
	@echo ${install-message}

server:	pokemon-info src/Server/Server.pro
	@echo "Compiling the server"
	@echo ${install-message}

install:
	@echo ${install-message}

# This should also clean up any binaries generated, but
# we can mess with these later.
clean:
	rm -rf *.o 		# Remove all object files
	rm -f src/Teambuilder/Makefile	# Remove generated makefiles
	rm -f src/Server/Makefile
	rm -f src/Utilities/Makefile
	rm -f src/PokemonInfo/Makefile
