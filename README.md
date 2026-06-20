# BingoBackend

## Description

An example of a backend implementation of game of Bingo plugin, programmed in C++. 

The repository includes the plugin libraries for Windows and Linux with the a Bingo game logic, these can be loaded at runtime by other applications.

The repository also contains a sample application that can load the plugin at runtime.

The repository includes all of the source code used to build the targets yourself.

Targets were configured with CMake compiled with MSVC(Windows) and Ninja(Linux).

**Note:** Used under the MIT License, this repository includes copies of:

For json file utilities:

https://github.com/nlohmann/json/

For loading dlls at runtime (to keep changes minimum between OSs):

https://github.com/dlfcn-win32/dlfcn-win32

## Table of Contents
- [Libraries](#targets)
- [Usage](#usage)
- [Logic](#logic)
- [Build](#build)
- [Help Commands](#help-commands)
- [License](#license)
- [Author](#author)

## Libraries

For users trying to use the precompiled plugin libraries, they are located under:

### Windows:
- /plugin/lib/linux/BingoPlugin/libBingoPlugin.so
- /plugin/lib/linux/BingoPlugin/include/libBingoPlugin.h

### Linux:
- /plugin/lib/linux/BingoPlugin/\<Debug or Release>/BingoPlugin.dll
- /plugin/lib/linux/BingoPlugin/include/BingoPlugin.h

## Usage

Run the Bingo client application (no UI), that uses the backend plugin:
```console
.\BingoApp.exe -s <your_path>/settings.json -l <your_path>/logs
```
- It reads the game settings and rules from 'settings.json'
- It writes the game state of each play round to a log file (more details on the [Help Commands](#help-commands) section)

Run the Bingo plugin test application (no UI), that directly uses the plugin code (no plugin):
```console
.\BingoBackend.exe -s <your_path>/settings.json -l <your_path>/logs -t <your_path>/testing.json
```
- It reads the game settings and rules from 'settings.json'
- It writes the game state of each play round to a log file (more details on the [Help Commands](#help-commands) section)
- 'testing.json' enforces the intermediate states of the game, in between plays, so to check if the game is eprforming well (eg. if all the prizes are being evaluated correctly during play draws).

**Note:** for linux users binaries will not have the .'exe' suffix.

**Note:** the repository includes sample .json files for the latter examples.

## Logic

The plugin interface class 'class IBingoPlugin', in IBingoPlugin.h, serves as the API documentation, and the more details about game logic, as well as it's uses on both included 'main.cpp' files (for /plugin and /app).

The basic game logic is:

"You pay 1 credit to play. After the 30 draw balls , you get the first prize ( inverted pyramid ). You then win 10 credits."  

The programmed game loop consists of the following steps:
- Create and init a Game object, with a given set of settings. Eg:
```json
{
    "MaxPlayers": 5,        // Maximum number of players.
    "MaxCards": 5,          // Maximum number of cards.
    "BallUniverse": [1,60], // Numbers in the balls can only be in this interval.
    "CardUniverse": [1,60], // Numbers in the cards can only be in this interval.
    "MaxDraws": 30,         // A draw consists of X balls. Not all balls are drawn in each game.
    "MaxExtraBalls": 1,     // Number of additional balls that can be bought in a game.
    "CardSize": [3,5],      // Number of cells in the card.
    "CardPrizePatterns":[
                        {"Pattern": [[1,1,1,1,1],
                                     [0,1,1,1,0],
                                     [0,0,1,0,0]],  // The game’s prizes 
                         "Prize":10},               // The amount of credits a user wins if his card has a prize pattern.
                        {"Pattern": [[1,1,1,1,1],
                                     [0,0,1,0,0],
                                     [1,1,1,1,1]],  // The game’s prizes 
                         "Prize":50}                // The amount of credits a user wins if his card has a prize pattern.
                        ],
    "PlayPrice": 1,         // Amount of credits you pay to play .
    "ExtraBallPrice": 1     // Amount of credits you pay for an extra ball.
}
```
- Add players with their credits and extra balls.
```cpp
    plugin->addPlayer(playerId);
    plugin->addPlayerCredits(playerId, 1); // Add credit for the extra ball below
    plugin->addPlayerExtraBall(playerId, 15);
    plugin->addPlayerCredits(playerId, 1); // Add credit for play
```
- Now that the players are set, start a game round, it will clear all the players cards and make players pay the round entry fee. If players don't have credits to pay the entry fee, they'll remain idle during plays, can hold no cards and cannot win any prizes. Players that can payed to play the game, start with the maximum ammout of cards with random generated cell content.
```cpp
    game->roundActive(); // Evaluates to false
    plugin->startRound();
    game->roundActive(); // Evaluates to true
```
- To run the game until the end of the current round. I.e. draw new unique balls until the max draw count is reached. drawNewBall() will evaluate player states and check for prizes. When the max draws is reached, drawNewBall() will return false, and become a noop.
```cpp
    while (game->drawNewBall()); // Run the play rounds.
    game->roundActive(); // Evaluates to false
    
    std::unordered_set<int> draws;
    game->getDraws(draws); // Get the generated draws.
```
- At this state the game round has ended, to convert player prizes to credits call convertPrizesToCredits(), this closes a game loop.
```cpp
    game->convertPrizesToCredits();
    unsigned credits;
    game->getPlayerCredits(playerId, credits);
```
- This is the stage where players can be added/removed and a new round can start by calling startRound().



Repeat these steps to instantiate and play games on your apps.

## Build

### Windows:

To build the **BingoBakend** target use the **CMakeLists.txt** inside /plugin.

The targets will end up inside:
- /plugin/bin/\<win or linux>/BingoBakend/\<Debug or Release>/BingoBakend.exe

To build **BingoPlugin.dll** use the **CMakeLists.txt** inside /plugin.

The targets will end up inside:
- /plugin/lib/\<win or linux>/BingoPlugin/\<Debug or Release>/BingoPlugin.dll
- /plugin/lib/\<win or linux>/BingoPlugin/include/bingoPlugin.h

To build the **BingoApp** target use the **CMakeLists.txt** inside /app.

**Note:** **BingoApp.exe**'s **CMakeLists.txt** will link to the librareis within /plugin.

The target will end up inside:
- /app/bin/\<win or linux>/\<Debug or Release>/BingoApp.exe

### Linux:

Everything similar to the previous example on windows.

## Help Commands

Option flags for **BingoApp.exe**:

- Use -h to show this help message.
- [required] Use -s followed by the path to the settings file.
- [optional] Use -l followed by the path to the log file directory.

Option flags for **BingoBakend.exe**:

- Use -h to show this help message.
- [required] Use -s followed by the path to the settings .json file.
- [optional] Use -l followed by the path to the log file directory.
- [optional] Use -t followed by the path to the test .json configuration file.

**Note:** log entries will generate one folder per game session. These folders will have subfolders corresponding to game rounds (eg: '/0' '/1' '/2' '/3' etc...), each one of them with a 'history.json' file.

**Note:** the repository includes sample .json files for the latter examples, variations of these can be created to be used with the binaries.

## License

MIT License. You're free to use the code "as is".

## Author

Pedro Lino