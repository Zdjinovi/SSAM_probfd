#ifndef LP_LP_SOLVER_H
#define LP_LP_SOLVER_H

#include "../utils/language.h"
#include "../utils/system.h"
#include "../option_parser.h"

#include <functional>
#include <memory>
#include <vector>

/*
  All methods that use COIN specific classes only do something useful
  if the planner is compiled with USE_LP. Otherwise, they just print
  an error message and abort.
*/
#ifdef USE_LP
#define LP_METHOD(X) X;
#else
#define LP_METHOD(X) NO_RETURN X { \
        ABORT("LP method called but the planner was compiled without LP support.\n" \
              "See http://www.fast-downward.org/LPBuildInstructions\n" \
              "to install an LP solver and use it in the planner."); \
    }
#endif

class CoinPackedVectorBase;
class OsiSolverInterface;

namespace lp
{
enum class LPSolverType {
    CLP, CPLEX, GUROBI, SOPLEX
};

enum class LPObjectiveSense {
    MAXIMIZE, MINIMIZE
};

void add_lp_solver_option_to_parser(options::OptionParser &parser);

class LPConstraint
{
    std::vector<int> variables;
    std::vector<double> coefficients;
    double lower_bound;
    double upper_bound;
public:
    LPConstraint(double lower_bound_, double upper_bound_);

    const std::vector<int> &get_variables() const
    {
        return variables;
    }
    const std::vector<double> &get_coefficients() const
    {
        return coefficients;
    }

    double get_lower_bound() const
    {
        return lower_bound;
    }
    void set_lower_bound(double lb)
    {
        lower_bound = lb;
    }
    double get_upper_bound() const
    {
        return upper_bound;
    }
    void set_upper_bound(double ub)
    {
        upper_bound = ub;
    }

    void clear();
    bool empty() const;
    // Coefficients must be added without duplicate indices.
    void insert(int index, double coefficient);
    void dump() const;
};

struct LPVariable {
    double lower_bound;
    double upper_bound;
    double objective_coefficient;

    LPVariable(double lower_bound_,
               double upper_bound_,
               double objective_coefficient_);
    void dump(unsigned x) const;
};

#ifdef __GNUG__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
class LPSolver
{
    bool is_initialized;
    bool is_solved;
    int num_permanent_constraints;
    int num_variables;
    bool has_temporary_constraints_;
#ifdef USE_LP
    std::unique_ptr<OsiSolverInterface> lp_solver;
#endif

    /*
      Temporary data for assigning a new problem. We keep the vectors
      around to avoid recreating them in every assignment.
    */
    std::vector<double> elements;
    std::vector<int> indices;
    std::vector<int> starts;
    std::vector<double> col_lb;
    std::vector<double> col_ub;
    std::vector<double> objective;
    std::vector<double> row_lb;
    std::vector<double> row_ub;
    std::vector<CoinPackedVectorBase *> rows;
    void clear_temporary_data();
    LPSolverType solver_type;
public:
    LP_METHOD(explicit LPSolver(LPSolverType solver_type))
    /*
      Note that the destructor does not use LP_METHOD because it should not
      have the attribute NO_RETURN. It also cannot be set to the default
      destructor here (~LPSolver() = default;) because OsiSolverInterface
      is a forward declaration and the incomplete type cannot be destroyed.
    */
    ~LPSolver();

    LP_METHOD(LPSolverType get_type() const)
    LP_METHOD(void load_problem(
                  LPObjectiveSense sense,
                  const std::vector<LPVariable> &variables,
                  const std::vector<LPConstraint> &constraints))
    LP_METHOD(void add_temporary_constraints(const std::vector<LPConstraint>
              &constraints))
    LP_METHOD(void clear_temporary_constraints())
    LP_METHOD(double get_infinity() const)

    LP_METHOD(int add_variable(const LPVariable &var,
                                const std::vector<int> &constraints,
                                const std::vector<double> &coefficients))
    LP_METHOD(void add_variable(const LPVariable& var,
                                const std::vector<int>& ids,
                                const std::vector<double>& coefs,
                                const std::string& name))

    LP_METHOD(void add_constraints(const std::vector<LPConstraint> &constraints))
    LP_METHOD(int add_constraint(const LPConstraint &constraint))
    LP_METHOD(void add_constraint(const LPConstraint &constraint, const std::string& name))

    LP_METHOD(void delete_variables(int num, const int* var_ids))
    LP_METHOD(void delete_constraints(int num, const int* constraint_ids))

    LP_METHOD(void set_objective_coefficients(const std::vector<double>
              &coefficients))
    LP_METHOD(void set_objective_coefficient(int index, double coefficient))
    LP_METHOD(void set_constraint_lower_bound(int index, double bound))
    LP_METHOD(void set_constraint_upper_bound(int index, double bound))
    LP_METHOD(void set_constraint_bounds(int index, double lb, double ub))
    LP_METHOD(void set_variable_lower_bound(int index, double bound))
    LP_METHOD(void set_variable_upper_bound(int index, double bound))
    LP_METHOD(void set_variable_upper_bounds(const std::vector<double> &bounds))

    LP_METHOD(void solve())

    /*
      Return true if the solving the LP showed that it is bounded feasible and
      the discovered solution is guaranteed to be optimal. We test for
      optimality explicitly because solving the LP sometimes finds suboptimal
      solutions due to numerical difficulties.
      The LP has to be solved with a call to solve() before calling this method.
    */
    LP_METHOD(bool has_optimal_solution() const)

    /*
      Return the objective value found after solving an LP.
      The LP has to be solved with a call to solve() and has to have an optimal
      solution before calling this method.
    */
    LP_METHOD(double get_objective_value() const)

    /*
      Return the solution found after solving an LP as a vector with one entry
      per variable.
      The LP has to be solved with a call to solve() and has to have an optimal
      solution before calling this method.
    */
    LP_METHOD(std::vector<double> extract_solution() const)
    LP_METHOD(std::vector<double> extract_dual_solution() const)

    LP_METHOD(int get_num_variables() const)
    LP_METHOD(int get_num_constraints() const)
    LP_METHOD(int has_temporary_constraints() const)
    LP_METHOD(void print_statistics() const)

    LP_METHOD(double get_constraint_lower_bound(int index) const)
    LP_METHOD(double get_constraint_upper_bound(int index) const)

    LP_METHOD(void store(const char* path) const)

    LP_METHOD(void get_constraint(int index, std::vector<double>& coefficients) const)
};
#ifdef __GNUG__
#pragma GCC diagnostic pop
#endif
}

#endif
