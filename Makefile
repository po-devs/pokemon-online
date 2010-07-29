BT=src/Teambuilder/
BS=src/Server/
BU=src/Utilities/
BP=src/PokemonInfo/


all:	
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the utilities library"
	cd $(BU) && qmake && make
	cd ../..
	@echo "Compiling the pokemon library"
	cd $(BP) && qmake && make
	cd ../..
	@echo "Compiling the client"
	cd $(BT) && qmake && make
	cd ../..
	@echo "Compiling the server"
	cd $(BS) && qmake && make
	cd ../..
	@echo "Make sure to add the bin directory to your library path, in order to run the program."
	@echo "For that look up on how to use LD_LIBRARY_PATH (like adding the appropriate export to the end of your ~/.bashrc)"
	

client:
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the utilities library"
	cd $(BU) && qmake && make
	cd ../..
	@echo "Compiling the pokemon library"
	cd $(BP) && qmake && make
	cd ../..
	@echo "Compiling the client"
	cd $(BT) && qmake && make
	cd ../..
	@echo "Make sure to add the bin directory to your library path, in order to run the program."
	@echo "For that look up on how to use LD_LIBRARY_PATH (like adding the appropriate export to the end of your ~/.bashrc)"

server:	
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the server utilities"
	cd $(BU) && qmake && make
	cd ../..
	@echo "Compiling the server pokemon library"
	cd $(BP) && qmake && make
	cd ../..
	@echo "Compiling the server"
	cd $(BS) && qmake && make
	cd ../..
	@echo "Make sure to add the bin directory to your library path, in order to run the program."
	@echo "For that look up on how to use LD_LIBRARY_PATH (like adding the appropriate export to the end of your ~/.bashrc)"

install:
	@echo "Nothing to do, run the executable in the bin folder and make sure to edit your ~/.bashrc to add the path to the bin directory to LD_LIBRARY_PATH"
