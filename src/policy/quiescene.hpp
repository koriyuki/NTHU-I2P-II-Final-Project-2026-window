#pragma once
#include "search_types.hpp"
#include "game_history.hpp"

struct QuiescenceParams {
    bool use_kp_eval = true;
    bool use_eval_mobility = true;
    bool report_partial = true;

    static QuiescenceParams from_map(const ParamMap& m){
        QuiescenceParams p;
        p.use_kp_eval       = param_bool(m, "UseKPEval", true);
        p.use_eval_mobility = param_bool(m, "UseEvalMobility", true);
        p.report_partial    = param_bool(m, "ReportPartial", true);
        return p;
    }
};

class Quiescence{
public:
    static int eval_ctx(
        State *state,
        int depth,
        GameHistory& history,
        int ply,
        SearchContext& ctx,
        const QuiescenceParams& p,
        int alpha,
        int beta
    );
    static SearchResult search(
        State *state,
        int depth,
        GameHistory& history,
        SearchContext& ctx,
        int alpha,
        int beta
    );
    static int quiescence_search(
        State *state,
        int alpha,
        int beta,
        GameHistory& history,
        SearchContext& ctx
    );
    static ParamMap default_params();
    static std::vector<ParamDef> param_defs();
    
};
