#include "card.h"
#include "utils.h"

#include <algorithm>
#include <unordered_set>

#ifndef NDEBUG
#include <cassert>
#endif

Card::Card(unsigned rows, unsigned cols, int univMin, int univMax) :
    m_rows(rows),
    m_cols(cols),
    m_univMin(univMin),
    m_univMax(univMax)
{
    reset();
}

bool Card::eval(const std::set<int>& draws, const Settings::BoolMatrix2D& pattern) const
{
    if (!valid())
        return false;

    for (unsigned i = 0, j = 0; i < m_rows; i++)
    {
        if (pattern.size() <= i)
            return false;

        for (j = 0; j < m_cols; j++)
        {
            if (pattern.at(i).size() <= j)
                return false;

            // On Pattern Match
            if (pattern.at(i).at(j))
                // On i,j in Draws
                if (std::find(draws.begin(), draws.end(), m_content.at(i).at(j)) == draws.end())
                    return false;
        }
    }

    return true;
}

bool Card::valid() const
{
    size_t size{ 0 };
    for (auto& row : m_content)
        size += row.size();

    // Additinal check 
#ifndef NDEBUG
    std::set<int> checker;
    for (auto& rows : m_content)
    {
        for (auto& elem : rows)
        {
            assert(checker.find(elem) == checker.end());
            checker.insert(elem);
        }
    }
#endif

    return static_cast<unsigned>(size) == (m_rows * m_cols) && (m_rows > 0) && (m_cols > 0);
}

void Card::reset()
{
    // Create a random card
    m_content.clear();
    
    int expectedSize = m_rows * m_cols;
    if ( expectedSize == 0 || (m_univMax-m_univMin) < expectedSize )
        return; // Cannot proceed
    
    // Create random content
    std::unordered_set<int> uniqueContent;
    while(uniqueContent.size() != expectedSize)
        uniqueContent.insert(Utils::random(m_univMin, m_univMax));
        

    std::vector<int> uniqueContentVec{ uniqueContent.begin(), uniqueContent.end() };

    // Copy to a more readable
    m_content.reserve(m_rows);
    std::vector<int> row;
    for (unsigned i = 0, j = 0; i < m_rows; i++)
    {
        row.clear();
        row.reserve(m_cols);
        for (j = 0; j < m_cols; j++)
        {
            row.push_back(uniqueContentVec.at(j + i * m_cols));
        }
        m_content.push_back(row);
    }
}