#include "probfd/solvers/mdp_solver.h"

#include "probfd/engines/idual.h"

#include "probfd/engine_interfaces/state_evaluator.h"

#include "probfd/progress_report.h"

#include "lp/lp_solver.h"

#include "option_parser.h"
#include "plugin.h"

namespace probfd {
namespace solvers {

using namespace engine_interfaces;

using IDualEngine = engines::idual::IDual<State, OperatorID>;

class IDualSolver : public MDPSolver {
public:
    explicit IDualSolver(const options::Options& opts)
        : MDPSolver(opts)
        , eval_(opts.get<std::shared_ptr<TaskStateEvaluator>>("eval"))
        , solver_type_(opts.get<lp::LPSolverType>("lpsolver"))
    {
    }

    static void add_options_to_parser(options::OptionParser& parser)
    {
        parser.add_option<std::shared_ptr<TaskStateEvaluator>>(
            "eval",
            "",
            "const");
        lp::add_lp_solver_option_to_parser(parser);
        MDPSolver::add_options_to_parser(parser);
    }

    virtual std::string get_engine_name() const override { return "idual"; }

    virtual engines::MDPEngineInterface<State>* create_engine() override
    {
        return engine_factory<IDualEngine>(
            solver_type_,
            eval_.get(),
            &progress_);
    }

private:
    std::shared_ptr<TaskStateEvaluator> eval_;
    lp::LPSolverType solver_type_;
};

static Plugin<SolverInterface>
    _plugin("idual", options::parse<SolverInterface, IDualSolver>);

} // namespace solvers
} // namespace probfd
