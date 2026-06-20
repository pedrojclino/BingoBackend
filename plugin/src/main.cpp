#include "game.h"
#include "gametest.h"

#include <iostream>

struct Inputs
{
    std::string settingsPath;
    std::string testingFilePath;
    std::string logFileDir;
};

void parseArgs(int argc, char* argv[], Inputs& o_inputs)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help")
        {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                << "  Use -h to show this help message.\n"
                << "  [required] Use -s followed by the path to the settings .json file.\n"
                << "  [optional] Use -t followed by the path to the test .json configuration file.\n"
                << "  [optional] Use -l followed by the path to the log file directory.\n";
        }
        else if (arg == "-s" && i+1 < argc)
        {
            o_inputs.settingsPath = argv[i+1];
        }
        else if (arg == "-t" && i + 1 < argc)
        {
            o_inputs.testingFilePath = argv[i + 1];
        }
        else if (arg == "-l" && i + 1 < argc)
        {
            o_inputs.logFileDir = argv[i + 1];
        }
    }
}

int main(int argc, char* argv[])
{
    Inputs inputs;
    parseArgs(argc, argv, inputs);

    Game game;
    std::string err;
    if (!game.init(inputs.settingsPath, inputs.logFileDir, err))
    {
        std::cerr << err << std::endl;
        std::cerr << "Failed to load settings!" << std::endl;
        return -1;
    }

    if (inputs.testingFilePath.empty())
    {
        // Testing site for a normal game with a random number distribution
        Game::PlayerID playerId{ -1 };
        if (!game.addPlayer(playerId))
        {
            std::cout << "Failed to add player\n";
            return -1;
        }
        if (!game.addPlayerCredits(playerId, 1)) // Add credit for the extra ball below
        {
            std::cout << "Failed to add player credits\n";
            return -1;
        }
        if (!game.addPlayerExtraBall(playerId, 15))
        {
            std::cout << "Failed to add player extra balls\n";
            return -1;
        }

        if (!game.addPlayerCredits(playerId, 1)) // Add credit for play
        {
            std::cout << "Failed to add player credits\n";
            return -1;
        }

        // Start a game round
        if (game.roundActive())
        {
            std::cout << "Rounds started as active\n";
            return -1;
        }

        std::cout << "> Starting game round...\n";
        game.startRound();

        // Write Player stats before run
        std::cout << "Player ID: " << playerId << "\n";
        unsigned credits;
        game.getPlayerCredits(playerId, credits);
        std::cout << " Start Credits: " << credits << "\n";
        std::cout << " Extra Balls: ";
        std::set<int> extraBalls;
        game.getPlayerExtraBalls(playerId, extraBalls);
        for (auto& extraBall : extraBalls)
            std::cout << extraBall << " ";
        std::cout << "\n";

        // Write Card
        std::cout << " Card ID: " << playerId << ":\n";
        Game::CardContent content;
        std::set<Game::CardID> cardIDs;
        if (!game.getPlayerCardIDs(playerId, cardIDs))
        {
            std::cout << "Failed to get player cards\n";
            return -1;
        }

        std::cout << "  -----------------\n";

        for (auto cardID : cardIDs)
        {
            game.getPlayerCard(playerId, cardID, content);
            for (auto& rows : content)
            {
                std::cout << "  ";
                for (auto& elem : rows)
                    std::cout << elem << " ";
                std::cout << "\n";
            }
            std::cout << "  -----------------\n";
        }

        if (!game.roundActive())
        {
            std::cout << "Rounds not active after restart\n";
            return -1;
        }

        // Run game
        std::cout << "> Running the game until the end...\n";
        while (game.drawNewBall());

        std::cout << "Draws:\n";
        std::unordered_set<int> draws;
        game.getDraws(draws);
        for (auto& draw : draws)
            std::cout << draw << " ";
        std::cout << "\n";

        // Write Player stats after run
        std::cout << "Player ID: " << playerId << "\n";
        game.convertPrizesToCredits();
        game.getPlayerCredits(playerId, credits);
        std::cout << " End Credits: " << credits << "\n";

        std::cout << "> Exiting...\n";
    }
    else
    {
        // Testing site for a prepared test modes

        GameTest gameTest;
        gameTest.init(inputs.settingsPath, inputs.logFileDir);
        if (!gameTest.loadTestSequence(inputs.testingFilePath))
        {
            std::cout << "Failed to load test settings!\n";
            return -1;
        }
        if (!gameTest.validateFullGame(/*writeLogs*/ true))
        {
            std::cout << "Failed while running tests!\n";
            return -1;
        }
        std::cout << "Tests passed!\n";
    }

    return 0;
}