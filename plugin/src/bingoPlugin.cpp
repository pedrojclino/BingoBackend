#define BUILDING_LIBRARY
#include "bingoPlugin.h"
#include "game.h"

// Cast the opaque pointer back to the real class internally
// # Challenge 3 API
extern "C" 
{
    API_EXPORT IBingoPlugin* createPlugin() 
    {
        return reinterpret_cast<IBingoPlugin*>(new Game());
    }

    API_EXPORT void destroyPlugin(IBingoPlugin* instance) 
    {
        delete reinterpret_cast<Game*>(instance);
    }
}