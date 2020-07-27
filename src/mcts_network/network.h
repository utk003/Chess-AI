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

    [[nodiscard]] inline std::string toString() const {
      std::string to_string = std::to_string(_num_layers) + "\n";

      int n;
      std::string line;
      for (n = 0; n < _num_layers; ++n)
        line += " " + std::to_string(_dimensions[n]);
      to_string += line.substr(1) + "\n";

      int i, j;
      for (n = 0; n < _num_layers - 1; ++n) {
        line = "";
        for (i = 0; i < _dimensions[n]; ++i)
          for (j = 0; j < _dimensions[n + 1]; ++j)
            to_string += " " + std::to_string(_connections[n][i][j]);
        to_string += line.substr(1) + "\n";
      }
      return to_string;
    }

    friend std::istream &operator>>(std::istream &input, Network *&net) {
      // Read number of layers in network
      int number_of_layers;
      input >> number_of_layers;
      std::vector<int> network_dimensions = std::vector<int>(number_of_layers);

      // Read network dimensions
      for (int n = 0; n < number_of_layers; ++n)
        input >> network_dimensions[n];

      net->updateInternals(network_dimensions);

      for (int n = 0; n < number_of_layers - 1; ++n)
        for (int i = 0; i < network_dimensions[n]; ++i)
          for (int j = 0; j < network_dimensions[n + 1]; ++j)
            input >> net->_connections[n][i][j];

      return input;
    }
    friend std::ostream &operator<<(std::ostream &output, Network *&net) {
      output << net->toString();
      return output;
    }

    [[nodiscard]] static inline Network *loadNetwork(const std::string &file_path) {
      std::ifstream in_stream(file_path);
      assert(in_stream.is_open());

      auto network = new Network();
      in_stream >> network;

      in_stream.close();

      return network;
    }

  protected:
    double predictPosition(game::Board *b) override;

  private:
    void updateInternals(const std::vector<int> &dimensions);

    void loadBoard(game::Board *b, std::vector<double> &input);
    void propagate_for_training();
    void propagate_for_training(std::vector<std::vector<double>> &unbounded);

    constexpr static double NEURON_THRESHOLD_VALUE = 16777216.0;
    [[nodiscard]] static inline double f(double x) {
      return NEURON_THRESHOLD_VALUE * modifiedSigmoid(x / NEURON_THRESHOLD_VALUE);
    }
    [[nodiscard]] static inline double df(double x) {
      const double sig = modifiedSigmoid(x / NEURON_THRESHOLD_VALUE);
      return 1.0 - sig * sig;
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

    static void storeNetwork();

    static void saveNetworks();
    static void clearNetworks();
    static void flushStorage(bool save_networks);

    static void saveBoard(const game::Board *board);
    static void setTestCaseSelector(const std::function<void(game::Board *)> &selector);

    static int NETWORK_SAVE_INTERVAL;

  private:
    static Network *_current_network;
    static std::vector<Network *> _network_storage;
    static std::function<void(game::Board *)> _network_training_case;
};

}

#endif // CHESS_AI_MCTS_NETWORK_NETWORK_H_