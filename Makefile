BT=src/Teambuilder/
Pokemon=src/PokemonInfo/
all:	
	@echo "Compilation de la lib"
	cd $(Pokemon) && qmake && make
	cd ../..
	@echo "Compilation du programme"
	cd $(BT) && qmake && make
	cd ../..
	@echo "How-to : # make install initialisera tout ce qu'il faut. Il suffira de démarer l'executable"

install:
	mv bin/lib*.so* /usr/lib/