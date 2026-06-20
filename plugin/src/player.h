#pragma once

#include "settings.h"
#include "card.h"

#include <set>
#include <unordered_set>
#include <map>

class GameTest;


// Class Player manages card containers and performs checks on these on each play event,
// the class also holds and monitors each player credits and prizes, as well as if it can play or not.
// Card data can be accessed in a protected manner.
// The player cannot play without credits, use addCredits().
// With credits, the player can payToPlay(), it'll enable a flag so each play() may accumulate prizes.
// On finish() prizes are converted to credits, and to play() again the player has to payToPlay().
// Note: player object properties are intentionally agnostic to the settings of the game.
class Player
{
public:
    Player() {};

    bool cardContent(const Card::CardID& id, Card::IntMatrix2D& o_content) const;
    std::set<Card::CardID> cardIDs() const;

    bool calcCardPrizes(const std::vector<Settings::CardProps>& cardProps, const Card::CardID& id, const std::unordered_set<int>& draws, unsigned& o_totalPrizes) const;

    bool calcTotalPrizes(const std::vector<Settings::CardProps>& cardProps, const std::unordered_set<int>& draws, unsigned& o_totalPrizes) const;

    bool calcAvailablePrizes(int& availablePrizes) const;

    void addCredits(unsigned credits) { m_credits += credits; }
    unsigned credits() const { return m_credits; }
    unsigned prizes() const { return m_prizes; }

    bool payForExtraBall(unsigned maxExtraBalls, unsigned price, int extraBall);
    std::set<int> extraBalls() const { return m_extraBalls; }

    void payToPlay(unsigned price);
    void resetCards(unsigned maxCards, unsigned cardRows, unsigned cardCols, unsigned cardUnivMin, unsigned  cardUnivMax);
    bool canPlay() { return m_canPlay; }
    bool play(const std::vector<Settings::CardProps>& cardProps, const std::unordered_set<int>& draws);
    void convertPrizesToCredits();

private:

    bool m_canPlay{ false };
    unsigned m_credits{ 0 };
    unsigned m_prizes{ 0 };
    std::set<int> m_extraBalls;
    std::map<Card::CardID,Card> m_cards;


friend GameTest;
};