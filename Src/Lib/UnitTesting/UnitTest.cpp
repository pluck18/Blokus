#include "pch.h"
#include "UnitTest.h"

#include "nlohmann/json.hpp"

namespace unit_testing {

std::vector<std::pair<std::string, std::any>> const& get_data(TestPrintHelperData const& data) {
    return *(data.data);
}

nlohmann::json to_json(std::any const& /*data*/) {
    return {};
}

nlohmann::json to_json(TestPrintHelperData const& data) {
    nlohmann::json json;

    for (auto& pair : get_data(data)) {
        json[pair.first] = to_json(pair.second);
    }

    return json;
}

std::string to_string(TestPrintHelperData const& data) {
    auto json = to_json(data);
    if (json.empty()) {
        using namespace std::string_literals;
        return "{}"s;
    }
    else {
        return json.dump(2);
    }
}

std::ostream& operator<<(std::ostream& stream, TestPrintHelperData const& data) {
    stream << to_string(data);
    return stream;
}

std::string test_print_helper(std::string const& value) {
    return std::format("\"{}\"", value);
}

std::string test_print_helper(std::string_view value) {
    return test_print_helper(std::string(std::move(value)));
}

std::string test_print_helper(char const* const value) {
    return test_print_helper(std::string(value));
}

}
