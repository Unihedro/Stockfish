/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOVEPICK_H_INCLUDED
#define MOVEPICK_H_INCLUDED

#include <algorithm> // For std::max
#include <cstring>   // For std::memset

#include "movegen.h"
#include "position.h"
#include "search.h"
#include "types.h"


/// The Stats struct stores moves statistics. According to the template parameter
/// the class can store History and Countermoves. History records how often
/// different moves have been successful or unsuccessful during the current search
/// and is used for reduction and move ordering decisions. 
/// Countermoves store the move that refute a previous one. Entries are stored
/// using only the moving piece and destination square, hence two moves with
/// different origin but same destination and piece will be considered identical.
template<typename T>
struct Stats {

  static const Value Max = Value(250);

  const T* operator[](Piece pc) const { return table[pc]; }
  T* operator[](Piece pc) { return table[pc]; }
  void clear() { std::memset(table, 0, sizeof(table)); }

  void update(Piece pc, Square to, Move m) {

    if (m != table[pc][to])
        table[pc][to] = m;
  }

  void update(Piece pc, Square to, Value v) {

    if (abs(table[pc][to] + v) < Max)
        table[pc][to] +=  v;
  }

private:
  T table[PIECE_NB][SQUARE_NB];
};

typedef Stats<Value> HistoryStats;
typedef Stats<Move> MovesStats;
typedef Stats<HistoryStats> CounterMovesHistoryStats;


/// MovePicker class is used to pick one pseudo legal move at a time from the
/// current position. The most important method is next_move(), which returns a
/// new pseudo legal move each time it is called, until there are no moves left,
/// when MOVE_NONE is returned. In order to improve the efficiency of the alpha
/// beta algorithm, MovePicker attempts to return the moves which are most likely
/// to get a cut-off first.

class MovePicker {
public:
  MovePicker(const MovePicker&) = delete;
  MovePicker& operator=(const MovePicker&) = delete;

  MovePicker(const Position&, Move, Depth, const HistoryStats&, const CounterMovesHistoryStats&, Square);
  MovePicker(const Position&, Move, const HistoryStats&, const CounterMovesHistoryStats&, PieceType);
  MovePicker(const Position&, Move, Depth, const HistoryStats&, const CounterMovesHistoryStats&, Move, Search::Stack*);

  template<bool SpNode> Move next_move();

private:
  template<GenType> void score();
  void generate_next_stage();
  ExtMove* begin() { return moves; }
  ExtMove* end() { return endMoves; }

  const Position& pos;
  const HistoryStats& history;
  const CounterMovesHistoryStats& counterMovesHistory;
  Search::Stack* ss;
  Move countermove;
  Depth depth;
  Move ttMove;
  ExtMove killers[3];
  Square recaptureSquare;
  Value captureThreshold;
  int stage;
  ExtMove *endQuiets, *endBadCaptures;
  ExtMove moves[MAX_MOVES], *cur = moves, *endMoves = moves;
};

#endif // #ifndef MOVEPICK_H_INCLUDED
