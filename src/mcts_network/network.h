#ifndef CHESSAI_MCTS_NETWORK_NETWORK_H_
#define CHESSAI_MCTS_NETWORK_NETWORK_H_

#include "network.fwd.h"

#include <string>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <cmath>

#include "chess/piece.fwd.h"
#include "chess/game.h"

namespace network {

class Neuron {
  friend class NeuronManager;

  public:
    Neuron() = delete;
    Neuron(const Neuron &n) = delete;
    Neuron &operator=(const Neuron &n) = delete;
    // DEFAULT DESTRUCTOR

    inline float value() { return _value; }
    
    inline int row() const { return _row; }
    inline int index() const { return _index; }

  private:
    Neuron(int r, int i);

    const int _row, _index;
    float _value;
};

class NeuronManager {
  public:
    NeuronManager(const NeuronManager &nm) = delete;
    NeuronManager &operator=(const NeuronManager &nm) = delete;

  protected:
    NeuronManager() {}

    inline Neuron* createNeuron(int r, int i) const { return new Neuron(r, i); }
    inline void setValue(Neuron* n, float val) { n -> _value = val; }
};

class Decider {
  public:
    Decider(const Decider &d) = delete;
    Decider &operator=(const Decider &d) = delete;

    virtual std::pair<float, std::map<game::Move, float>> prediction(game::Game* game);
    virtual ~Decider() {} // Do nothing
  
  protected:
    Decider() {}
    inline float random(float a, float b) { return (b - a) * rand() / RAND_MAX + a; } // Random numbers from [a,b)

    virtual float predictPosition(game::Board* b) = 0;
};

class Network : public Decider, NeuronManager, game::BoardController {
  public:
    Network(const Network &d) = delete;
    Network &operator=(const Network &d) = delete;

    Network(); // default: 64-225-100-1 (for now)
    Network(std::vector<int> dims); // dimensions of network - starts off fully connected
    ~Network();

    void randomifyNetwork();
    void uniformNetwork();

    std::string toString();
  
  protected:
    virtual float predictPosition(game::Board* b);

  private:
    void propagate();

    constexpr static float NEURON_THRESHOLD_VALUE = 16777216.0f;
    inline float f(float x) const { return NEURON_THRESHOLD_VALUE * modifiedSigmoid(x / NEURON_THRESHOLD_VALUE); }
    inline float df(float x) const { const float sig = modifiedSigmoid(x / NEURON_THRESHOLD_VALUE); return 1.0f - sig * sig; }

    inline float modifiedSigmoid(float x) const {
      return 2.0f / (1.0f + exp(-2.0f * x)) - 1.0f; // Sigmoid centered at (0,0) ranging from -1 to 1 w/ derivative = 1 @ (0,0)
    }

    std::vector<std::vector<Neuron*>> _neurons;
    std::vector<std::vector<std::vector<float>>> _connections;
};

class Randomizer : public Decider {
  public:
    Randomizer() {}
    Randomizer(const Randomizer &d) = delete;
    Randomizer &operator=(const Randomizer &d) = delete;

    ~Randomizer() {} // Do nothing
  
  protected:
    virtual float predictPosition(game::Board* b);
};

}

#endif // CHESSAI_MCTS_NETWORK_NETWORK_H_