/****************************************************************************************[Solver.h]
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
CryptoMiniSat -- Copyright (c) 2009 Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef Solver_h
#define Solver_h

#include <cstdio>

#include "Vec.h"
#include "Heap.h"
#include "Alg.h"
#include "Logger.h"
#include "MersenneTwister.h"
#include "SolverTypes.h"
#include "clause.h"
#include <string.h>

//#define VERBOSE_DEBUG_XOR
//#define VERBOSE_DEBUG

//=================================================================================================
// Solver -- the main class:


class Solver
{
public:

    // Constructor/Destructor:
    //
    Solver();
    ~Solver();

    // Problem specification:
    //
    Var     newVar    (bool polarity = true, bool dvar = true); // Add a new variable with parameters specifying variable mode.
    bool    addClause (vec<Lit>& ps, const uint group, const char* group_name);  // Add a clause to the solver. NOTE! 'ps' may be shrunk by this method!
    bool    addXorClause (vec<Lit>& ps, bool xor_clause_inverted, const uint group, const char* group_name);  // Add a xor-clause to the solver. NOTE! 'ps' may be shrunk by this method!

    // Solving:
    //
    lbool    simplify    ();                        // Removes already satisfied clauses.
    lbool    solve       (const vec<Lit>& assumps); // Search for a model that respects a given set of assumptions.
    lbool    solve       ();                        // Search without assumptions.
    bool    okay         () const;                  // FALSE means solver is in a conflicting state

    // Variable mode:
    //
    void    setPolarity    (Var v, bool b); // Declare which polarity the decision heuristic should use for a variable. Requires mode 'polarity_user'.
    void    setDecisionVar (Var v, bool b); // Declare if a variable should be eligible for selection in the decision heuristic.
    void    setSeed (const uint32_t seed);  // Sets the seed to be the given number
    void    permutateClauses();             // Permutates the clauses using the seed. It updates the seed in mtrand
    void    needRealUnknowns();             // Uses the "real unknowns" set by setRealUnknown
    void    setRealUnknown(const uint var); //sets a variable to be 'real', i.e. to preferentially branch on it during solving (when useRealUnknown it turned on)
    void    setMaxRestarts(const uint num); //sets the maximum number of restarts to given value

    // Read state:
    //
    lbool   value      (const Var& x) const;       // The current value of a variable.
    lbool   value      (const Lit& p) const;       // The current value of a literal.
    lbool   modelValue (const Lit& p) const;       // The value of a literal in the last model. The last call to solve must have been satisfiable.
    int     nAssigns   ()      const;       // The current number of assigned literals.
    int     nClauses   ()      const;       // The current number of original clauses.
    int     nLearnts   ()      const;       // The current number of learnt clauses.
    int     nVars      ()      const;       // The current number of variables.
	
	// proof logging:
    FILE* output = fopen("cryptominisat_proof_log.proof", "wb");
	FILE* cnf_output = fopen("cryptominisat_cnf_log.cnf", "wb");
    int orig_nvars;
    int constraint_pointer=0;
    int og_constraint_counter=0;
    void transform_xor_clause(vec<Lit>& oc, vec<Lit>& transformed);
    void print_clause(FILE* f, vec<Lit>& pst);
	
    // Extra results: (read-only member variable)
    //
    vec<lbool> model;             // If problem is satisfiable, this vector contains the model (if any).
    vec<Lit>   conflict;          // If problem is unsatisfiable (possibly under assumptions),
    // this vector represent the final conflict clause expressed in the assumptions.

    // Mode of operation:
    //
    double    var_decay;          // Inverse of the variable activity decay factor.                                            (default 1 / 0.95)
    double    clause_decay;       // Inverse of the clause activity decay factor.                                              (1 / 0.999)
    double    random_var_freq;    // The frequency with which the decision heuristic tries to choose a random variable.        (default 0.02)
    int       restart_first;      // The initial restart limit.                                                                (default 100)
    double    restart_inc;        // The factor with which the restart limit is multiplied in each restart.                    (default 1.5)
    double    learntsize_factor;  // The intitial limit for learnt clauses is a factor of the original clauses.                (default 1 / 3)
    double    learntsize_inc;     // The limit for learnt clauses is multiplied with this factor each restart.                 (default 1.1)
    bool      expensive_ccmin;    // Controls conflict clause minimization.                                                    (default TRUE)
    int       polarity_mode;      // Controls which polarity the decision heuristic chooses. See enum below for allowed modes. (default polarity_false)
    int       verbosity;          // Verbosity level. 0=silent, 1=some progress report                                         (default 0)
    uint      restrictedPickBranch; // Pick variables to branch on preferentally from the highest [0, restrictedPickBranch]. If set to 0, preferentiality is turned off (i.e. picked randomly between [0, all])
    bool      useRealUnknowns;    // Whether 'real unknown' optimization should be used. If turned on, VarActivity is only bumped for variables for which the real_unknowns[var] == true
    vector<bool> realUnknowns;    // The important variables. This vector stores 'false' at realUnknowns[var] if the var is not a real unknown, and stores a 'true' if it is a real unkown. If var is larger than realUnkowns.size(), then it is not an important variable

    enum { polarity_true = 0, polarity_false = 1, polarity_user = 2, polarity_rnd = 3 };

    // Statistics: (read-only member variable)
    //
    uint64_t starts, decisions, rnd_decisions, propagations, conflicts;
    uint64_t clauses_literals, learnts_literals, max_literals, tot_literals;

    //Logging
    void needStats();              // Prepares the solver to output statistics
    void needProofGraph();         // Prepares the solver to output proof graphs during solving
    void setVariableName(int var, const char* name); // Sets the name of the variable 'var' to 'name'. Useful for statistics and proof logs (i.e. used by 'logger')
    void startClauseAdding();      // Before adding clauses, but after setting up the Solver (need* functions, verbosity), this should be called
    void endFirstSimplify();       // After the clauses are added, and the first simplify() is called, this must be called
    const vec<Clause*>& get_sorted_learnts(); //return the set of learned clauses, sorted according to the logic used in MiniSat to distinguish between 'good' and 'bad' clauses
    const vec<Clause*>& get_learnts() const; //Get all learnt clauses
    const vec<Clause*>& get_unitary_learnts() const; //return the set of unitary learned clauses

protected:
    // Helper structures:
    //
    struct VarOrderLt {
        const vec<double>&  activity;
        bool operator () (Var x, Var y) const {
            return activity[x] > activity[y];
        }
        VarOrderLt(const vec<double>&  act) : activity(act) { }
    };

    friend class VarFilter;
    struct VarFilter {
        const Solver& s;
        VarFilter(const Solver& _s) : s(_s) {}
        bool operator()(Var v) const {
            return s.assigns[v].isUndef() && s.decision_var[v];
        }
    };

    // Solver state:
    //
    bool                ok;               // If FALSE, the constraints are already unsatisfiable. No part of the solver state may be used!
    vec<Clause*>        clauses;          // List of problem clauses.
    vec<XorClause*>     xorclauses;       // List of problem xor-clauses.
    vec<Clause*>        learnts;          // List of learnt clauses.
    vec<Clause*>        unitary_learnts;  // List of learnt clauses.
    double              cla_inc;          // Amount to bump next clause with.
    vec<double>         activity;         // A heuristic measurement of the activity of a variable.
    double              var_inc;          // Amount to bump next variable with.
    vec<vec<Clause*> >  watches;          // 'watches[lit]' is a list of constraints watching 'lit' (will go there if literal becomes true).
    vec<vec<XorClause*> >  xorwatches;    // 'xorwatches[var]' is a list of constraints watching var in XOR clauses.
    vec<lbool>          assigns;          // The current assignments
    vec<char>           polarity;         // The preferred polarity of each variable.
    vec<char>           decision_var;     // Declares if a variable is eligible for selection in the decision heuristic.
    vec<Lit>            trail;            // Assignment stack; stores all assigments made in the order they were made.
    vec<int32_t>        trail_lim;        // Separator indices for different decision levels in 'trail'.
    vec<Clause*>        reason;           // 'reason[var]' is the clause that implied the variables current value, or 'NULL' if none.
    vec<int32_t>        level;            // 'level[var]' contains the level at which the assignment was made.
    int                 qhead;            // Head of queue (as index into the trail -- no more explicit propagation queue in MiniSat).
    int                 simpDB_assigns;   // Number of top-level assignments since last execution of 'simplify()'.
    int64_t             simpDB_props;     // Remaining number of propagations that must be made before next execution of 'simplify()'.
    vec<Lit>            assumptions;      // Current set of assumptions provided to solve by the user.
    Heap<VarOrderLt>    order_heap;       // A priority queue of variables ordered with respect to the variable activity.
    double              progress_estimate;// Set by 'search()'.
    bool                remove_satisfied; // Indicates whether possibly inefficient linear scan for satisfied clauses should be performed in 'simplify'.
    MTRand mtrand;                        // random number generator
    Logger logger;                        // dynamic logging, statistics
    bool dynamic_behaviour_analysis;      //should 'logger' be called whenever a propagation/conflict/decision is made?
    uint                maxRestarts;      // More than this number of restarts will not be performed

    // Temporaries (to reduce allocation overhead). Each variable is prefixed by the method in which it is
    // used, exept 'seen' wich is used in several places.
    //
    vec<char>           seen;
    vec<Lit>            analyze_stack;
    vec<Lit>            analyze_toclear;
    vec<Lit>            add_tmp;

    //Logging
    uint learnt_clause_group; //the group number of learnt clauses. Incremented at each added learnt clause

    // Main internal methods:
    //
    void     insertVarOrder   (Var x);                                                 // Insert a variable in the decision order priority queue.
    Lit      pickBranchLit    (int polarity_mode);                                     // Return the next decision variable.
    void     newDecisionLevel ();                                                      // Begins a new decision level.
    void     uncheckedEnqueue (Lit p, Clause* from = NULL);                            // Enqueue a literal. Assumes value of literal is undefined.
    bool     enqueue          (Lit p, Clause* from = NULL);                            // Test if fact 'p' contradicts current state, enqueue otherwise.
    Clause*  propagate        (const bool xor_as_well = true);                         // Perform unit propagation. Returns possibly conflicting clause.
    Clause*  propagate_xors   (const Lit& p);
    void     cancelUntil      (int level);                                             // Backtrack until a certain level.
    void     analyze          (Clause* confl, vec<Lit>& out_learnt, int& out_btlevel); // (bt = backtrack)
    void     analyzeFinal     (Lit p, vec<Lit>& out_conflict);                         // COULD THIS BE IMPLEMENTED BY THE ORDINARIY "analyze" BY SOME REASONABLE GENERALIZATION?
    bool     litRedundant     (Lit p, uint32_t abstract_levels);                       // (helper method for 'analyze()')
    lbool    search           (int nof_conflicts, int nof_learnts);                    // Search for a given number of conflicts.
    void     reduceDB         ();                                                      // Reduce the set of learnt clauses.
    template<class T>
    void     removeSatisfied  (vec<T*>& cs);                                           // Shrink 'cs' to contain only non-satisfied clauses.
    void     cleanClauses     (vec<XorClause*>& cs);
    void     cleanClauses     (vec<Clause*>& cs);                                      // Remove TRUE or FALSE variables from the xor clauses and remove the FALSE variables from the normal clauses
    llbool   handle_conflict  (vec<Lit>& learnt_clause, Clause* confl, int& conflictC);// Handles the conflict clause
    llbool   new_decision     (int& nof_conflicts, int& nof_learnts, int& conflictC);  // Handles the case when all propagations have been made, and now a decision must be made

    // Maintaining Variable/Clause activity:
    //
    void     varDecayActivity ();                      // Decay all variables with the specified factor. Implemented by increasing the 'bump' value instead.
    void     varBumpActivity  (Var v);                 // Increase a variable with the current 'bump' value.
    void     claDecayActivity ();                      // Decay all clauses with the specified factor. Implemented by increasing the 'bump' value instead.
    void     claBumpActivity  (Clause& c);             // Increase a clause with the current 'bump' value.

    // Operations on clauses:
    //
    void     attachClause     (XorClause& c);
    void     attachClause     (Clause& c);             // Attach a clause to watcher lists.
    void     detachClause     (const XorClause& c);
    void     detachClause     (const Clause& c);       // Detach a clause to watcher lists.
    void     removeClause(XorClause& c);                       // Detach and free a clause.
	void     removeClause(Clause& c); 
    bool     locked           (const Clause& c) const; // Returns TRUE if a clause is a reason for some implication in the current state.
    bool     satisfied        (const XorClause& c) const; // Returns TRUE if the clause is satisfied in the current state
    bool     satisfied        (const Clause& c) const; // Returns TRUE if the clause is satisfied in the current state.

    // Misc:
    //
    int      decisionLevel    ()      const; // Gives the current decisionlevel.
    uint32_t abstractLevel    (const Var& x) const; // Used to represent an abstraction of sets of decision levels.
    double   progressEstimate ()      const; // DELETE THIS ?? IT'S NOT VERY USEFUL ...

    // Debug:
    void     printLit         (const Lit l) const;
    void     printClause      (const Clause& c) const;
    void     printClause      (const XorClause& c) const;
    void     verifyModel      ();
    void     checkLiteralCount();
};


//=================================================================================================
// Implementation of inline methods:


inline void Solver::insertVarOrder(Var x)
{
    if (!order_heap.inHeap(x) && decision_var[x]) order_heap.insert(x);
}

inline void Solver::varDecayActivity()
{
    var_inc *= var_decay;
}
inline void Solver::varBumpActivity(Var v)
{
    if ( (activity[v] += var_inc) > 1e100 ) {
        // Rescale:
        for (int i = 0; i < nVars(); i++)
            activity[i] *= 1e-100;
        var_inc *= 1e-100;
    }

    // Update order_heap with respect to new activity:
    if (order_heap.inHeap(v))
        order_heap.decrease(v);
}

inline void Solver::claDecayActivity()
{
    cla_inc *= clause_decay;
}
inline void Solver::claBumpActivity (Clause& c)
{
    if ( (c.activity() += cla_inc) > 1e20 ) {
        // Rescale:
        for (int i = 0; i < learnts.size(); i++)
            learnts[i]->activity() *= 1e-20;
        cla_inc *= 1e-20;
    }
}

inline bool     Solver::enqueue         (Lit p, Clause* from)
{
    return value(p) != l_Undef ? value(p) != l_False : (uncheckedEnqueue(p, from), true);
}
inline bool     Solver::locked          (const Clause& c) const
{
    return reason[c[0].var()] == &c && value(c[0]) == l_True;
}
inline void     Solver::newDecisionLevel()
{
    trail_lim.push(trail.size());
    #ifdef VERBOSE_DEBUG
    std::cout << "New decision level:" << trail_lim.size() << std::endl;
    #endif
}
inline int      Solver::decisionLevel ()      const
{
    return trail_lim.size();
}
inline uint32_t Solver::abstractLevel (const Var& x) const
{
    return 1 << (level[x] & 31);
}
inline lbool    Solver::value         (const Var& x) const
{
    return assigns[x];
}
inline lbool    Solver::value         (const Lit& p) const
{
    return assigns[p.var()] ^ p.sign();
}
inline lbool    Solver::modelValue    (const Lit& p) const
{
    return model[p.var()] ^ p.sign();
}
inline int      Solver::nAssigns      ()      const
{
    return trail.size();
}
inline int      Solver::nClauses      ()      const
{
    return clauses.size() + xorclauses.size();
}
inline int      Solver::nLearnts      ()      const
{
    return learnts.size();
}
inline int      Solver::nVars         ()      const
{
    return assigns.size();
}
inline void     Solver::setPolarity   (Var v, bool b)
{
    polarity    [v] = (char)b;
}
inline void     Solver::setDecisionVar(Var v, bool b)
{
    decision_var[v] = (char)b;
    if (b) {
        insertVarOrder(v);
    }
}
inline lbool     Solver::solve         ()
{
    vec<Lit> tmp;
    return solve(tmp);
}
inline bool     Solver::okay          ()      const
{
    return ok;
}
inline void     Solver::setSeed (const uint32_t seed)
{
    mtrand.seed(seed);    // Set seed of the variable-selection and clause-permutation(if applicable)
}
inline void     Solver::needStats()
{
    dynamic_behaviour_analysis = true;    // Sets the solver and the logger up to generate statistics
    logger.statistics_on = true;
}
inline void     Solver::needProofGraph()
{
    dynamic_behaviour_analysis = true;    // Sets the solver and the logger up to generate proof graphs during solving
    logger.proof_graph_on = true;
}
inline void     Solver::setVariableName(int var, const char* name)
{
    while (var >= nVars()) newVar();
    logger.set_variable_name(var, name);
} // Sets the varible 'var'-s name to 'name' in the logger
inline void     Solver::startClauseAdding()
{
    if (dynamic_behaviour_analysis) logger.begin();    // Needs to be called before adding any clause
}
inline void     Solver::endFirstSimplify()
{
    if (dynamic_behaviour_analysis) logger.end(Logger::done_adding_clauses);    // Needs to be called before adding any clause
}
inline void     Solver::needRealUnknowns()
{
    useRealUnknowns = true;
}


//=================================================================================================
// Debug + etc:

static inline void logLit(FILE* f, Lit l)
{
    fprintf(f, "%sx%d", l.sign() ? "~" : "", l.var()+1);
}

static inline void logLits(FILE* f, const vec<Lit>& ls)
{
    fprintf(f, "[ ");
    if (ls.size() > 0) {
        logLit(f, ls[0]);
        for (int i = 1; i < ls.size(); i++) {
            fprintf(f, ", ");
            logLit(f, ls[i]);
        }
    }
    fprintf(f, "] ");
}

static inline const char* showBool(bool b)
{
    return b ? "true" : "false";
}


// Just like 'assert()' but expression will be evaluated in the release version as well.
static inline void check(bool expr)
{
    assert(expr);
}

//=================================================================================================
#endif
