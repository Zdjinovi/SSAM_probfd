#ifndef TASK_UTILS_SUCCESSOR_GENERATOR_H
#define TASK_UTILS_SUCCESSOR_GENERATOR_H

#include "per_task_information.h"

#include <memory>
#include <vector>

class OperatorID;
class State;
class TaskBaseProxy;

namespace successor_generator {
class GeneratorBase;

class SuccessorGenerator {
    std::unique_ptr<GeneratorBase> root;

public:
    explicit SuccessorGenerator(const TaskBaseProxy& task_proxy);
    /*
      We cannot use the default destructor (implicitly or explicitly)
      here because GeneratorBase is a forward declaration and the
      incomplete type cannot be destroyed.
    */
    ~SuccessorGenerator();

    void generate_applicable_ops(
        const State& state,
        std::vector<OperatorID>& applicable_ops) const;
};

extern PerTaskInformation<SuccessorGenerator> g_successor_generators;
} // namespace successor_generator

#endif
