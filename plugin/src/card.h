#pragma once

#include "settings.h"

#include <vector>
#include <set>

class GameTest;

// Class Card serves as a coitnainer for a random clsuter of numbers.
// On eval() the card can be checked for prizes, based on the draws and a card pattern.
class Card
{

public:
    typedef int CardID;
    typedef std::vector<std::vector<int>> IntMatrix2D;

    Card() {};
    Card(unsigned rows, unsigned cols, int univMin, int univMax);
    const IntMatrix2D& content() const { return m_content; };
    void reset();
    bool valid() const;

    bool eval(const std::set<int>& draws, const Settings::BoolMatrix2D& pattern) const;

private:
    IntMatrix2D m_content;
    unsigned m_cols{ 0 };
    unsigned m_rows{ 0 };
    int m_univMin{ 0 };
    int m_univMax{ 0 };

friend GameTest;
};