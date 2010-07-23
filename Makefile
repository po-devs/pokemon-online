BT=src/Teambuilder/
BS=src/Server/
BU=src/Utilities
BP=src/PokemonInfo


all:	
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the client"
	cd $(BT) && qmake && make
	cd ../..
	@echo "Compiling the server utilities"
	cd $(BU) && qmake && make
	cd ../..
	@echo "Compiling the server pokemon library"
	cd $(BP) && qmake && make
	cd ../..
	@echo "Compiling the server"
	cd $(BS) && qmake && make
	cd ../..
	@echo "Make sure to add the bin directory to your library path if you want to use the server."
	@echo "For that look up on how to use LD_LIBRARY_PATH"
	

client:
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the client"
	cd $(BT) && qmake && make
	cd ../..

server:	
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the main server DLL"
	cd $(BSL) && qmake && make
	cd ../..
	@echo "Compiling the server"
	cd $(BS) && qmake && make
	cd ../..
	@echo "Make sure to add the pogeymon-online/bin directory to your library path."
	@echo "For that look up on how to use LD_LIBRARY_PATH"

install:
	@echo "Nothing to do, run the executable in the bin folder"
