#ifndef PROBFD_OPEN_LISTS_FIFO_OPEN_LIST_H
#define PROBFD_OPEN_LISTS_FIFO_OPEN_LIST_H

#include "probfd/engine_interfaces/open_list.h"

#include <deque>

namespace probfd {

/// This namespace contains implementations of open lists.
namespace open_lists {

template <typename Action>
class FifoOpenList : public engine_interfaces::OpenList<Action> {
    std::deque<StateID> queue_;

public:
    ~FifoOpenList() override = default;

    void clear() override { queue_.clear(); }

    unsigned size() const override { return queue_.size(); }

    StateID pop() override
    {
        StateID s = queue_.front();
        queue_.pop_front();
        return s;
    }

    void push(const StateID& state_id) override { queue_.push_back(state_id); }
};

} // namespace open_lists
} // namespace probfd

#endif // __FIFO_OPEN_LIST_H__