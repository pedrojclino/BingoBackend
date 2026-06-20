#pragma once

#include <vector>
#include <set>
#include <unordered_set>
#include <string>

class Game;

class GameTest
{
public:

    typedef std::vector<int> CardRow;
    typedef std::vector<CardRow> CardRows;
    typedef std::vector<CardRows> Cards;

    struct PlayerData
    {
        int startingCredits;
        int finalCredits;
        std::set<int> extraBalls;
        Cards cardsData;
    };

    struct GameData
    {
        std::unordered_set<int> draws;
        std::vector<PlayerData> playersData;
    };

    static const std::string m_drawsKey;
    static const std::string m_playersKey;
    static const std::string m_startingCreditsKey;
    static const std::string m_finalCreditsKey;
    static const std::string m_extraBallsKey;
    static const std::string m_cardsKey;

    void init(const std::string_view& settingsPath, const std::string_view& logFileDir);
    bool loadTestSequence(const std::string_view& testSettingsPath);
    bool validateFullGame(bool writeLogs = false);

private:
    void drawBalls(Game& o_game, const std::unordered_set<int>& draws);

    std::vector<GameData> m_gameDatas;

    std::string_view m_settingsPath;
    std::string_view m_logFileDir;
};