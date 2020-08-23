#ifndef CHESS_AI_MCTS_NETWORK_NETWORK_H_
#define CHESS_AI_MCTS_NETWORK_NETWORK_H_

#include "network.fwd.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <cmath>
#include <functional>
#include <fstream>

#include "decider.h"
#include "../chess/piece.fwd.h"
#include "../chess/game.h"
#include "tree.fwd.h"

namespace network {

class Network : public decider::Decider {
    friend class NetworkManager;

  public:
    Network(const Network &d) = delete;
    Network &operator=(const Network &d) = delete;

    Network(); // default: 64-225-100-1 (for now)
    explicit Network(std::vector<int> dims); // dimensions of network - starts off fully connected
    ~Network() override;

    static Network *randomNetwork();
    static Network *randomNetwork(const std::vector<int> &dims);
    static void randomNetwork(Network *network);

    static Network *uniformNetwork();
    static Network *uniformNetwork(const std::vector<int> &dims);
    static void uniformNetwork(Network *network);

    Network *clone();
    static Network *clone(Network *network);

    friend std::istream &operator>>(std::istream &input, Network *&net);
    friend std::ostream &operator<<(std::ostream &output, Network *&net);

    static Network *loadFromFile(const std::string &file_path);

  protected:
    double predictPosition(game::Board *b) override;

  private:
    void updateInternals(const std::vector<int> &dimensions);

    void loadBoard(game::Board *b, std::vector<double> &input);
    void propagate_for_training();
    void propagate_for_training(std::vector<std::vector<double>> &unbounded);

    constexpr static double NEURON_THRESHOLD_MAX = 4.0;
    constexpr static double NEURON_THRESHOLD_WIDTH = 4.0; // 16777216.0;
    [[nodiscard]] static inline double f(double x) {
      return NEURON_THRESHOLD_MAX * modifiedSigmoid(x / NEURON_THRESHOLD_WIDTH);
    }
    [[nodiscard]] static inline double df(double x) {
      const double sig = modifiedSigmoid(x / NEURON_THRESHOLD_WIDTH);
      return NEURON_THRESHOLD_MAX * (1.0 - sig * sig) / NEURON_THRESHOLD_WIDTH;
    }

    // Sigmoid centered at (0,0) ranging from -1 to 1 w/ derivative = 1 @ (0,0)
    [[nodiscard]] static inline double modifiedSigmoid(double x) { return 2.0 / (1.0 + exp(-2.0 * x)) - 1.0; }


    int _num_layers, _input_index, _output_index, _max_width;
    std::vector<int> _dimensions;

    std::vector<std::vector<double>> _neurons;

    std::vector<std::vector<double>> _thetas; // unbounded neuron values
    std::vector<std::vector<double>> _omegas; // intermediate weight change measure

    std::vector<std::vector<std::vector<double>>> _connections;
    std::vector<std::vector<std::vector<double>>> _connection_changes;
};

class NetworkManager {
  public:
    NetworkManager(const NetworkManager &nm) = delete;
    NetworkManager &operator=(const NetworkManager &nm) = delete;

  protected:
    NetworkManager() = default;
    ~NetworkManager() = default;

    inline static void loadBoard(Network *network, game::Board *b) { network->loadBoard(b, network->_neurons[0]); }
    inline static void propagate(Network *network) { network->propagate_for_training(); }
    inline static void propagate(Network *network, std::vector<std::vector<double>> &unbounded) {
      network->propagate_for_training(unbounded);
    }

    inline static std::pair<int, std::pair<int, int>> get_other_data(Network *network) {
      return {network->_num_layers, {network->_input_index, network->_output_index}};
    }
    inline static std::vector<int> &get_dimensions(Network *network) { return network->_dimensions; }

    inline static std::vector<std::vector<double>> &get_neurons(Network *network) { return network->_neurons; }
    inline static std::vector<std::vector<std::vector<double>>> &
    get_connections(Network *network) { return network->_connections; }
    inline static std::vector<std::vector<std::vector<double>>> &
    get_delta_conns(Network *network) { return network->_connection_changes; }

    inline static std::vector<std::vector<double>> &thetas_array(Network *network) { return network->_thetas; }
    inline static std::vector<std::vector<double>> &omegas_array(Network *network) { return network->_omegas; }

    inline static double threshold(double x) { return network::Network::f(x); }
    inline static double threshold_deriv(double x) { return network::Network::df(x); }
};

class Optimizer : NetworkManager {
  public:
    static void
    optimize(Network *network, game::Board *input, double TARGET_OUTPUT, double &lambda, double MAX_LEARNING_CONSTANT,
             double LEARNING_RATE);

    static constexpr double DEFAULT_MAX_LEARNING_CONSTANT = 15.0;
    static constexpr double DEFAULT_LEARNING_RATE = 1.1;
    static constexpr double DEFAULT_INITIAL_LAMBDA = 2.0;

    static constexpr int DEFAULT_TRAINING_REPETITIONS = 5;
};

class NetworkStorage : NetworkManager {
  public:
    static Network *current_network();

    static Network *initialize();
    static Network *initialize(const std::vector<int> &dims);
    static Network *initialize(const std::string &file_path);

    static void saveNetwork();
    static void flushStorage();

    static void saveBoard(const game::Board *board, const tree::Node *node);
    static void setTestCaseSelector(const std::function<void(game::Board *, double)> &selector);

    static bool SAVE_NETWORKS;

    inline const static std::string NETWORK_DUMP_DIRECTORY = "network_dump/";
    inline const static std::string LATEST_NETWORK_FILE_PATH = NETWORK_DUMP_DIRECTORY + "latest.txt";

  private:
    static void saveLatestNetwork();

    static Network *_current_network;
    static long _network_count;
    static std::function<void(game::Board *, double)> _network_training_case;
};

}

#endif // CHESS_AI_MCTS_NETWORK_NETWORK_H_