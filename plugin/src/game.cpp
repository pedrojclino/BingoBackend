#include "game.h"
#include "utils.h"
#include "json.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>

bool Game::init(const std::string_view& settingsPath, const std::string_view& logFileDir, std::string& o_error)
{
    m_sessionId = Utils::epochTime();
    m_roundCount = 0;
    m_players.clear();

    return m_settings.loadSettings(settingsPath, o_error) && 
           setupLogDir(logFileDir, o_error);
}

void  Game::enableLogging(bool enable)
{
    m_logToFile = enable;
}

bool Game::addPlayer(PlayerID& o_id)
{
    if (m_players.size() == m_settings.maxPlayers())
        return false;

    o_id = Utils::random(1, m_settings.maxPlayers());
    while (m_players.find(o_id) != m_players.end())
        o_id = Utils::random(1, m_settings.maxPlayers());
    m_players.insert({ o_id, Player()});

    return true;
}

bool Game::removePlayer(PlayerID id)
{
    if (m_players.find(id) == m_players.end())
        return false;

    m_players.erase(id);
    return true;
}

bool Game::getPlayerCard(const PlayerID& playerId, const Card::CardID& cardId, CardContent& o_content) const
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    if(!m_players.at(playerId).cardContent(cardId, o_content))
        return false;

    return true;
}

bool Game::getPlayerCardIDs(const PlayerID& playerId, std::set<Card::CardID>& o_cardIDs) const
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    o_cardIDs = m_players.at(playerId).cardIDs();
    return true;
}

bool Game::calcPlayerCardPrizes(const PlayerID& playerId, const Card::CardID& cardId, unsigned& o_prizes) const
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    if (!m_players.at(playerId).calcCardPrizes(m_settings.cardProps(), cardId, m_draws, o_prizes))
        return false;

    return true;
}

bool Game::calcPlayerPrizes(const PlayerID& playerId, unsigned& o_totalPrizes) const
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    if (!m_players.at(playerId).calcTotalPrizes(m_settings.cardProps(), m_draws, o_totalPrizes))
        return false;

    return true;
}

bool Game::addPlayerCredits(const PlayerID& playerId, const unsigned credits)
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    m_players.at(playerId).addCredits(credits);

    return true;
}

bool Game::getPlayerCredits(const PlayerID& playerId, unsigned& o_credits) const
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    o_credits = m_players.at(playerId).credits();

    return true;
}

bool Game::addPlayerExtraBall(const PlayerID& playerId, int extraBall)
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    if (!m_players.at(playerId).payForExtraBall(m_settings.maxExtraBalls(), m_settings.extraBallPrice(), extraBall))
        return false;

    return true;
}

bool Game::getPlayerExtraBalls(const PlayerID& playerId, std::set<int>& o_extraBalls) const
{
    if (m_players.find(playerId) == m_players.end())
        return false;

    o_extraBalls = m_players.at(playerId).extraBalls();

    return true;
}

void Game::getDraws(std::unordered_set<int>& o_draws) const
{
    o_draws = m_draws;
}

bool Game::roundActive() const
{
    return m_roundActive;
}

void Game::startRound()
{
    if (m_roundActive)
        return; // Already running

    m_draws.clear();

    for (auto& player : m_players)
    {
        // Convert player prizes to credits
        player.second.convertPrizesToCredits();

        // Eg: Player plays 1 credit to play
        player.second.payToPlay(m_settings.playPrice());

        // Add player cards
        player.second.resetCards(m_settings.maxCards(), m_settings.cardRows(), m_settings.cardCols(), m_settings.cardUnivMin(), m_settings.cardUnivMax());

    }

    // Unblock drawNewBall() calls, if there are players to play
    if (m_players.size())
    {
        m_roundActive = true;
        m_roundCount++;
    }
}

bool Game::drawNewBall()
{
    if (!m_roundActive)
        return false; // Game completed

    int newDraw{ Utils::random(m_settings.ballUnivMin(), m_settings.ballUnivMax()) };
    while(m_draws.find(newDraw) != m_draws.end())
        newDraw = Utils::random(m_settings.ballUnivMin(), m_settings.ballUnivMax());
    m_draws.insert(newDraw);

    // Block new drawNewBall() calls, until startRound()
    m_roundActive = m_draws.size() != m_settings.maxDraws();

    // Check player cards for prizes with the new draws. 
    // Note: If player didn't pay to play, play() is a noop.
    for (auto& player : m_players)
        player.second.play(m_settings.cardProps(), m_draws);

    // Write play event to log
    writeLogEntry();

    return true;
}

void Game::convertPrizesToCredits()
{
    for (auto& player : m_players)
        player.second.convertPrizesToCredits();
}

bool Game::setupLogDir(const std::string_view& logFileDir, std::string& o_error)
{
    m_logToFile = false;
    m_logFileDir = logFileDir;

    if (m_logFileDir.empty())
        return true;

    if (std::filesystem::exists(m_logFileDir) && std::filesystem::is_directory(m_logFileDir))
    {
        m_logToFile = true;
        return true;
    }

    o_error += "Error: Log file directory does not exist.\n";
    return false;
}

bool Game::writeLogEntry() const
{
    if (!m_logToFile || !std::filesystem::exists(m_logFileDir) || !std::filesystem::is_directory(m_logFileDir))
        return false;

    // Attempt to open the file
    std::filesystem::path logDir(m_logFileDir);
    std::filesystem::path sessionDir(std::to_string(m_sessionId));
    std::filesystem::path playDir(std::to_string(m_roundCount));
    std::filesystem::path filename("history.json");
    std::string fullFilePath{ (logDir / sessionDir / playDir / filename).string() };
    
    // Create sub directory
    std::filesystem::create_directories(logDir / sessionDir / playDir);

    nlohmann::json fileData;
    if (std::filesystem::exists(fullFilePath)) {
        std::ifstream file(fullFilePath);
        if (file.peek() != std::ifstream::traits_type::eof()) {
            try {
                file >> fileData;
                if (!fileData.is_array()) {
                    std::cerr << "Error: File exists but is not a JSON array.\n";
                    return false;
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing JSON: " << e.what() << "\n";
                return false;
            }
        }
        else {
            // File exists but is empty
            fileData = nlohmann::json::array();
        }
    }
    else {
        // File doesn't exist, start new array
        fileData = nlohmann::json::array();
    }

    // Append to file
    nlohmann::json newData;
    newData["Play"] = static_cast<int>(m_draws.size());
    newData["Draws"] = m_draws;

    nlohmann::json jsPlayers = nlohmann::json::array();
    nlohmann::json jsPlayer;
    nlohmann::json jsCards;
    nlohmann::json jsCard;
    for (auto& player : m_players)
    {
        jsCards = nlohmann::json::array(); // Reset
        for (auto& cardId : player.second.cardIDs())
        {
            unsigned prizes;
            player.second.calcCardPrizes(m_settings.cardProps(), cardId, m_draws, prizes);

            Card::IntMatrix2D content;
            player.second.cardContent(cardId, content);

            jsCard["ID"] = cardId;
            jsCard["Prizes"] = prizes;
            jsCard["Content"] = content;
            jsCards.push_back(jsCard);
        }

        unsigned prizes;
        player.second.calcTotalPrizes(m_settings.cardProps(), m_draws, prizes);

        jsPlayer["ID"] = player.first;
        jsPlayer["Credits"] = player.second.credits();
        jsPlayer["Total Prizes"] = prizes;
        jsPlayer["ExtraBalls"] = player.second.extraBalls();
        jsPlayer["Cards"] = jsCards;
        jsPlayers.push_back(jsPlayer);
    }

    newData["Players"] = jsPlayers;

    // Append Data
    std::ofstream file(fullFilePath);
    fileData.push_back(newData);
    file << fileData.dump(1);

    // 5. Check for write errors before closing
    if (file.fail())
        return false;

    file.close();
    return true;
}