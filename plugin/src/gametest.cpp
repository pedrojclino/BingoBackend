#include "gametest.h"
#include "game.h"
#include "player.h"
#include "json.hpp"

#include <iostream>
#include <fstream>

const std::string GameTest::m_drawsKey = "Draws";
const std::string GameTest::m_playersKey = "Players";
const std::string GameTest::m_startingCreditsKey = "StartingCredits";
const std::string GameTest::m_finalCreditsKey = "FinalCredits";
const std::string GameTest::m_extraBallsKey = "ExtraBalls";
const std::string GameTest::m_cardsKey = "Cards";

namespace
{
    void printErrorMessage(const std::string_view& ketField)
    {
        std::cout << "Could not find '" << ketField << "' field in testing file!\n";
    }

    bool loadCardData(const nlohmann::json& gamePlayerCardRows, GameTest::CardRows& o_cardRows)
    {
        for (int i = 0, j=0; i < gamePlayerCardRows.size(); i++)
        {
            GameTest::CardRow cardRow;
            auto& gamePlayerCardCols = gamePlayerCardRows.at(i);
            for (j = 0; j < gamePlayerCardCols.size(); j++)
            {
                cardRow.push_back(gamePlayerCardCols.at(j));
            }
            o_cardRows.push_back(cardRow);
        }

        return true;
    }

    bool loadPlayerData(const nlohmann::json& gamePlayerData, GameTest::PlayerData& o_playerData)
    {
        if (!gamePlayerData.contains(GameTest::m_startingCreditsKey))
        {
            printErrorMessage(GameTest::m_startingCreditsKey);
            return false;
        }
        o_playerData.startingCredits = gamePlayerData.at(GameTest::m_startingCreditsKey);

        if (!gamePlayerData.contains(GameTest::m_finalCreditsKey))
        {
            printErrorMessage(GameTest::m_finalCreditsKey);
            return false;
        }
        o_playerData.finalCredits = gamePlayerData.at(GameTest::m_finalCreditsKey);

        if (!gamePlayerData.contains(GameTest::m_extraBallsKey))
        {
            printErrorMessage(GameTest::m_extraBallsKey);
            return false;
        }
        auto& extraBalls = gamePlayerData.at(GameTest::m_extraBallsKey);
        for (int i = 0; i < extraBalls.size(); i++)
            o_playerData.extraBalls.insert(static_cast<int>(extraBalls.at(i)));

        if (!gamePlayerData.contains(GameTest::m_cardsKey))
        {
            printErrorMessage(GameTest::m_cardsKey);
            return false;
        }

        // Import Cards
        GameTest::Cards cards;
        auto gamePlayerCards = gamePlayerData.at(GameTest::m_cardsKey);
        for (int i = 0; i < gamePlayerCards.size(); i++)
        {
            GameTest::CardRows cardRowsData;
            auto& gamePlayerCardRows = gamePlayerCards.at(i);
            if (!loadCardData(gamePlayerCardRows, cardRowsData))
                return false;
            cards.push_back(cardRowsData);
        }
        o_playerData.cardsData = cards;
        return true;
    }

    bool loadGameData(const nlohmann::json& fullGamedata, GameTest::GameData& o_gameData)
    {
        if (!fullGamedata.contains(GameTest::m_drawsKey))
        {
            printErrorMessage(GameTest::m_drawsKey);
            return false;
        }
        auto drawsData = fullGamedata.at(GameTest::m_drawsKey);
        for (int i = 0; i < drawsData.size(); i++)
        {
            auto newElem = drawsData.at(i);
            if (o_gameData.draws.find(newElem) != o_gameData.draws.end())
                return false;
            o_gameData.draws.insert((int)drawsData.at(i));
        }

        if (!fullGamedata.contains(GameTest::m_playersKey))
        {
            printErrorMessage(GameTest::m_playersKey);
            return false;
        }
        std::vector<GameTest::PlayerData> playersData;
        auto& gamePlayersData = fullGamedata.at(GameTest::m_playersKey);
        for (int i = 0; i < gamePlayersData.size(); i++)
        {
            // Import PlayerData
            GameTest::PlayerData playerData;
            auto& gamePlayerData = gamePlayersData.at(i);
            if (!loadPlayerData(gamePlayerData, playerData))
                return false;
            playersData.push_back(playerData);
        }
        o_gameData.playersData = playersData;
        return true;
    }
}

void GameTest::init(const std::string_view& settingsPath, const std::string_view& logFileDir)
{
    m_settingsPath = settingsPath;
    m_logFileDir = logFileDir;
}

bool GameTest::loadTestSequence(const std::string_view& testSettingsPath)
{
    m_gameDatas.clear();

    std::ifstream file(testSettingsPath.data());

    // Check if file opened successfully
    if (!file.is_open())
    {
        std::cout << "Could not open settings file!";
        return false;
    }

    // Parse the JSON content
    nlohmann::json data;
    file >> data;

    if (!data.is_array())
        return false;

    // Load games
    for (int i = 0; i < data.size(); i++)
    {
        // Import GameData
        GameData gameData;
        if (!loadGameData(data.at(i), gameData))
            return false;
        m_gameDatas.push_back(gameData);
    }

    return true;
};

void GameTest::drawBalls(Game& o_game, const std::unordered_set<int>& draws)
{
    o_game.m_settings.m_maxDraws = static_cast<unsigned>(draws.size()); // Force this
    for (auto newDraw : draws)
    {
        if (!o_game.m_roundActive)
            return; // Game completed

        o_game.m_draws.insert(newDraw);

        // Block new drawNewBall() calls, until startRound()
        o_game.m_roundActive = o_game.m_draws.size() != o_game.m_settings.maxDraws();

        // Check player cards for prizes with the new draws. 
        // Note: If player didn't pay to play, play() is a noop.
        for (auto& player : o_game.m_players)
            player.second.play(o_game.m_settings.cardProps(), o_game.m_draws);

        // Write play event to log
        o_game.writeLogEntry();
    }
}

bool GameTest::validateFullGame(bool writeLogs)
{
    Game game;
    for (auto& gameData : m_gameDatas)
    {
        std::string err;
        if (!game.init(m_settingsPath, m_logFileDir, err))
            return false;

        game.enableLogging(writeLogs);


        // Play a Game
        if (game.roundActive())
            return false;

        // Load Players and their extra balls (stable throughout rounds)
        std::map<Game::PlayerID, int> playersFinalCredits;
        std::map<Game::PlayerID, Cards> playerCardContents;
        for (auto& playerData : gameData.playersData)
        {
            Game::PlayerID playerId{ -1 };
            if (!game.addPlayer(playerId))
                return false;

            game.addPlayerCredits(playerId, playerData.startingCredits);
            for (auto& extraBall : playerData.extraBalls)
                game.addPlayerExtraBall(playerId, extraBall);
            playersFinalCredits[playerId] = playerData.finalCredits;

            playerCardContents[playerId] = playerData.cardsData;
        }

        // Now players can pay to play (this clears any cards to start a new round)
        game.startRound();

        // Re-create Cards
        for (const auto& [playerId, cardsData] : playerCardContents)
        {
            std::set<Card::CardID> cardIDs;
            if (!game.getPlayerCardIDs(playerId, cardIDs))
                return false;

            size_t idx{0};
            for (auto cardID : cardIDs)
            {
                if (cardsData.size() <= idx)
                    game.m_players[playerId].m_cards[cardID] = Card(); // Clear
                else
                    game.m_players[playerId].m_cards[cardID].m_content = cardsData[idx]; // Set test content
                idx++;
            }
        }

        if (!game.roundActive())
            return false;

        // Loop until the game has finished
        drawBalls(game, gameData.draws);

        game.convertPrizesToCredits();

        // Match expected prizes
        for (auto& player : game.m_players)
        {
            if (playersFinalCredits[player.first] != (int)player.second.credits())
                return false; // This is the final logic checker
        }
    }

    return true;
}