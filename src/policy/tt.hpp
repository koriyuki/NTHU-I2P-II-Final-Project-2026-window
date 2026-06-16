#pragma once

#include <cstdint>
#include "state.hpp"

enum TTFlag {
    TT_EXACT,
    TT_LOWER,
    TT_UPPER
};

struct TTEntry {
    uint64_t hash = UINT64_MAX;
    int depth = -1;
    int score = 0;
    TTFlag flag = TT_EXACT;
    Move best_move;
};

constexpr int TT_SIZE = 1 << 20;

extern TTEntry TT[TT_SIZE];

inline int tt_index(uint64_t hash){
    return hash & (TT_SIZE - 1);
}

bool tt_probe(
    uint64_t hash,
    int depth,
    int alpha,
    int beta,
    int& score,
    Move& best_move
);

void tt_store(
    uint64_t hash,
    int depth,
    int score,
    TTFlag flag,
    Move best_move
);