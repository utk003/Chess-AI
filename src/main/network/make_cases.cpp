#include "make_cases.h"

#include <chrono>
#include <functional>

#include "../run_game.h"
#include "../../util/string_util.h"
#include "../../util/thread_util.h"

std::atomic_bool network::save_network(false);

double get_overall_result(double mcts_result, double game_result, double scale = 4.0) { // TODO tweak weights/scale
  static double mcts_weight = 5.0;
  static double game_result_weight = 1.0;
  static double weights_sum = mcts_weight + game_result_weight; // 6.0

  return scale * (mcts_weight * mcts_result + game_result_weight * game_result) / weights_sum; // 5:1 weight
}

long file_counter = 0L;
void run_simulation_threaded(const int ITERATION_INDEX, std::vector<std::string> &file_paths,
                             std::atomic_bool &is_complete) {
  game::GameResult result = game::run_game(player::PlayerType::AI, player::PlayerType::AI);

  // Update training case target value
  for (auto &pair : network::training_boards) {
    pair.second = get_overall_result(pair.second, result.evaluate()); // update case weight
    file_paths.push_back(network::save_training_case(std::to_string(file_counter++), pair)); // save case to file
    delete pair.first;
  }
  network::training_boards.clear();

  if (settings::PRINT_GAME_SIMULATION_DEBUG_INFORMATION) {
    // print iteration completion time
    std::chrono::system_clock::time_point time_on_start = std::chrono::system_clock::now();
    time_t time = std::chrono::system_clock::to_time_t(time_on_start);
    std::cout << "Finished Generating Cases for Game #" << ITERATION_INDEX << ": " << ctime(&time);
  }

  is_complete.store(true);
}
void run_simulation(const int ITERATION_INDEX, std::vector<std::string> &file_paths) {
  std::atomic_bool is_complete(false);
  run_simulation_threaded(ITERATION_INDEX, file_paths, is_complete);
}

void network::generate_training_cases(std::vector<std::string> &file_paths, int &sim_count, int num_simulations) {
  generate_training_cases(settings::TRAINING_TERMINATION_CONDITION, file_paths, sim_count, num_simulations);
}
void network::generate_training_cases(const std::function<bool()> &termination_condition,
                                      std::vector<std::string> &file_paths,
                                      int &sim_count, int num_simulations) {
  network::NetworkStorage::setTestCaseSelector([&](game::Board *b, double d) -> void {
    settings::SIMULATION_BOARD_SAVE_PROCEDURE(training_boards, b, d);
  });

  if (!termination_condition()) {
    if (settings::PRINT_GAME_SIMULATION_DEBUG_INFORMATION)
      std::cout << "GAME SIMULATION TIMESTAMP DATA:" << std::endl;

    if (num_simulations <= 1) {
      while (!termination_condition())
        run_simulation(++sim_count, file_paths);
    } else {
      std::vector<std::atomic_bool> sim_checkers(num_simulations);
      for (int i = 0; i < num_simulations; ++i)
        sim_checkers[i].store(true);

      int cycle_counter = 0;
      thread::do_while_waiting_for([&] {
        for (auto &it: sim_checkers)
          if (it) {
            it.store(false);
            thread::create(run_simulation_threaded, ++sim_count, std::ref(file_paths), std::ref(it));
            ++cycle_counter;
          }
        if (cycle_counter >= num_simulations) {
          cycle_counter -= num_simulations;
          save_network.store(true); // TODO sync better with save in train.cpp @ ~line 36
        }
      }, termination_condition);
      thread::wait_for([&] {
        return !std::any_of(sim_checkers.begin(), sim_checkers.end(), [](std::atomic_bool &b) { return !b.load(); });
      });
    }
  }
}

std::string
network::save_training_case(const std::string &case_index, const std::pair<game::Board *, double> &training_case) {
  std::string file_path = "training_cases/case_" + case_index + ".txt";
  training_case.first->saveToFile(
    file_path, [&](std::ostream &out) -> void { out << string::from_double(training_case.second) << std::endl; }, false
  );
  return file_path;
}
std::pair<game::Board *, double> network::load_training_case(const std::string &file_path, bool pad_file_path) {
  double weight;
  auto *b = new game::Board(8, 8);
  b->loadFromFile(pad_file_path ? "training_cases/" + file_path + ".txt": file_path,
                  [&](std::istream &in) -> void { in >> weight; });
  return {b, weight};
}