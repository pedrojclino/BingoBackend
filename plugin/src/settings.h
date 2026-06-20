#pragma once 

#include <vector>
#include <string>

class GameTest;

class Settings
{
public:
    Settings() {};

    typedef std::vector<std::vector<bool>> BoolMatrix2D;

    struct CardProps
    {
        CardProps(const BoolMatrix2D& arg0, int arg1) : pattern{ arg0 }, prize{ arg1 } {}
        BoolMatrix2D pattern;
        int prize{ -1 };
    };

    static const std::string maxPlayersKey;
    static const std::string maxCardsKey;
    static const std::string ballUniverseKey;
    static const std::string cardUniverseKey;
    static const std::string maxDrawsKey;
    static const std::string maxExtraBallsKey;
    static const std::string cardSizeKey;
    static const std::string cardPrizePatternsKey;
    static const std::string cardPatternKey;
    static const std::string cardPrizeKey;
    static const std::string playPriceKey;
    static const std::string extraBallPriceKey;

    bool loadSettings(const std::string_view& filepath, std::string& o_err);

    int ballUnivMin() const { return m_ballUnivMin; };
    int ballUnivMax() const { return m_ballUnivMax; };

    int cardUnivMin() const { return m_cardUnivMin; };
    int cardUnivMax() const { return m_cardUnivMax; };

    unsigned cardRows() const { return m_cardRows; };
    unsigned cardCols() const { return m_cardCols; };

    unsigned maxPlayers() const { return m_maxPlayers; };

    unsigned maxCards() const { return m_maxCards; };

    unsigned maxDraws() const { return m_maxDraws; };

    unsigned maxExtraBalls() const { return m_maxExtraBalls; };

    unsigned playPrice() const { return m_playPrice; };

    unsigned extraBallPrice() const { return m_extraBallPrice; };

    const std::vector<CardProps>& cardProps() const { return m_cardPrizePatterns; };

private:

    unsigned m_maxPlayers{ 0 };
    unsigned m_maxCards{ 0 };
    int m_ballUnivMin{ -1 };
    int m_ballUnivMax{ -1 };
    int m_cardUnivMin{ -1 };
    int m_cardUnivMax{ -1 };
    unsigned m_maxDraws{ 0 };
    unsigned m_maxExtraBalls{ 0 };
    unsigned m_cardRows{ 0 };
    unsigned m_cardCols{ 0 };
    std::vector<CardProps> m_cardPrizePatterns;
    unsigned m_playPrice{ 0 };
    unsigned m_extraBallPrice{ 0 };

friend GameTest;
};
