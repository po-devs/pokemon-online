BT=src/Teambuilder/

all:	
	@echo "Compilation du programme"
	cd $(BT) && qmake && make
	cd ../..
	@echo "How-to : # Démarrez l'exécutable dans le dossier bin"

install:
	@echo "Rien à faire"
