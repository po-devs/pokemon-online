BT=src/Teambuilder/
BS=src/Server/
BSL=src/MainDll

all:	
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the client"
	cd $(BT) && qmake && make
	cd ../..
	@echo "Compiling the main server DLL"
	cd $(BSL) && qmake && make
	cd ../..
	@echo "Compiling the server"
	cd $(BS) && qmake && make
	cd ../..

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

install:
	@echo "Nothing to do, run the executable in the bin folder"
