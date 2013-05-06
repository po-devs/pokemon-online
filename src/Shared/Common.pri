# Common shadow build directory for all builds
CONFIG(shadow)|!equals($${_PRO_FILE_PWD_}, $${OUT_PWD}) {
   CONFIG(debug, debug|release) {
      OBJECTS_DIR=$${_PRO_FILE_PWD_}/../../build/debug/$$basename(_PRO_FILE_PWD_)
   }
   CONFIG(release, debug|release) {
      OBJECTS_DIR=$${_PRO_FILE_PWD_}/../../build/release/$$basename(_PRO_FILE_PWD_)
   }
   message("Shadow build enabled. Obj dir" $$OBJECTS_DIR)
} else {
   message("No shadow build")
}

bin = $$PWD/../../bin
#adds debug suffix to libraries when compiled
CONFIG(debug, debug|release) {
    mac {
        TARGET = $$join(TARGET,,,_debug)
        utilities = -L$$bin -lpo-utilities_debug
        pokemoninfo = $$utilities -lpo-pokemoninfo_debug
        battlemanager = $$pokemoninfo -lpo-battlemanager_debug
        websocket = -lqtwebsocket_debug
        json = -lqjson_debug
    }
    win32 {
        TARGET = $$join(TARGET,,,d)
        utilities = -L$$bin -lpo-utilitiesd
        pokemoninfo = $$utilities -lpo-pokemoninfod
        battlemanager = $$pokemoninfo -lpo-battlemanagerd
        websocket = -lqtwebsocketd
        json = -lqjsond
    }
    !mac:!win32 {
        utilities = -L$$bin -lpo-utilities
        pokemoninfo = $$utilities -lpo-pokemoninfo
        battlemanager = $$pokemoninfo -lpo-battlemanager
        websocket = -lqtwebsocket
        json = -lqjson
    }
} else {
    utilities = -L$$bin -lpo-utilities
    pokemoninfo = $$utilities -lpo-pokemoninfo
    battlemanager = $$pokemoninfo -lpo-battlemanager
    websocket = -lqtwebsocket
    json = -lqjson
}
