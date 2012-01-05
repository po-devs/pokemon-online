# attempt autodetection of which qmake to use.
ifeq ($(shell uname),darwin)	# For mac OSX
QMAKE=qmake -spec macx-g++ CONFIG+="${CONFIG}"
else				# For everyone else.
QMAKE=qmake CONFIG+="${CONFIG}"
endif

# Default make binary. Some systems have nmake or gmake (some bsds)
MAKE=make

install-message="Nothing to do, run the executable in the bin folder and make sure to edit your ~/.bashrc to add the path to the bin directory to LD_LIBRARY_PATH"

all: client server plugins
	@echo "Read instructions in HowToBuild.txt"
	@echo ${install-message}

bin/Server: pokemon-info src/Server/Server.pro ;

%.cpp: ;
%.o: ;
%.h: ;
DIRE = src/Utilities src/Teambuilder src/Server src/PokemonInfo			\
src/BattleLogs src/veekun_data_extracter src/ChainBreeding src/MoveMachine	\
src/EventCombinations src/level_balance src/UsageStatistics			\
src/StatsExtracter src/Registry src/DOSTest src/PokesIndexConverter src/BattleManager \

# Instruct make on how to convert any given .pro file to a Makefile
# and then compile that Makefile. This expands to the correct rule for
# whichever directory needs making.
define QMAKE_template
$(1)/%.pro: Makefile $$(wildcard $(1)/*.cpp) $$(wildcard $(1)/*.h) $$(wildcard $(1)/*.o)
	$$(QMAKE) -o ${1}/Makefile $$@
	$${MAKE} -C $${@D}
endef

$(foreach d, ${DIRE}, $(eval $(call QMAKE_template,$(d))))

utilities: src/Utilities/Utilities.pro 
	@echo "compiling utilities."

pokemon-info: utilities src/PokemonInfo/PokemonInfo.pro ;

battlelib: pokemon-info src/BattleManager/BattleManager.pro ;

battlelogs: battlelib src/BattleLogs/BattleLogs.pro ;
usagestats: pokemon-info src/UsageStatistics/UsageStatistics.pro ;
plugins: battlelogs usagestats ;

client: battlelib src/Teambuilder/Teambuilder.pro ;

server:	bin/Server ;

install:
	@echo ${install-message}

# This should also clean up any binaries generated, but
# we can mess with these later.
clean:
	${RM} src/*/*.o 		# Remove all object files
	${RM} src/Teambuilder/Makefile	# Remove generated makefiles
	${RM} src/Server/Makefile
	${RM} src/Utilities/Makefile
	${RM} src/PokemonInfo/Makefile
	${RM} src/BattleManager/Makefile

strip:
	${STRIP} bin/*.so
	${STRIP} Pokemon-Online
	${STRIP} Server

clean-scripts:
	${RM} bin/scripts.js

clean-dlls:
	${RM} bin/zip.dll
	${RM} bin/zlib1.dll

clean-mac-app:
	${RM} bin/bundle_mac_app.sh
