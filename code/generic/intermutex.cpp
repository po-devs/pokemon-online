#include "intermutex.hpp"

namespace interface
{

Mutex::Mutex()
{
    handle = SDL_CreateMutex();
}

Mutex::~Mutex()
{
    SDL_DestroyMutex(handle);
}

void Mutex::Lock()
{
    SDL_mutexP(handle);
}

void Mutex::Unlock()
{
    SDL_mutexV(handle);
}

} // namespace interface
