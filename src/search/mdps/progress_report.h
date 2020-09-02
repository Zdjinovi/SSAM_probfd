#pragma once

#include "value_type.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace probabilistic {

class ProgressReport {
public:
    ProgressReport(
        const value_type::value_t min_change,
        std::ostream& out = std::cout,
        const bool enabled = true);
    ~ProgressReport() = default;

    void operator()();

    void enable();
    void disable();

    void register_print(std::function<void(std::ostream&)> f);

    void register_value(
        const std::string& val_name,
        std::function<value_type::value_t()> getter);

private:
    const value_type::value_t min_change_;
    bool enabled_;
    std::ostream* out_;

    std::vector<std::string> value_names_;
    std::vector<std::function<value_type::value_t()>> value_getters_;
    std::vector<value_type::value_t> last_printed_values_;
    std::vector<value_type::value_t> extracted_values_;
    std::vector<std::function<void(std::ostream&)>> additional_informations_;
};

} // namespace probabilistic
