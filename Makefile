BT=src/Teambuilder/
BS=src/Server/

all:	
	@echo "Read instructions in HowToBuild.txt"
	@echo "Compiling the client"
	cd $(BT) && qmake && make
	cd ../..
	@echo "Compiling the server"
	cd $(BS) && qmake && make
	cd ../..

install:
	@echo "Nothing to do, run the executable in the bin folder"
