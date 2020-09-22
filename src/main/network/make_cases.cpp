#include "make_cases.h"

#include "../initialization.h"
#include "../run_game.h"
#include "../../util/string_util.h"

double get_overall_result(double mcts_result, double game_result, double scale = 4.0) { // TODO tweak weights/scale
  static double mcts_weight = 5.0;
  static double game_result_weight = 1.0;
  static double weights_sum = mcts_weight + game_result_weight; // 6.0

  return scale * (mcts_weight * mcts_result + game_result_weight * game_result) / weights_sum; // 5:1 weight
}

long file_counter = 0L;
void run_generation_iteration(const int ITERATION_INDEX) {
  game::GameResult result = game::run_game(player::PlayerType::AI, player::PlayerType::AI);

  // Update training case target value
  for (auto &pair : network::training_boards) {
    pair.second = get_overall_result(pair.second, result.evaluate()); // update case weight
    network::save_training_case("training_case/case_" + std::to_string(file_counter++), pair); // save case to file
    delete pair.first;
  }
  network::training_boards.clear();

//  // Save network "if necessary"
//  if (ITERATION_INDEX % settings::GAMES_PER_NETWORK_SAVE == 0)
//    network::NetworkStorage::saveNetwork(); // TODO shift to where it is actually needed (in training loop)

  if (settings::PRINT_BOARD_SIMULATION_DATA) {
    // print iteration completion time
    std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
    time_t time1 = std::chrono::system_clock::to_time_t(time_on_start);
    std::cout << "Finished Generating Cases for Game #" << ITERATION_INDEX << ": " << ctime(&time1);
  }
}

bool network::generate_training_cases(const std::function<bool()> &termination_condition) {
  if (!init::verify())
    return false;

  network::NetworkStorage::setTestCaseSelector([&](game::Board *b, double d) -> void {
    settings::SIMULATION_BOARD_SAVE_PROCEDURE(training_boards, b, d);
  });

//  int i = 0;
//  while (!termination_condition()) {
//    run_generation_iteration(++i); // TODO unlimited case generation
//  }
  for (int i = 1; i <= settings::NUMBER_OF_GAMES_TO_SIMULATE; ++i)
    run_generation_iteration(i);

  return true;
}

void network::save_training_case(const std::string &file_path, const std::pair<game::Board *, double> &training_case) {
  training_case.first->saveToFile(file_path, [&](std::ostream &out) -> void {
    out << string::from_double(training_case.second) << std::endl;
  });
}
std::pair<game::Board *, double> network::load_training_case(const std::string &file_path) {
  double weight;
  auto *b = new game::Board(8, 8);
  b->loadFromFile(file_path, [&](std::istream &in) -> void { in >> weight; });
  return {b, weight};
}