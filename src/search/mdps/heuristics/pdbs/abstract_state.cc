#include "abstract_state.h"

#include "../../../global_state.h"
#include "../../../globals.h"

#include <cassert>
#include <limits>
#include <sstream>

namespace probabilistic {
namespace pdbs {

AbstractStateMapper::AbstractStateMapper(
    const std::vector<int>& variables,
    const std::vector<int>& domains)
    : oob_(false)
    , size_(-1)
    , vars_(variables)
    , domains_(variables.size())
    , multipliers_(variables.size(), 1)
{
    assert(!variables.empty());
    const int maxint = std::numeric_limits<int>::max();
    for (unsigned i = 1; i < variables.size(); i++) {
        assert(variables[i - 1] < static_cast<int>(domains.size()));
        const int d = domains[variables[i - 1]];
        domains_[i - 1] = d;
        if (multipliers_[i - 1] > maxint / d) {
            oob_ = true;
            return;
        }
        multipliers_[i] = multipliers_[i - 1] * d;
    }
    assert(variables.back() < static_cast<int>(domains.size()));
    const int d = domains[variables.back()];
    domains_.back() = d;
    if (multipliers_.back() > maxint / d) {
        oob_ = true;
        return;
    }
    size_ = multipliers_.back() * d;
}

unsigned
AbstractStateMapper::size() const
{
    return size_;
}

bool
AbstractStateMapper::is_size_within_limits() const
{
    return !oob_;
}

const std::vector<int>&
AbstractStateMapper::get_variables() const
{
    return vars_;
}

const std::vector<int>&
AbstractStateMapper::get_domains() const
{
    return domains_;
}

AbstractState
AbstractStateMapper::operator()(const GlobalState& state) const
{
    AbstractState res(0);
    for (int i = vars_.size() - 1; i >= 0; i--) {
        res.id += multipliers_[i] * state[vars_[i]];
    }
    return res;
}

AbstractState
AbstractStateMapper::operator()(const std::vector<int>& state) const
{
    AbstractState res(0);
    for (int i = vars_.size() - 1; i >= 0; i--) {
        res.id += multipliers_[i] * state[vars_[i]];
    }
    return res;
}

AbstractState
AbstractStateMapper::from_values(const std::vector<int>& values) const
{
    assert(values.size() == vars_.size());
    AbstractState res(0);
    for (int i = vars_.size() - 1; i >= 0; i--) {
        res.id += multipliers_[i] * values[i];
    }
    return res;
}

AbstractState
AbstractStateMapper::from_values_partial(
    const std::vector<int>& indices,
    const std::vector<int>& values) const
{
    assert(values.size() == vars_.size());
    AbstractState res(0);
    for (int i = indices.size() - 1; i >= 0; i--) {
        const int j = indices[i];
        res.id += multipliers_[j] * values[j];
    }
    return res;
}

AbstractState
AbstractStateMapper::from_value_partial(int idx, int val) const
{
    return AbstractState(multipliers_[idx] * val);
}

std::vector<int>
AbstractStateMapper::to_values(AbstractState abstract_state) const
{
    std::vector<int> values(vars_.size(), -1);
    for (int i = vars_.size() - 1; i >= 0; i--) {
        values[i] = ((int)(abstract_state.id / multipliers_[i])) % domains_[i];
    }
    return values;
}

void
AbstractStateMapper::to_values(
    AbstractState abstract_state,
    std::vector<int>& values) const
{
    values.resize(vars_.size());
    for (int i = vars_.size() - 1; i >= 0; i--) {
        values[i] = ((int)(abstract_state.id / multipliers_[i])) % domains_[i];
    }
}

AbstractStateToString::AbstractStateToString(
    std::shared_ptr<AbstractStateMapper> state_mapper)
    : state_mapper_(state_mapper)
{
}

std::string
AbstractStateToString::operator()(AbstractState state) const
{
    std::ostringstream out;
    std::vector<int> values = state_mapper_->to_values(state);
    out << "#" << state.id << ": ";
    for (unsigned i = 0; i < values.size(); i++) {
        const int var = state_mapper_->get_variables()[i];
        out << (i > 0 ? ", " : "") << ::g_fact_names[var][values[i]];
    }
    return out.str();
}

std::ostream&
operator<<(std::ostream& out, const AbstractState& s)
{
    out << "PDBState(" << s.id << ")";
    return out;
}
} // namespace pdbs
} // namespace probabilistic

namespace utils {
void
feed(HashState& h, const probabilistic::pdbs::AbstractState& s)
{
    feed(h, s.id);
}
} // namespace utils
