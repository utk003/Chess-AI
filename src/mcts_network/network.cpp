#include "network.h"

#include <cstdlib>

// Neuron class
network::Neuron::Neuron(int r, int i) : _row(r), _index(i) {
  _value = 0.0f;
}

// Decider class
std::pair<float, std::map<game::Move, float>> network::Decider::prediction(game::Game* game) {
  game::Board* board = game -> board();

  piece::PieceColor current_color = game -> getCurrentColor();
  int color_multiplier = current_color.colorCode();

  float currEval = color_multiplier * (this -> predictPosition(board));

  std::map<game::Move, float> actionMap;
  std::vector<game::Move> moves = game -> possibleMoves(current_color);

  int i;
  for (i = 0; i < moves.size(); ++i) {
    board -> doMove(new game::Move(moves[i]), game);
    actionMap[moves[i]] = 0.0f; //color_multiplier * (this -> predictPosition(board));
    board -> undoMove(game);
  }

  return {currEval, actionMap};
}

// Network class
network::Network::Network() : Network({64, 225, 100, 1}) {}

network::Network::Network(std::vector<int> dims) {
  assert(dims.size() > 0);

  if (dims.front() != 64)
    dims.insert(dims.begin(), 64);
  if (dims.back() != 1)
    dims.push_back(1);

  assert(dims.size() > 2);

  int n, i, j;
  for (n = 0; n < dims.size(); ++n) {
    std::vector<Neuron*> neuronRow;

    for (i = 0; i < dims[n]; ++i)
      neuronRow.push_back(createNeuron(n, i));
    
    _neurons.push_back(neuronRow);
  }

  for (n = 0; n < dims.size() - 1; ++n) {
    std::vector<std::vector<float>> conns;

    for (i = 0; i < dims[n]; ++i) {
      std::vector<float> connVals;

      for (j = 0; j < dims[n+1]; ++j)
        connVals.push_back(0.0f);
      
      conns.push_back(connVals);
    }

    _connections.push_back(conns);
  }
}

network::Network::~Network() {
  Neuron* neuron;
  int i, j;
  for (i = 0; i < _neurons.size(); ++i) {
    for (j = 0; j < _neurons[i].size(); ++j) {
      neuron = _neurons[i][j];
      _neurons[i][j] = nullptr;
      delete neuron;
    }
    _neurons[i].clear();
  }
  _neurons.clear();

  for (i = 0; i < _connections.size(); ++i) {
    for (j = 0; j < _connections[i].size(); ++j)
      _connections[i][j].clear();
    _connections[i].clear();
  }
}

float network::Network::predictPosition(game::Board* b) {
  std::vector<piece::Piece*> board = getBoard(b);
  
  int i;
  for (i = 0; i < 64; i++)
    setValue(_neurons[0][i], board[i] -> code()); // get values of pieces from board
  
  propagate();

  return _neurons.back()[0] -> value();
}

void network::Network::propagate() {
  int n, i, j, prev;
  float sum;
  for (n = 1; n < _neurons.size(); ++n) {
    prev = n - 1;
    for (j = 0; j < _neurons[n].size(); ++j) {
      sum = 0.0f;
      for (i = 0; i < _neurons[prev].size(); ++i)
        sum += _neurons[prev][i] -> value() * _connections[prev][i][j];
      setValue(_neurons[n][j], f(sum));
    }
  }  
}

void network::Network::randomifyNetwork() {
  int n, i, j;
  for (n = 0; n < _connections.size(); ++n)
    for (i = 0; i < _connections[n].size(); ++i)
      for (j = 0; j < _connections[n][i].size(); ++j)
        _connections[n][i][j] = random(-1.0f, 1.0f);
}

void network::Network::uniformNetwork() {
  int n, i, j;
  for (n = 0; n < _connections.size(); ++n)
    for (i = 0; i < _connections[n].size(); ++i)
      for (j = 0; j < _connections[n][i].size(); ++j)
        _connections[n][i][j] = 0.0f;
}

std::string network::Network::toString() {
  std::string _toString = "";
  int n, i, j;
  for (n = 0; n < _connections.size(); ++n) {
    for (i = 0; i < _connections[n].size(); ++i)
      for (j = 0; j < _connections[n][i].size(); ++j)
        _toString += std::to_string(_connections[n][i][j]) + " ";
    _toString += "\n";
  }
  return _toString;
}

// Randomizer class
float network::Randomizer::predictPosition(game::Board* b) {
  return random(-1.0f, 1.0f);
}