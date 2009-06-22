#ifndef IMAGE_MANAGER_H_INCLUDED
#define IMAGE_MANAGER_H_INCLUDED

#include <string>
#include <map>
#include "intervideo.hpp"

//ImageManager. Note that the fact that Surface are already shared with a refcount when copying
//makes it really easy to make

class ImageManager
{
public:
    //All images will be loaded relatively to this directory,
    std::string directory;
    //I believe it's explicit enough, that and that it should be private as well
    std::map <std::string, interface::Surface> ressources;

    //rep = that directory
    ImageManager(const char *rep = "");

    //pass "." as an argument for getting the current dir, otherwise changes the dir
    //But it's dangerous to do runtime directories changes
    const std::string &SetDirectory(const char *rep = ".");

    //load the wanted ressource with those settings
    //note that the settings only work if the image is not already loaded
    interface::Surface LoadRessource (const char *filename, bool raw = false);
    interface::Surface LoadRessource (const char *filename, const interface::Color &colorkey,
        bool raw = false);

    //Destroy the non-used images (garbage collector lol!)
    void clean_up();

    //Destroy everything
    void clear();
};


#endif // IMAGE_MANAGER_H_INCLUDED
