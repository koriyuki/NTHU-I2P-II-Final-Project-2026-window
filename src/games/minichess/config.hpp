#pragma once
#include "../../config.hpp"

/* === MiniChess Board === */
#ifndef BOARD_H
#define BOARD_H 6
#endif
#ifndef BOARD_W
#define BOARD_W 5
#endif

#define MAX_STEP 100
#define USE_BITBOARD

/* MVV-LVA piece values for move ordering (indexed by piece type) */
static const int PIECE_VALUES[] = {
    0,   /* EMPTY=0 */
    100,  /* PAWN=1 */
    500,  /* ROOK=2 */
    300,  /* KNIGHT=3 */
    300,  /* BISHOP=4 */
    900,  /* QUEEN=5 */
    10000, /* KING=6 */
};

/* === Piece display === */
#define PIECE_STR_LEN 2
const char PIECE_TABLE[2][7][5] = {
    {"  ", "wP", "wR", "wn", "wB", "wQ", "wK"},
    {"  ", "bP", "bR", "bn", "bB", "bQ", "bK"},
};
