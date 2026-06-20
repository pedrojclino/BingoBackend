#include "settings.h"
#include "json.hpp"

#include <fstream>

const std::string Settings::maxPlayersKey = "MaxPlayers";
const std::string Settings::maxCardsKey = "MaxCards";
const std::string Settings::ballUniverseKey = "BallUniverse";
const std::string Settings::cardUniverseKey = "CardUniverse";
const std::string Settings::maxDrawsKey = "MaxDraws";
const std::string Settings::maxExtraBallsKey = "MaxExtraBalls";
const std::string Settings::cardSizeKey = "CardSize";
const std::string Settings::cardPrizePatternsKey = "CardPrizePatterns";
const std::string Settings::cardPatternKey = "Pattern";
const std::string Settings::cardPrizeKey = "Prize";
const std::string Settings::playPriceKey = "PlayPrice";
const std::string Settings::extraBallPriceKey = "ExtraBallPrice";

namespace
{
    void appendErrorMessage(const std::string& ketField, std::string& o_err)
    {
        o_err += "Could not find '" + ketField + "' field in settings file!\n";
    }
}


bool Settings::loadSettings(const std::string_view& filepath, std::string& o_err)
{
    std::ifstream file(filepath.data());

    // Check if file opened successfully
    if (!file.is_open())
    {
        o_err += "Could not open settings file!";
        return false;
    }

    // Parse the JSON content
    nlohmann::json data;
    file >> data;

    // Load fields
    if (!data.contains(Settings::maxPlayersKey))
    {
        appendErrorMessage(Settings::maxPlayersKey, o_err);
        return false;
    }
    m_maxPlayers = data[Settings::maxPlayersKey];

    if (!data.contains(Settings::maxCardsKey))
    {
        appendErrorMessage(Settings::maxCardsKey, o_err);
        return false;
    }
    m_maxCards = data[Settings::maxCardsKey];

    if (!data.contains(Settings::ballUniverseKey) || data[Settings::ballUniverseKey].size() !=2 )
    {
        appendErrorMessage(Settings::ballUniverseKey, o_err);
        return false;
    }
    m_ballUnivMin = data[Settings::ballUniverseKey][0];
    m_ballUnivMax = data[Settings::ballUniverseKey][1];
    if (m_ballUnivMin <=0 || m_ballUnivMax <=0 || m_ballUnivMax < m_ballUnivMin)
    {
        o_err = "Invalid ball universe!\n";
        return false;
    }

    if (!data.contains(Settings::cardUniverseKey) || data[Settings::cardUniverseKey].size() != 2)
    {
        appendErrorMessage(Settings::cardUniverseKey, o_err);
        return false;
    }
    m_cardUnivMin = data[Settings::cardUniverseKey][0];
    m_cardUnivMax = data[Settings::cardUniverseKey][1];
    if (m_cardUnivMin <=0 || m_cardUnivMax <=0 || m_cardUnivMax < m_cardUnivMin)
    {
        o_err = "Invalid card universe!\n";
        return false;
    }

    if (!data.contains(Settings::maxDrawsKey))
    {
        appendErrorMessage(Settings::maxDrawsKey, o_err);
        return false;
    }
    m_maxDraws = data[Settings::maxDrawsKey];

    if (!data.contains(Settings::maxExtraBallsKey))
    {
        appendErrorMessage(Settings::maxExtraBallsKey, o_err);
        return false;
    }
    m_maxExtraBalls = data[Settings::maxExtraBallsKey];

    if (!data.contains(Settings::cardSizeKey) || data[Settings::cardSizeKey].size() != 2)
    {
        appendErrorMessage(Settings::cardSizeKey, o_err);
        return false;
    }
    m_cardRows = data[Settings::cardSizeKey][0];
    m_cardCols = data[Settings::cardSizeKey][1];
    if (m_cardRows <= 0 || m_cardCols <= 0)
    {
        o_err = "Invalid card dimensions!\n";
        return false;
    }
    
    if (!data.contains(Settings::cardPrizePatternsKey))
    {
        appendErrorMessage(Settings::cardPrizePatternsKey, o_err);
        return false;
    }

    auto cardPrizePatterns = data[Settings::cardPrizePatternsKey];
    size_t cardCount = cardPrizePatterns.size();
    m_cardPrizePatterns.clear();
    for (int card = 0; card < cardCount; card++)
    {
        auto cardPrizePattern = cardPrizePatterns[card];
        if (!cardPrizePattern.contains(Settings::cardPrizeKey) || !cardPrizePattern.contains(Settings::cardPatternKey))
        {
            appendErrorMessage(Settings::cardPrizeKey, o_err);
            return false;
        }

        std::vector<std::vector<bool>> pattern;
        int prize = cardPrizePattern[Settings::cardPrizeKey];
        auto cardPattern = cardPrizePattern[Settings::cardPatternKey];
        if (cardPattern.size() != m_cardRows)
        {
            o_err += "Invalid 'Pattern' size on a settings pattern!\n";
            return false;
        }

        for (unsigned i = 0, j = 0; i < m_cardRows;i++)
        {
            if (cardPattern[i].size() != m_cardCols)
            {
                o_err += "Invalid 'Pattern' size on a settings pattern!\n";
                return false;
            }

            std::vector<bool> line;
            for (j = 0; j < m_cardCols;j++)
            {
                line.push_back((int)cardPattern[i][j]);
            }
            pattern.push_back(line);
        }
        
        m_cardPrizePatterns.push_back(CardProps(pattern, prize));
    }

    if (!data.contains(Settings::playPriceKey))
    {
        appendErrorMessage(Settings::playPriceKey, o_err);
        return false;
    }
    m_playPrice = data[Settings::playPriceKey];

    if (!data.contains(Settings::extraBallPriceKey))
    {
        appendErrorMessage(Settings::extraBallPriceKey, o_err);
        return false;
    }
    m_extraBallPrice = data[Settings::extraBallPriceKey];
    return true;
}