#!/usr/bin/env python2.6
# This script fixes the linking of Client Plugins
# It uses the shell command install_name_tool to properly change 
# dependency paths for libraries 

import shutil
import os.path
import commands

def find_dylibs():
    dylibs = commands.getoutput(r"find . -iname \*.dylib").split()
    return [dylib for dylib in dylibs if not os.path.islink(dylib)]

def get_shared_libs(dylib):
    libs = commands.getoutput(r"otool -L {dylib}".format(dylib=dylib)).splitlines()[1:]
    return [line.split("(")[0].strip() for line in libs]

def change_linking(dylib, old_path, dyn_path):
    command = r"install_name_tool -change {old_path} {dyn_path} {dylib}" .format(old_path=old_path, dyn_path=dyn_path, dylib=dylib)
    return commands.getstatusoutput(command)[0]

def install_dependency(from_path, to_path, install_name): 
    shutil.copy(from_path, to_path) 
    perm_fix_command = "chmod -R u+w {path}".format(path=to_path)
    if commands.getstatusoutput(perm_fix_command)[0] > 0:
       print("Warning: chmod failed for {path}".format(path=to_path))
    command = r"install_name_tool -id {install_name} {dylib}".format(install_name=install_name, dylib=to_path)
    return commands.getstatusoutput(command)[0]

def fix_linking_for_dylib(dylib):
    LINK = "@executable_path/../Frameworks/{path}"
    FRAMEWORKS = "../Pokemon-Online.app/Contents/Frameworks/{path}"
    ALWAYS_FIX_LIBS=["libpokemonlib.1.dylib", "libbattlelib.1.dylib", "libutilities.1.dylib"]
    shared_libs = get_shared_libs(dylib)
    for shared_lib in shared_libs:
        # Assume all shared libs in /usr/local to be bundable
        if shared_lib in ALWAYS_FIX_LIBS: 
            library=shared_lib
        elif not shared_lib.startswith("/usr/local"):
            continue
        elif ".framework" in shared_lib:
            # Need to bundle the whole framework
            index = shared_lib.rindex("/", 0, shared_lib.index(".framework"))
            library = shared_lib[index+1:] 
        else:
            # Bundle only last part
            index = shared_lib.rindex("/")
            library = shared_lib[index+1:] 
        install_name = LINK.format(path=library)
        if not os.path.exists(FRAMEWORKS.format(path = library.split("/")[0])):
            new_path = FRAMEWORKS.format(path=library.split("/")[0])
            install_dependency(shared_lib, FRAMEWORKS.format(path=library.split("/")[0]), install_name)
            fix_linking_for_dylib(new_path)
        change_linking(dylib, shared_lib, install_name)
 
def fix_linking():
    dylibs =  find_dylibs()
    for dylib in dylibs:
        fix_linking_for_dylib(dylib)

if __name__ == "__main__":
    fix_linking()
