#include "decider.h"

#include "../chess/game.h"
#include <iostream>

// Decider class
std::pair<double, std::map<game::Move, double>> decider::Decider::prediction(game::Game *game) {
  game::Board *board = game->board()->clone(); // TODO see if needs clone() to protect against messed up undoMove()?

  piece::PieceColor current_color = game->getCurrentColor();
  int color_multiplier = current_color.colorCode();

  double currEval = color_multiplier * predictPosition(board);

  std::map<game::Move, double> actionMap;
  std::vector<game::Move> moves = game->possibleMoves(current_color);

  for (int i = 0; i < moves.size(); ++i) {
    if (moves[i].verify(board)) {
      board->doMove(new game::Move(moves[i]), game);
      actionMap[moves[i]] = color_multiplier * predictPosition(board);
      board->undoMove(game);
    } else {
      std::cout << "Error on Move #" << i << std::endl;
      for (int j = 0; j < moves.size(); ++j)
        std::cout << j << " " << moves[j].toString() << std::endl;
      game->board()->saveToFile("decider_original");
      board->saveToFile("decider_corrupted");
      debug_assert();
    }
  }

  delete board; // TODO paired with todo above ^^^ -> remove iff clone() is removed from above

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
      return piece->color().colorCode() * piece->type().minimaxValue();
    }
  );
}