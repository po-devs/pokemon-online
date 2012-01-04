# Common shadow build directory for all builds
CONFIG(shadow) {
   CONFIG(debug, debug|release) {
      OBJECTS_DIR=$${_PRO_FILE_PWD_}_build_debug
   }
   CONFIG(release, debug|release) {
      OBJECTS_DIR=$${_PRO_FILE_PWD_}_build_release
   }
   message("Shadow build enabled. Obj dir" $$OBJECTS_DIR)
} else {
   message("No shadow build")
}

macx {
   # Mac OSX does not have recent enough GCC for compiling PO
   # Hence a custom installation must be provided.

   INCLUDEPATH += /usr/local/gcc-4.6.2/include
   LIBS += -L/usr/local/gcc-4.6.2/lib

   # Same applies for libzip installation.

   INCLUDEPATH += /usr/local/include
   LIBS += -L/usr/local/lib
}
