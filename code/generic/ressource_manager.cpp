#include "ressource_manager.hpp"
#include <iostream>
#include <sstream>

using namespace std;
using interface::Surface;
using interface::Color;
using interface::Font;

//rep = that directory
ImageManager::ImageManager(const char *rep)
    :directory(rep)
{

}


//pass "." as an argument for getting the current dir, otherwise changes the dir
//But it's dangerous to do runtime directories changes
const std::string &ImageManager::SetDirectory(const char *rep)
{
    if (rep == ".")
        return directory;
    else
        return (directory = rep);
}

//load the wanted ressource with those settings
//note that the settings only work if the image is not already loaded
Surface ImageManager::LoadRessource (const char *filename, bool raw)
{
    string path = directory + filename;

    map <string, Surface>::iterator it;
    if ( (it=ressources.find(path)) == ressources.end() )
    {
        Surface s;
        raw?s.raw_load(path.c_str()):s.load(path.c_str());
        ressources[path] = s;
        return s;
    } else
    {
        return it->second;
    }
}

Surface ImageManager::LoadRessource (const char *filename, const Color &colorkey, bool raw)
{
    string path = directory + filename;

    map <string, Surface>::iterator it;
    if ( (it=ressources.find(path)) == ressources.end() )
    {
        Surface s;
        raw?s.raw_load(path.c_str(),colorkey):s.load(path.c_str(),colorkey);
        ressources[path] = s;
        return s;
    } else
    {
        return it->second;
    }
}

//Destroy the non-used images (garbage collector lol!)
void ImageManager::clean_up()
{
    map <string, Surface>::iterator it;

    it = ressources.begin();

    while( it != ressources.end() )
    {
        if (it->second.refcount() <= 1) //if the Surface is invalid or only stored in our ressources
            ressources.erase(it++);//erases the current it while giving the new one
        else
            ++it; //simply increasing
    }
}

//Destroy everything
void ImageManager::clear()
{
    ressources.clear();
}


/***

    FONT MANAGER!

***/



//rep = that directory
FontManager::FontManager(const char *rep)
    :directory(rep)
{

}


//pass "." as an argument for getting the current dir, otherwise changes the dir
//But it's dangerous to do runtime directories changes
const std::string &FontManager::SetDirectory(const char *rep)
{
    if (rep == ".")
        return directory;
    else
        return (directory = rep);
}

//load the wanted ressource with those settings
//note that the settings only work if the image is not already loaded
Font FontManager::LoadRessource (const char *filename, Uint16 ptsize)
{
    string path = directory + filename;

    ostringstream in;
    in << path << "_" << ptsize;

    /* Actually this is that string that will be stored, it contains
       the path and the ptsize...*/
    const string res = in.str();

    map <string, Font>::iterator it;
    if ( (it=ressources.find(res)) == ressources.end() )
    {
        Font f(path.c_str(), ptsize);
        ressources[res] = f;
        return f;
    } else
    {
        return it->second;
    }
}

//Destroy the non-used fonts (garbage collector lol!)
void FontManager::clean_up()
{
    map <string, Font>::iterator it;

    it = ressources.begin();

    while( it != ressources.end() )
    {
        if (it->second.refcount() <= 1) //if the Surface is invalid or only stored in our ressources
            ressources.erase(it++);//erases the current it while giving the new one
        else
            ++it; //simply increasing
    }
}

//Destroy everything
void FontManager::clear()
{
    ressources.clear();
}
