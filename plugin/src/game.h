#pragma once

#include "bingoPlugin.h"
#include "settings.h"
#include "player.h"

#include <map>

class GameTest;

// Class Game implements the API inherited by the plugin base class - more details there.
class Game : public IBingoPlugin
{
public:
    virtual ~Game() {};

    // # Challenge 2 Protocol
    virtual bool init(const std::string_view& settingsPath, const std::string_view& logFileDir, std::string& o_error) override;
    virtual void enableLogging(bool enable) override;
    virtual bool addPlayer(PlayerID& o_id) override;
    virtual bool removePlayer(PlayerID id) override;
    virtual bool getPlayerCard(const PlayerID& playerId, const Card::CardID& cardId, CardContent& o_content) const override;
    virtual bool getPlayerCardIDs(const PlayerID& playerId, std::set<Card::CardID>& o_cardIds) const override;
    virtual bool calcPlayerCardPrizes(const PlayerID& playerId, const Card::CardID& cardId, unsigned& o_prizes) const override;
    virtual bool calcPlayerPrizes(const PlayerID& playerId, unsigned& o_totalPrizes) const override;
    virtual bool addPlayerCredits(const PlayerID& playerId, const unsigned credits) override;
    virtual bool getPlayerCredits(const PlayerID& playerId, unsigned& o_credits) const override;
    virtual bool addPlayerExtraBall(const PlayerID& playerId, int extraBall) override;
    virtual bool getPlayerExtraBalls(const PlayerID& playerId, std::set<int>& o_extraBalls) const override;
    virtual void getDraws(std::unordered_set<int>& o_draws) const override;
    virtual bool roundActive() const override;
    virtual void startRound() override;
    virtual bool drawNewBall() override;
    virtual void convertPrizesToCredits() override;
    
private:
    bool writeLogEntry() const;
    bool setupLogDir(const std::string_view& logFileDir, std::string& o_error);

    Settings m_settings;

    unsigned m_sessionId{ 0 }; // 1 session per game 
    unsigned m_roundCount{ 0 }; // 1 round = cycle from restart() -> finished()
    bool m_roundActive{ false }; // True after running restart(), False of finish()

    bool m_logToFile{ false };
    std::string m_logFileDir{""};

    std::unordered_set<int> m_draws;
    std::map<PlayerID, Player> m_players;

friend GameTest;
};