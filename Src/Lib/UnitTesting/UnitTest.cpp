#include "pch.h"
#include "UnitTest.h"

#include <format>

namespace unit_testing {

//std::string to_string(TestPrintHelperData const& data) {
//    if (data.empty()) {
//        using namespace std::string_literals;
//        return "{}"s;
//    }
//    else {
//        return data.dump(2);
//    }
//}
//
//std::ostream& operator<<(std::ostream& stream, TestPrintHelperData const& data) {
//    stream << to_string(data);
//    return stream;
//}

std::string test_print_helper(std::string const& value) {
    return value;
}

std::string test_print_helper(std::string_view value) {
    return test_print_helper(std::string(std::move(value)));
}

std::string test_print_helper(char const* const value) {
    return test_print_helper(std::string(value));
}

}

int main() {}
