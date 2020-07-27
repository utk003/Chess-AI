#include "network.h"

#include <algorithm>
#include <string>
#include <fstream>

// Network class
network::Network::Network() : Network({64, 144, 225, 100, 1}) {}

network::Network::Network(std::vector<int> dims) {
  assert(!dims.empty());

  if (dims.front() != 64)
    dims.insert(dims.begin(), 64);
  if (dims.back() != 1)
    dims.push_back(1);

  assert(dims.size() > 2);

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
        network->_connections[n][i][j] = random(-1.0f, 1.0f);
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
        network->_connections[n][i][j] = 0.0f;
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
  double error = omegas[output_index][0] * omegas[output_index][0] / 2.0f;

  // Back-propagation here
  int layer, next_layer, index, next_index;
  double psi;
  for (layer = output_index - 1; layer >= input_index; --layer) {
    next_layer = layer + 1;
    for (next_index = 0; next_index < dimensions[next_layer]; ++next_index) {
      // get psi value
      psi = omegas[next_layer][next_index] * threshold_deriv(thetas[next_layer][next_index]);
      // clear omega
      omegas[next_layer][next_index] = 0.0f;

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
  double new_error = new_error_diff * new_error_diff / 2.0f;

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

// NetworkStorage Class
int network::NetworkStorage::NETWORK_SAVE_INTERVAL = 0;

network::Network *network::NetworkStorage::_current_network = nullptr;
std::vector<network::Network *> network::NetworkStorage::_network_storage;
std::function<void(game::Board *)> network::NetworkStorage::_network_training_case;

network::Network *network::NetworkStorage::current_network() {
  if (_current_network == nullptr)
    initialize();
  return _current_network;
}

network::Network *network::NetworkStorage::initialize() {
  delete _current_network;
  _current_network = new Network();
  return _current_network;
}
network::Network *network::NetworkStorage::initialize(const std::vector<int> &dims) {
  delete _current_network;
  _current_network = new Network(dims);
  return _current_network;
}
network::Network *network::NetworkStorage::initialize(const std::string &file_path) {
  delete _current_network;
  _current_network = network::Network::loadNetwork(file_path);
  return _current_network;
}

void network::NetworkStorage::storeNetwork() {
  if (_current_network != nullptr) {
    _network_storage.push_back(_current_network);
    _current_network = _current_network->clone();
  } else
    _current_network = new Network();
}

void network::NetworkStorage::saveNetworks() {
  for (int i = 0; i < _network_storage.size(); ++i) {
    // TODO finalize network dump system
    std::ofstream myfile;
    myfile.open("../network_dump/network " + std::to_string(i * NETWORK_SAVE_INTERVAL) + ".txt");
    myfile << _network_storage[i];
    myfile.close();
  }
}

void network::NetworkStorage::clearNetworks() {
  for (auto &i : _network_storage)
    delete i;
  _network_storage.clear();
}

void network::NetworkStorage::flushStorage(bool save_networks) {
  if (_current_network != nullptr) {
    _network_storage.push_back(_current_network);
    _current_network = nullptr;
  }

  if (save_networks)
    saveNetworks();

  clearNetworks();
}

void network::NetworkStorage::saveBoard(const game::Board *board) {
  if (_network_training_case) {
    game::Board *copy = board->clone();
    _network_training_case(copy);
    delete copy;
  }
}
void network::NetworkStorage::setTestCaseSelector(const std::function<void(game::Board *)> &selector) {
  _network_training_case = selector;
}