// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <utility>
#include <vector>

#include "boost/ut.hpp"
#include "fmt/format.h"
#include "gsl/gsl"
#include "nlohmann/json.hpp"
#include "magic_enum.hpp"
#include "range/v3/all.hpp"

#include <windows.h>


std::string narrow_string(std::wstring const& wide) {

    auto const slength = gsl::narrow_cast<int>(wide.length() + 1);
    auto const len = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), slength, 0, 0, 0, 0);
    std::string narrow(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, wide.c_str(), slength, &narrow[0], len, 0, 0);
    return narrow;
}

std::wstring widen_string(std::string const& narrow) {

    auto const slength = gsl::narrow_cast<int>(narrow.length() + 1);
    auto const len = MultiByteToWideChar(CP_ACP, 0, narrow.c_str(), slength, 0, 0);
    std::wstring wide(len, '\0');
    MultiByteToWideChar(CP_ACP, 0, narrow.c_str(), slength, &wide[0], len);
    return wide;
}

// ----------------------------------------------------------------------------

class TestPrintHelperData {
public:
    TestPrintHelperData() = default;

    TestPrintHelperData(nlohmann::json&& data) :
        data(std::move(data)) {}

    auto str() const {
        if (data.empty()) {
            using namespace std::string_literals;
            return "{}"s;
        }
        else {
            return data.dump(2);
        }
    }

private:
    nlohmann::json data;
};



template<typename T>
concept test_print_helper_exists = requires(std::remove_cvref_t<T> const& v) {
    { test_print_helper(v) } -> std::same_as<TestPrintHelperData>;
};



struct S {};

TestPrintHelperData test_print_helper(S const& /*value*/) {
    return {};
}

// ----------------------------------------------------------------------------

namespace cfg {

struct printer : boost::ut::printer {
    template <class T>
        //requires test_print_helper_exists<T>
    auto& operator<<(T const& t) {
        if constexpr (test_print_helper_exists<T>) {
            auto const data = test_print_helper(t);
            auto const str = data.str();
            boost::ut::printer::operator<<(str);
        }
        else {
            boost::ut::printer::operator<<(t);
        }
        return *this;
    }
};

}  // namespace cfg

template <>
auto boost::ut::cfg<boost::ut::override> = boost::ut::runner<boost::ut::reporter<cfg::printer>>{};

// ----------------------------------------------------------------------------

const boost::ut::suite rules_suite = [] {

using namespace boost::ut;
using namespace boost::ut::bdd;

"PrintHelper"_test = [] {

    given("Given an empty structure") = []{
        S s;

        when("When retrieving printer output") = [&s] {
            cfg::printer printer;
            printer << s;
            auto const result = printer.str();

            then("Then the ouput is equivalent to an empty json") = [&result] {
                auto const reference = "{}";

                expect(that % result == reference);
            };
        };
    };
};

};

// ----------------------------------------------------------------------------

int main() {}
