#include "main/initialization.h"
#include "main/run_game.h"
#include "main/network/make_cases.h"
#include "main/network/train.h"

// command line arguments
static char **arguments;
static int num_args;

// The initialize() method initializes all of the different modules of this program
// and ensures that all necessary preconditions are met.
//
// Initialization can be verified using init::verify(), which will return true iff
// the program believes that it is ready to run any of its features, including but
// not limited to training the network and playing a game with graphics (using OpenGL).
void initialize() {
  num_args <= 0 ? init::updateWorkingDirectory(): init::updateWorkingDirectory(arguments[0]);

  init::initializeRNG();

  init::updateMCTSParameters(0.15);

  init::updateNetworkSettings(false);
  init::updateTrainingParameters();
}

// The execute() method is the core of the entire program, where all of the independent
// functions provided in this program can be run. Specifically, the program can decide
// to run training procedures or play an actual game between 2 human players, between
// a human and a computer, or even between 2 computers!
//
// There is planned support for external (possibly command-line-based) modifiers which
// will allow users to configure program usage at runtime rather than compile-time.
//
// There is also a planned UI which will allow users to select configurations while the
// program is running. However, this is not a high-priority feature.
void execute() {

}

// The terminate() method deletes any pointers, etc. and clears any containers.
// It's main role is to prevent memory leaks by responsibly managing data.
//
// The method effectively acts as a reset switch, by clearing most settings
// and freeing memory allocated during the initialization phase of the program.
//
// This method can be thought of as the inverse of the initialize() method.
void terminate() {
  network::NetworkStorage::flushStorage(); // delete any stored networks
}

// The main() method is the main method from where the initialize(), execute(),
// and terminate() methods are called. It also saves any command-line arguments.
int main(int len, char **args) {
  // shift arguments by 1 b/c args[0] is the command used to run the program??
  arguments = args + 1;
  num_args = len - 1;

  // initialize, run, and close program
  initialize();
  execute();
  terminate();

  // exit
  return 0;
}