#pragma once

#ifdef _WIN32
    // Windows
    #ifdef BUILDING_LIBRARY
        #define API_EXPORT __declspec(dllexport)
    #else
        #define API_EXPORT __declspec(dllimport)
    #endif
#else
    // Linux/Unix
    #define API_EXPORT __attribute__((visibility("default")))
#endif

#include <string>
#include <vector>
#include <set>
#include <unordered_set>

// Class IBingoPlugin serves as the plugin interface to the underlying Bingo Backend game.
// Call createPlugin() inside your application to create and get a pointer to the plugin object.
// Call destroyPlugin() inside your application to delete the plugin object.

// Class IBingoPlugin is a container of Players.
// On init() a session id is created, settings are loaded and an optional log location can be set.
// Logs will register actions made during the Bingo game iterations.
// The settings loaded will define the game rules (eg. max number of players, card nnumbers etc...).
// A normal usage would be to start by adding players with their credits and extra balls.
// To start a game round use startRound(), it will clear all the players cards and make players pay the round entry fee. 
// If players don't have credits to pay the entry fee, they'll remain idle during plays, can hold no cards and cannot win any prizes.
// When players enter the game they enter with the maximum ammout of cards.
// To advance on the current round call drawNewBall() consecutively, until max draws is reached. 
// drawNewBall() will evaluate player states and check for prizes.
// When the max draws is reached, drawNewBall() will return false, and become a noop.
// At this state the game round has ended, to convert player prizes to credits call convertPrizesToCredits(), this closes a game loop.
// This is the stage where players can be added/removed and a new round can start by calling startRound().

class IBingoPlugin 
{
public:

    // # Challenge 2 Protocol
    typedef int PlayerID;
    typedef int CardID;
    typedef std::vector<std::vector<int>> CardContent;

    virtual ~IBingoPlugin() {} // Virtual destructor is mandatory
    virtual bool init(const std::string_view& settingsPath, const std::string_view& logFileDir, std::string& o_error) = 0;
    virtual void enableLogging(bool enable) = 0;
    virtual bool addPlayer(PlayerID& o_id) = 0;
    virtual bool removePlayer(PlayerID id) = 0;
    virtual bool getPlayerCard(const PlayerID& playerId, const CardID& cardId, CardContent& o_content) const = 0;
    virtual bool getPlayerCardIDs(const PlayerID& playerId, std::set<CardID>& o_cardIds) const = 0;
    virtual bool calcPlayerCardPrizes(const PlayerID& playerId, const CardID& cardId, unsigned& o_prizes) const = 0;
    virtual bool calcPlayerPrizes(const PlayerID& playerId, unsigned& o_totalPrizes) const = 0;
    virtual bool addPlayerCredits(const PlayerID& playerId, const unsigned credits) = 0;
    virtual bool getPlayerCredits(const PlayerID& playerId, unsigned& o_credits) const = 0;
    virtual bool addPlayerExtraBall(const PlayerID& playerId, int extraBall) = 0;
    virtual bool getPlayerExtraBalls(const PlayerID& playerId, std::set<int>& o_extraBalls) const = 0;
    virtual void getDraws(std::unordered_set<int>& o_draws) const = 0;
    virtual bool roundActive() const = 0;
    virtual void startRound() = 0;
    virtual bool drawNewBall() = 0;
    virtual void convertPrizesToCredits() = 0;
};

extern "C" 
{
    // Factory function to create the instance
    API_EXPORT IBingoPlugin* createPlugin();

    // Destructor function to clean up
    API_EXPORT void destroyPlugin(IBingoPlugin* instance);
}