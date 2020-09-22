#include "network.h"

#include <algorithm>
#include <string>
#include <fstream>
#include <cstdio>

#include "tree.h"
#include "../util/string_util.h"

// Network class
network::Network::Network() : Network({64, 144, 225, 100, 1}) {}

network::Network::Network(std::vector<int> dims) {
  if (dims.empty()) {
    DEBUG_ASSERT
    dims = {64, 144, 225, 100, 1};
  } else {
    if (dims[0] != 64)
      dims.insert(dims.begin(), 64);
    if (dims.back() != 1)
      dims.push_back(1);

    if (dims.size() <= 2) {
      DEBUG_ASSERT
      dims = {64, 144, 225, 100, 1};
    }
  }

  updateInternals(dims);
}

void network::Network::updateInternals(const std::vector<int> &dims) {
  _num_layers = dims.size();
  _input_index = 0;
  _output_index = _num_layers - 1;

  _dimensions = dims;

  _max_width = 0;

  for (int n = 0; n < _num_layers; ++n) {
    _neurons.emplace_back(dims[n], 0.0);
    _thetas.emplace_back(dims[n], 0.0);
    _omegas.emplace_back(dims[n], 0.0);

    _max_width = std::max(_max_width, dims[n]);
  }

  for (int n = 0; n < _num_layers - 1; ++n) {
    _connections.emplace_back(dims[n], std::vector<double>(dims[n + 1], 0.0));
    _connection_changes.emplace_back(dims[n], std::vector<double>(dims[n + 1], 0.0));
  }
}

network::Network::~Network() {
  _dimensions.clear();

  _neurons.clear();
  _connections.clear();
  _connection_changes.clear();

  _thetas.clear();
  _omegas.clear();
}

double network::Network::predictPosition(game::Board *b) {
  std::vector<std::vector<double>> compact_network = {std::vector<double>(_max_width, 0.0),
                                                      std::vector<double>(_max_width, 0.0)};
  loadBoard(b, compact_network[0]);

  int n, i, j, prev, compact_layer = 1;
  double sum;
  for (n = 1; n < _num_layers; ++n) {
    prev = n - 1;

    // n = 1 <--> c_layer = 1-1 = 0 -> in sync w/ prev layer var
    compact_layer = 1 - compact_layer;

    for (j = 0; j < _dimensions[n]; ++j) {
      sum = 0.0;

      for (i = 0; i < _dimensions[prev]; ++i)
        sum += compact_network[compact_layer][i] * _connections[prev][i][j];

      // 1-compact_layer is in sync w/ n layer var
      compact_network[1 - compact_layer][j] = f(sum);
    }
  }

  return compact_network[_output_index % 2][0];
}

void network::Network::loadBoard(game::Board *b, std::vector<double> &input) {
  std::vector<piece::Piece *> board = b->pieces();
  for (int i = 0; i < _dimensions[0]; i++)
    input[i] = board[i]->code(); // get values of pieces from board
}

void network::Network::propagate_for_training() {
  int n, i, j, prev;
  double sum;
  for (n = 1; n < _num_layers; ++n) {
    prev = n - 1;
    for (j = 0; j < _dimensions[n]; ++j) {
      sum = 0.0;
      for (i = 0; i < _dimensions[prev]; ++i)
        sum += _neurons[prev][i] * _connections[prev][i][j];
      _neurons[n][j] = f(sum);
    }
  }
}

void network::Network::propagate_for_training(std::vector<std::vector<double>> &unbounded) {
  int n, i, j, prev;
  double sum;
  for (n = 1; n < _num_layers; ++n) {
    prev = n - 1;
    for (j = 0; j < _dimensions[n]; ++j) {
      sum = 0.0;
      for (i = 0; i < _dimensions[prev]; ++i)
        sum += _neurons[prev][i] * _connections[prev][i][j];

      unbounded[n][j] = sum;
      _neurons[n][j] = f(sum);
    }
  }
}

network::Network *network::Network::randomNetwork() {
  auto network = new Network();
  randomNetwork(network);
  return network;
}

network::Network *network::Network::randomNetwork(const std::vector<int> &dims) {
  auto network = new Network(dims);
  randomNetwork(network);
  return network;
}

void network::Network::randomNetwork(Network *network) {
  int n, i, j;
  for (n = 0; n < network->_num_layers - 1; ++n)
    for (i = 0; i < network->_dimensions[n]; ++i)
      for (j = 0; j < network->_dimensions[n + 1]; ++j)
        network->_connections[n][i][j] = random(-1.0, 1.0);
}

network::Network *network::Network::uniformNetwork() {
  auto network = new Network();
  uniformNetwork(network);
  return network;
}

network::Network *network::Network::uniformNetwork(const std::vector<int> &dims) {
  auto network = new Network(dims);
  uniformNetwork(network);
  return network;
}

void network::Network::uniformNetwork(Network *network) {
  int n, i, j;
  for (n = 0; n < network->_num_layers - 1; ++n)
    for (i = 0; i < network->_dimensions[n]; ++i)
      for (j = 0; j < network->_dimensions[n + 1]; ++j)
        network->_connections[n][i][j] = 0.0;
}

network::Network *network::Network::clone() {
  auto network = new Network(_dimensions);

  int n, i, j;
  for (n = 0; n < network->_num_layers - 1; ++n)
    for (i = 0; i < network->_dimensions[n]; ++i)
      for (j = 0; j < network->_dimensions[n + 1]; ++j)
        network->_connections[n][i][j] = _connections[n][i][j];

  return network;
}
network::Network *network::Network::clone(Network *network) {
  return network->clone();
}

network::Network *network::Network::loadFromFile(const std::string &file_path) {
  Network *network = nullptr;

  std::ifstream in_stream(file_path);
  if (in_stream.is_open()) {
    in_stream >> network;
    in_stream.close();
  } else DEBUG_ASSERT

  return network;
}

// Optimizer class
void network::Optimizer::optimize(Network *network, game::Board *input, const double TARGET_OUTPUT, double &lambda,
                                  const double MAX_LEARNING_CONSTANT, const double LEARNING_RATE) {
  std::vector<std::vector<double>> &thetas = thetas_array(network);
  std::vector<std::vector<double>> &omegas = omegas_array(network);

  loadBoard(network, input);
  propagate(network, thetas);

  std::pair<int, std::pair<int, int>> network_data = get_other_data(network);
  int network_size = network_data.first;
  int input_index = network_data.second.first;
  int output_index = network_data.second.second;

  std::vector<int> &dimensions = get_dimensions(network);
  std::vector<std::vector<double>> &neurons = get_neurons(network);
  std::vector<std::vector<std::vector<double>>> &weights = get_connections(network);
  std::vector<std::vector<std::vector<double>>> &delta_weights = get_delta_conns(network);

  // Initialize errors
  omegas[output_index][0] = TARGET_OUTPUT - neurons[output_index][0];
  double error = omegas[output_index][0] * omegas[output_index][0] / 2.0;

  // Back-propagation here
  int layer, next_layer, index, next_index;
  double psi;
  for (layer = output_index - 1; layer >= input_index; --layer) {
    next_layer = layer + 1;
    for (next_index = 0; next_index < dimensions[next_layer]; ++next_index) {
      // get psi value
      psi = omegas[next_layer][next_index] * threshold_deriv(thetas[next_layer][next_index]);
      // clear omega
      omegas[next_layer][next_index] = 0.0;

      for (index = 0; index < dimensions[layer]; ++index) {
        omegas[layer][index] += psi * weights[layer][index][next_index];
        delta_weights[layer][index][next_index] = psi * lambda * neurons[layer][index];
        weights[layer][index][next_index] += delta_weights[layer][index][next_index];
      }
    }
  }

  // Get new error
  propagate(network);
  double new_error_diff = TARGET_OUTPUT - neurons[output_index][0];
  double new_error = new_error_diff * new_error_diff / 2.0;

  if (new_error < error) {
    if (lambda < MAX_LEARNING_CONSTANT)
      lambda *= LEARNING_RATE;
  } else {
    lambda /= LEARNING_RATE;
    for (layer = 0; layer < network_size - 1; ++layer)
      for (index = 0; index < dimensions[layer]; ++index)
        for (next_index = 0; next_index < dimensions[layer + 1]; ++next_index)
          weights[layer][index][next_index] -= delta_weights[layer][index][next_index];
  }
}

void network::Optimizer::dropout(Network *network, double dropout_rate) {
  std::vector<int> &dimensions = get_dimensions(network);
  int network_size = dimensions.size();
  std::vector<std::vector<std::vector<double>>> &connections = get_connections(network);

  int layer, index, next_index;
  for (layer = 0; layer < network_size - 1; ++layer)
    for (index = 0; index < dimensions[layer]; ++index)
      for (next_index = 0; next_index < dimensions[layer + 1]; ++next_index)
        if (math::chance(dropout_rate))
          connections[layer][index][next_index] = math::random(-1.0, 1.0);
}

// NetworkStorage Class
network::Network *network::NetworkStorage::_current_network = nullptr;
long network::NetworkStorage::_network_count = 0;
bool network::NetworkStorage::SAVE_NETWORKS = false;
std::function<void(game::Board *, double)> network::NetworkStorage::_network_training_case;

network::Network *network::NetworkStorage::current_network() {
  if (_current_network == nullptr) DEBUG_ASSERT
  return _current_network;
}

// TODO decide whether new network for training is random or uniform 0s
template<class... Args>
inline network::Network *create_network(Args &&... args) {
  return network::Network::randomNetwork(args...);
}

network::Network *network::NetworkStorage::initialize() {
  if (_current_network != nullptr) {
    DEBUG_ASSERT
    delete _current_network;
  }

  _current_network = create_network();
  saveLatestNetwork();

  return _current_network;
}
network::Network *network::NetworkStorage::initialize(const std::vector<int> &dims) {
  if (_current_network != nullptr) {
    DEBUG_ASSERT
    delete _current_network;
  }

  _current_network = create_network(dims);
  saveLatestNetwork();

  return _current_network;
}
network::Network *network::NetworkStorage::initialize(const std::string &file_path) {
  if (_current_network != nullptr) {
    DEBUG_ASSERT
    delete _current_network;
  }

  _current_network = network::Network::loadFromFile(file_path);
  saveLatestNetwork();

  return _current_network;
}

void network::NetworkStorage::saveNetwork() {
  if (SAVE_NETWORKS && _current_network != nullptr) {
    if (_network_count >= 0) {
      std::string past_network = NETWORK_DUMP_DIRECTORY + "gen-" + std::to_string(_network_count++) + ".txt";
      rename(LATEST_NETWORK_FILE_PATH.c_str(), past_network.c_str());
    } else
      _network_count++;

    saveLatestNetwork();
  }
}

void network::NetworkStorage::saveLatestNetwork() {
  if (SAVE_NETWORKS) {
    std::ofstream new_net_file;
    new_net_file.open(LATEST_NETWORK_FILE_PATH);
    new_net_file << _current_network;
    new_net_file.close();
  }
}

void network::NetworkStorage::flushStorage() {
  saveNetwork();
  delete _current_network;
  _current_network = nullptr;
}

void network::NetworkStorage::saveBoard(const game::Board *board, const tree::Node *node) {
  if (_network_training_case) {
    game::Board *board_copy = board->clone();
    _network_training_case(board_copy, node->value());
    delete board_copy;
  }
}
void network::NetworkStorage::setTestCaseSelector(const std::function<void(game::Board *, double)> &selector) {
  _network_training_case = selector;
}

// Network to/from iostream
namespace network {

std::istream &operator>>(std::istream &input, Network *&net) {
  if (net == nullptr)
    net = new Network();

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

std::ostream &operator<<(std::ostream &output, Network *&net) {
  if (net != nullptr) {
    output << net->_num_layers << std::endl;

    int n;
    for (n = 0; n < net->_num_layers - 1; ++n)
      output << net->_dimensions[n] << " ";
    output << net->_dimensions[net->_num_layers - 1] << std::endl;

    int i, j;
    for (n = 0; n < net->_num_layers - 1; ++n)
      for (i = 0; i < net->_dimensions[n]; ++i)
        for (j = 0; j < net->_dimensions[n + 1]; ++j)
          output << string::from_double(net->_connections[n][i][j]) << " ";
  } else DEBUG_ASSERT

  return output;
}

}