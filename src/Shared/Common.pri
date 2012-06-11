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
