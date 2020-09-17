#include "decider.h"

#include "../chess/game.h"
#include <iostream>

// Decider class
std::pair<double, std::map<game::Move, double>> decider::Decider::prediction(game::Game *game) {
  game::Board *board = game->board();

  piece::PieceColor current_color = game->getCurrentColor();
  double color_multiplier = current_color.value();

  double currEval = color_multiplier * predictPosition(board);

  std::map<game::Move, double> actionMap;
  std::vector<game::Move> moves = game->possibleMoves(current_color);

  for (auto &move : moves)
    if (move.verify(board)) {
      board->doMove(new game::Move(move), game);
      actionMap[move] = color_multiplier * predictPosition(board);
      board->undoMove(game);
    } else DEBUG_ASSERT

  return {currEval, actionMap};
}

// Randomizer class
double decider::Randomizer::predictPosition(game::Board *b) {
  return random(-1.0, 1.0);
}

// Minimaxer class
double decider::Minimaxer::predictPosition(game::Board *b) {
  return b->score(
    [&](piece::Piece *piece) -> double {
      return piece->color().value() * piece->type().minimaxValue();
    }
  );
}