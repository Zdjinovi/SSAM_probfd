#ifndef PROBFD_OPEN_LISTS_LIFO_OPEN_LIST_H
#define PROBFD_OPEN_LISTS_LIFO_OPEN_LIST_H

#include "probfd/engine_interfaces/open_list.h"

#include <deque>

namespace probfd {
namespace open_lists {

template <typename Action>
class LifoOpenList : public engine_interfaces::OpenList<Action> {
    std::deque<StateID> queue_;

public:
    ~LifoOpenList() override = default;

    bool empty() const override { return queue_.empty(); }

    unsigned size() const override { return queue_.size(); }

    StateID pop() override
    {
        StateID s = queue_.back();
        queue_.pop_back();
        return s;
    }

    void push(const StateID& state_id) override { queue_.push_back(state_id); }

    void clear() override { queue_.clear(); }
};

} // namespace open_lists
} // namespace probfd

#endif // __LIFO_OPEN_LIST_H__