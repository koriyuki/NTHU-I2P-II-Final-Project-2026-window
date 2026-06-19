#include <utility>
#include <iostream>
#include "state.hpp"
#include "minimax.hpp"
#include "tt.hpp"
#include "pvs.hpp"
#include "quiescene.hpp"

/*============================================================
 * PVS — eval_ctx
 *
 * Principal Variation Search. Caller manages memory.
 *============================================================*/
int Quiescence::eval_ctx(
    State *state,
    int depth,
    GameHistory& history,
    int ply,
    SearchContext& ctx,
    const QuiescenceParams& p,
    int alpha,
    int beta
){
    ctx.nodes++;

    if(ply > ctx.seldepth)
        ctx.seldepth = ply;

    if(ctx.stop)
        return 0;

    if(state->legal_actions.empty() &&
       state->game_state == UNKNOWN)
    {
        state->get_legal_actions();
    }

    if(state->game_state == WIN)
        return P_MAX - ply;

    if(state->game_state == DRAW)
        return 0;

    int rep_score;

    if(state->check_repetition(history, rep_score))
        return rep_score;

    uint64_t hash = state->hash();

    Move tt_move;
    int tt_score;

    if(tt_probe(
        hash,
        depth,
        alpha,
        beta,
        tt_score,
        tt_move))
    {
        return tt_score;
    }

    history.push(hash);

    if(depth <= 0){
        //return Quiescence::quiescence_search(state, alpha, beta, history, ctx);
        int score = quiescence_search(state, alpha, beta, history, ctx);
        history.pop(hash);

        return score;
    }

    auto moves = state->legal_actions;

    /*
    std::sort(moves.begin(), moves.end(),
    [&](const Move& a, const Move& b)
    {
        return state->mvv_lva_score(a)
             > state->mvv_lva_score(b);
    });
    */

     std::sort(
        moves.begin(),
        moves.end(),
        [&](const Move& a, const Move& b)
        {
            if(a == tt_move)
                return true;
             if(b == tt_move)
                return false;

            // [ Hackathon TODO 4-0 ]
            // implement move ordering heuristic (e.g. MVV/LVA) to sort moves before searching
            int score_a = state->mvv_lva_score(a);
            int score_b = state->mvv_lva_score(b);
            if(score_a != score_b){
                return score_a > score_b;
            }

             if(a == tt_move)
                return true;
             if(b == tt_move)
                return false;

             return false;
        }
    );

    int alpha_orig = alpha;

    int best_score = M_MAX;
    Move best_move;

    for(int i = 0; i < (int)moves.size(); i++){

        State* child =
            (State*)state->next_state(moves[i]);

        bool same = child->same_player_as_parent();

        int score;

        if(i == 0){

            int raw = eval_ctx(
                child,
                depth - 1,
                history,
                ply + 1,
                ctx,
                p,
                same ? alpha : -beta,
                same ? beta  : -alpha
            );

            score = same ? raw : -raw;
        }
        else{

            int raw = eval_ctx(
                child,
                depth - 1,
                history,
                ply + 1,
                ctx,
                p,
                same ? alpha : -(alpha + 1),
                same ? alpha + 1 : -alpha
            );

            score = same ? raw : -raw;

            if(score > alpha &&
               score < beta)
            {
                raw = eval_ctx(
                    child,
                    depth - 1,
                    history,
                    ply + 1,
                    ctx,
                    p,
                    same ? alpha : -beta,
                    same ? beta  : -alpha
                );

                score = same ? raw : -raw;
            }
        }

        delete child;

        if(score > best_score){
            best_score = score;
            best_move = moves[i];
        }

        if(score > alpha)
            alpha = score;

        if(alpha >= beta)
            break;
    }

    history.pop(hash);

    TTFlag flag;

    if(best_score <= alpha_orig)
        flag = TT_UPPER;
    else if(best_score >= beta)
        flag = TT_LOWER;
    else
        flag = TT_EXACT;

    tt_store(
        hash,
        depth,
        best_score,
        flag,
        best_move
    );

    return best_score;
}


/*============================================================
 * PVS — search
 *
 * Iterate legal moves, call eval_ctx, return SearchResult.
 *============================================================*/
SearchResult Quiescence::search(
    State *state,
    int depth,
    GameHistory& history,
    SearchContext& ctx,
    int alpha,
    int beta
){
    ctx.reset();
    QuiescenceParams p = QuiescenceParams::from_map(ctx.params);
    SearchResult result;
    result.depth = depth;

    if(!state->legal_actions.size()){
        state->get_legal_actions();
    }

    alpha = M_MAX;
    beta = -M_MAX;
    int best_score = M_MAX;
    int move_index = 0;
    int total_moves = (int)state->legal_actions.size();

    for(auto& action : state->legal_actions){
        /* [ Hackathon TODO 4-1 ]
         * search this move like TODO 3, but starting from the root */
        State* next = (State*)state->next_state(action);
        bool same = next->same_player_as_parent();
        int raw;
        if (same){
            raw = eval_ctx(next, depth - 1, history, 1, ctx, p, alpha, beta);
        }else{
            raw = eval_ctx(next, depth - 1, history, 1, ctx, p, -beta, -alpha);
        }
        int score = same ? raw : -raw;
        delete next;
        if(score > best_score){
        // [ Hackathon TODO 4-2 ]
        // keep this move if it is the best so far
            best_score = score;
            result.best_move = action;
            result.score = best_score;
            result.pv = {action};

            if(p.report_partial && ctx.on_root_update){
                ctx.on_root_update({result.best_move, best_score, depth, move_index + 1, total_moves});
            }
        }
        if (best_score > alpha) {
            alpha = best_score;
        }
        if (alpha >= beta) {
            break;
        }
        move_index++;
    }

    // [ Hackathon TODO 4-3 ]
    // update result and return
    result.score = best_score;
    result.nodes = ctx.nodes;
    result.seldepth = ctx.seldepth;
    return result;
} 

int Quiescence::quiescence_search(
    State *state,
    int alpha,
    int beta,
    GameHistory& history,
    SearchContext& ctx
){
    // [ Hackathon TODO 3-1 ]
    // implement quiescence search, which only searches capture moves until a quiet position is reached
    int stand_pat = state->evaluate();

    if(stand_pat >= beta){
        return stand_pat;
    }
    if(stand_pat > alpha){
        alpha = stand_pat;
    }

    auto capture_moves = state->get_capture_moves();
    for(auto& move : capture_moves){
        State* child = (State*)state->next_state(move);
        int score = -Quiescence::quiescence_search(child, -beta, -alpha, history, ctx);
        delete child;

        if(score >= beta){
            return score;
        }
        if(score > alpha){
            alpha = score;
        }
    }
    return alpha;
}

/*============================================================
 * Quiescence — default_params / param_defs
 *============================================================*/
ParamMap Quiescence::default_params(){
    return {
        {"UseKPEval", "true"},
        {"UseEvalMobility", "true"},
        {"ReportPartial", "true"},
    };
}

std::vector<ParamDef> Quiescence::param_defs(){
    return {
        {"UseKPEval", ParamDef::CHECK, "true"},
        {"UseEvalMobility", ParamDef::CHECK, "true"},
        {"ReportPartial", ParamDef::CHECK, "true"},
    };
}
