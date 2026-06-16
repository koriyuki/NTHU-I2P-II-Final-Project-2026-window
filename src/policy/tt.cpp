#include "tt.hpp"

TTEntry TT[TT_SIZE];

bool tt_probe(
    uint64_t hash,
    int depth,
    int alpha,
    int beta,
    int& score,
    Move& best_move
){
    TTEntry& e = TT[tt_index(hash)];

    if(e.hash != hash)
        return false;

    if(e.depth < depth)
        return false;

    score = e.score;
    best_move = e.best_move;

    switch(e.flag){

    case TT_EXACT:
        return true;

    case TT_LOWER:
        if(score >= beta)
            return true;
        break;

    case TT_UPPER:
        if(score <= alpha)
            return true;
        break;
    }

    return false;
}

void tt_store(
    uint64_t hash,
    int depth,
    int score,
    TTFlag flag,
    Move best_move
){
    TTEntry& e = TT[tt_index(hash)];

    if(depth >= e.depth){
        e.hash = hash;
        e.depth = depth;
        e.score = score;
        e.flag = flag;
        e.best_move = best_move;
    }
}