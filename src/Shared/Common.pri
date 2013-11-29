contains(EXTRAS, test) {
   dirprefix = tests/
} else {
   dirprefix =
}

# Common shadow build directory for all builds
CONFIG(shadow)|!equals($${_PRO_FILE_PWD_}, $${OUT_PWD}) {
   CONFIG(debug, debug|release) {
      OBJECTS_DIR=$${_PRO_FILE_PWD_}/../../build/debug/$$dirprefix$$basename(_PRO_FILE_PWD_)
   }
   CONFIG(release, debug|release) {
      OBJECTS_DIR=$${_PRO_FILE_PWD_}/../../build/release/$$dirprefix$$basename(_PRO_FILE_PWD_)
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
        DEFINES += EXE_SUFFIX="_debug"
    }
    win32 {
        TARGET = $$join(TARGET,,,d)
        utilities = -L$$bin -lpo-utilitiesd
        pokemoninfo = $$utilities -lpo-pokemoninfod
        battlemanager = $$pokemoninfo -lpo-battlemanagerd
        websocket = -lqtwebsocketd
        json = -lqjsond
        DEFINES += EXE_SUFFIX="d"
    }
    !mac:!win32 {
        TARGET = $$join(TARGET,,,_debug)
        utilities = -L$$bin -lpo-utilities_debug
        pokemoninfo = $$utilities -lpo-pokemoninfo_debug
        battlemanager = $$pokemoninfo -lpo-battlemanager_debug
        websocket = -lqtwebsocket_debug
        json = -lqjson_debug
        DEFINES += EXE_SUFFIX="_debug"
    }
} else {
    utilities = -L$$bin -lpo-utilities
    pokemoninfo = $$utilities -lpo-pokemoninfo
    battlemanager = $$pokemoninfo -lpo-battlemanager
    websocket = -lqtwebsocket
    json = -lqjson
    DEFINES += EXE_SUFFIX=""
}


contains(QT_VERSION, ^5\\.[0-9]\\..*) {
  DEFINES += QT5
  QT += widgets
  CONFIG += c++11
} else {
  QMAKE_CXXFLAGS += "-std=c++0x -U__STRICT_ANSI__"
}

unix:!mac {
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN'"
}
