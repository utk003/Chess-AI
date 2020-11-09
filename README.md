# DNN Chess AI

## Design Rationale
This project revolves around a neural-network-powered chess AI, whose design was heavily influenced by [Deepmind](https://en.wikipedia.org/wiki/DeepMind)'s [AlphaZero](https://en.wikipedia.org/wiki/AlphaZero) chess engine. AlphaZero used a similar approach with a [Monte Carlo tree search (MCTS)](https://en.wikipedia.org/wiki/Monte_Carlo_tree_search) and [deep](https://en.wikipedia.org/wiki/Deep_learning#Deep_neural_networks) [convolutional neural networks](https://en.wikipedia.org/wiki/Convolutional_neural_network) to create a powerful chess AI. While these projects share many design principles, my project differs implementationally from AlphaZero in many significant ways.

Primarily, my implementation uses a [deep](https://en.wikipedia.org/wiki/Deep_learning#Deep_neural_networks), [fully-connected, multi-layer](https://en.wikipedia.org/wiki/Multilayer_perceptron), [feed-forward](https://en.wikipedia.org/wiki/Feedforward_neural_network) [artificial neural network](https://en.wikipedia.org/wiki/Artificial_neural_network) as opposed to the deep convolutional neural network used by Deepmind. The reason for implementing a more fundamental network stems from the fact that more complex and mainstream network structures like [CNNs](https://en.wikipedia.org/wiki/Convolutional_neural_network) and [RNNs](https://en.wikipedia.org/wiki/Recurrent_neural_network) are just modified feedforward networks, either with more connections or preprocessing elements such as convolutional kernels. As a result, this project instead investigates the more fundamental network structure, the understanding and design of which can then both be expanded to the more complex networks. *TODO: Additional network implementations (specifically, a CNN) and training algorithms (such as [Q-learning](https://en.wikipedia.org/wiki/Q-learning) and [NEAT](https://en.wikipedia.org/wiki/Neuroevolution_of_augmenting_topologies)) are currently planned.*

Additionally, rather than using [ML](https://en.wikipedia.org/wiki/Machine_learning) libraries such as [TensorFlow](https://www.tensorflow.org/), my network is implemented from scratch, with my [dynamic-programming](https://en.wikipedia.org/wiki/Dynamic_programming), memory-optimized [backpropagation](https://en.wikipedia.org/wiki/Backpropagation) algorithm and a custom [dropout](https://en.wikipedia.org/wiki/Dilution_(neural_networks)) process. This decision again ties into the project goal of developing a deeper understanding of neural networks and their applications.

## Project Description
This is a [chess](https://en.wikipedia.org/wiki/Chess) game written in [C++](https://en.wikipedia.org/wiki/C%2B%2B). 

This program comes with a complete chess implementation, a versatile UI system, a host of different AI algorithms, including a neural-network-powered one, and a built-in network training system.

### The Chess Implementation
The program includes a robust chess implementation, which provides 100%-accurate chess gameplay. The system allows moves like [castling](https://en.wikipedia.org/wiki/Castling), [en passant](https://en.wikipedia.org/wiki/En_passant), and [pawn promotion](https://en.wikipedia.org/wiki/Promotion_(chess)), and it prevents players from making illegal moves such as putting their own king in check.

The source code for this part of this project can be found in the "src/chess" directory, in the piece.\* and game.\* files, and anyone can use it for other projects as outlined by the license.

### The UI
The included user interface, which was implemented using [OpenGL](https://www.opengl.org), uses high quality chess piece graphics derived from their respective emoji, and it is additionally capable of displaying past moves, current move selections, and current legal moves and captures. The previous move is indicated by a green outline around the start and end squares, the currently selected squares are shown with an orange outline, and the currently legal moves and captures are shown with blue and red outlines, respectively.

The UI also supports various keyboard input options, which play important roles in gameplay functionality and additional display options.
- Q, R, K, and B are the 4 keys necessary for pawn promotion. When your piece reaches the promotion rank, press the key corresponding to the piece to which you wish to promote.
- Z is the undo button. Its current implementation skips back to the previous human move (if 2 non-human players are playing, it does nothing). *TODO: The program may crash if the undo button is used while a non-human player is calculating its next move.*
- S is the game state save button. If you ever want to save the current game state for future analysis, this button will create a save file. This button currently does not store player data, so make sure you only save on white's turn.
- TAB is the expanded UI toggle button. If the expanded UI is enabled, the display output includes the blue and red outlines indicative of currently legal moves and captures. Otherwise, the UI only displays the green and orange outlines for previous moves and current grid selections.

The source code for the UI system used in this project can be found in the "src/graphics" directory, in the opengl.\* and shader.\* files. Additionally, the assets used for graphics can be found in the "assets" directory. Anyone can use these for other projects as outlined by the license.

*TODO: A 3D graphics system is currently planned, but no work has been done for this so far.*

### The Non-Human Player Algorithms
This project includes 6 different player options: the human player and 5 non-human player types. The program supports human vs human, human vs non-human, and non-human vs non-human games.
- The HUMAN player type is the only user-interactable type. Use this type in order to make your own moves. Any human-controlled moves can be made by clicking on the appropriate cells of the grid in the display.
- The RANDOM player type is the simplest non-human player. It plays a random legal move on its turn, with all possible moves being equally likely.
- The MINIMAX player type is the next non-human player type. It plays its move based on the [minimax](https://en.wikipedia.org/wiki/Minimax) algorithm.
- The ALPHA-BETA PRUNING player type is an optimized extension of the MINIMAX player. It plays using the [alpha-beta pruning](https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning) algorithm.
- The MONTE CARLO TREE SEARCH player type is the base class for the final AI player. It determines its moves using a probabilistic Monte Carlo tree search backed by random move selection.
- The NEURAL-NETWORK-POWERED AI player type is the final non-human player option, and it expands upn the MONTE CARLO TREE SEARCH player. It uses the same probabilistic Monte Carlo tree search, but backed by a deep neural network rather than random move selection.

All of the source code relating to the player logic (barring the network and the Monte Carlo tree search) can be found in the "src/player" directory in the player.\* files. The network and MCTS code can be found in the "src/mcts_network" directory in the decider.\*, network.\*, and tree.\* files. As usual, anyone can use any of the code relating to the players, the neural network, or the Monte Carlo tree search for other projects as outlined by the license.

### The Network Training System
Lastly, the project includes all of the code necessary to train the network. It can automatically create cases to train the network on and train the network on those cases in parallel. The training algorithm uses a custom-written and memory-optimized backpropagation algorithm as well as a weak-dilution dropout algorithm to prevent overfitting. Due to lack of resources such as computer time, as of November 8, 2020, the network is trained enough to play a decent early game, but it still fails to understand the important chess concepts like [material](https://en.wikipedia.org/wiki/Glossary_of_chess#material) and [tempo](https://en.wikipedia.org/wiki/Tempo_(chess)).

The source code for this part of this project can be found in the "src/main/network" directory, in the make_cases.\* and train.\* files, and anyone can use it for other projects as outlined by the license.

## Usage Intructions
Clone repository
Configure parameters in main.cpp (see Configuration Options)
Build/Run program

### Configuration Options
All of the configurable options are in src/main.cpp, mostly in the first method initialize(...). All of them are detailed by names/comments, and you can leave them at the default values if you desire.

Additionally, the execute() method in src/main.cpp is where you can specify running a game or training the network. Uncomment whichever process you wish to run.
If you choose to run execute_gameplay(), specify what player types you wish to play with in the method arguments. By default, the method runs a human vs AI game.

*TODO: An external, runtime-configurable, configuration file is planned.*

### Build/Run Program
Build the program as you normally would (see Additional Build Information)

Run the program from the parent directory of the "src" folder.
If the directory name is not "Chess-AI", provide the directory name or absolute directory path as an argument to the program when running it.

### Additional Build Information
This code compiles correctly with both [Clang](https://clang.llvm.org/) and [GCC](https://gcc.gnu.org/). In order to compile the program for yourself, you will need a few external libraries/header files: [GLFW](https://www.glfw.org/), [GLEW](http://glew.sourceforge.net/), [GSL](https://www.gnu.org/software/gsl/), [GLM](https://glm.g-truc.net/0.9.9/index.html), and [stb](https://github.com/nothings/stb).

This codebase can be compiled on both MacOS and Linux. WindowsOS support has been included but not tested.

Run [CMake](https://cmake.org/) from the directory of "CMakeLists.txt" to generate a makefile, which can then be used to compile the project itself.

## Changelog/Journal
See journal.txt for a push-to-push change/update log, my thoughts on the project, and the issues I encountered/how I fixed them.
