#include "player.h"
#include "utils.h"

#include <algorithm>

bool Player::cardContent(const Card::CardID& id, Card::IntMatrix2D& o_content) const
{
    o_content = Card::IntMatrix2D();

    if (m_cards.find(id) == m_cards.end())
        return false;

    o_content = m_cards.at(id).content();
    return true;
}

std::set<Card::CardID> Player::cardIDs() const
{
    std::set<int> ids;

    for (auto& card : m_cards)
        ids.insert(card.first);
    return ids;
}

bool Player::calcCardPrizes(const std::vector<Settings::CardProps>& cardProps, const Card::CardID& id, const std::unordered_set<int>& draws, unsigned& o_totalPrizes) const
{
    o_totalPrizes = 0;

    if (m_cards.find(id) == m_cards.end())
        return false;

    // Combine numbers
    // # Challenge 4 use Extra Balls as Extra Draws
    std::set<int> totalDraws(draws.begin(), draws.end());
    totalDraws.insert(m_extraBalls.begin(), m_extraBalls.end());

    auto& card = m_cards.at(id);
    for (auto cardProp : cardProps)
    {
        if (card.eval(totalDraws, cardProp.pattern))
            o_totalPrizes += cardProp.prize; // Accumulate prizes
    }

    return true;
}

bool Player::calcTotalPrizes(const std::vector<Settings::CardProps>& cardProps, const std::unordered_set<int>& draws, unsigned& o_totalPrizes) const
{
    o_totalPrizes = 0;

    unsigned cardPrizes;
    for (auto& card : m_cards)
    {
        if (!calcCardPrizes(cardProps, card.first, draws, cardPrizes))
            return false;        
        o_totalPrizes += cardPrizes; // Accumulate won prizes
    }

    return true;
}

bool Player::payForExtraBall(unsigned maxExtraBalls, unsigned price, int extraBall)
{
    if (std::find(m_extraBalls.begin(), m_extraBalls.end(), extraBall) != m_extraBalls.end())
        return false;

    if (maxExtraBalls < m_extraBalls.size() + 1)
        return false;

    if ((int)m_credits - (int)price < 0)
        return false;

    m_credits -= price;

    m_extraBalls.insert(extraBall);
    return true;
}

void Player::payToPlay(unsigned price)
{
    if ((int)m_credits - (int)price < 0)
    {
        m_credits = 0;
        m_canPlay = false;
    }
    else if (m_credits == 0)
    {
        m_canPlay = false;
    }
    else
    {
        m_credits -= price;  
        m_canPlay = true;
    }
}

void Player::resetCards(unsigned maxCards, unsigned cardRows, unsigned cardCols, unsigned cardUnivMin, unsigned  cardUnivMax)
{
    m_cards.clear();
    for (unsigned idx=0; idx < maxCards; idx++)
    {
        Card newCard(cardRows, cardCols, cardUnivMin, cardUnivMax);
        if (!newCard.valid())
            return; // Error
        m_cards.insert({ Card::CardID(idx), newCard });
    }
}

bool Player::play(const std::vector<Settings::CardProps>& cardProps, const std::unordered_set<int>& draws)
{
    if (!m_canPlay)
        return false;

    // Update Prices
    if (!calcTotalPrizes(cardProps, draws, m_prizes))
        return false;

    return true;
}

void Player::convertPrizesToCredits()
{
    m_canPlay = false;
    m_cards.clear();
    m_credits += m_prizes;
    m_prizes = 0;
}