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

    auto& json() const {
        return data;
    }

    friend static void to_json(nlohmann::json& j, TestPrintHelperData const& data);

    friend auto operator<=>(TestPrintHelperData const&, TestPrintHelperData const&) = default;
};

static void to_json(nlohmann::json& j, TestPrintHelperData const& data) {
    j = data.json();
}


struct S {
    friend auto operator<=>(S const&, S const&) = default;
};

auto test_print_helper(S const& /*value*/) {
    return TestPrintHelperData{};
}

struct Si {
    int i;

    friend auto operator<=>(Si const&, Si const&) = default;
};

auto test_print_helper(Si const& value) {
    using namespace std::string_literals;
    return TestPrintHelperData{ { { "integer"s, value.i } } };
}

struct Sc {
    Si i;
    double d;

    friend auto operator<=>(Sc const&, Sc const&) = default;
};

auto test_print_helper(Sc const& value) {
    using namespace std::string_literals;
    return TestPrintHelperData{ {
        { "i"s, test_print_helper(value.i) },
        { "d"s, value.d },
        } };
}

// ----------------------------------------------------------------------------

auto test_print_helper(std::string const& value) {
    using namespace std::string_literals;
    return "\""s + value + "\""s;
}

auto test_print_helper(std::string_view value) {
    return test_print_helper(std::string(std::move(value)));
}

auto test_print_helper(char const * const value) {
    return test_print_helper(std::string(value));
}

// ----------------------------------------------------------------------------

namespace cfg {

struct Printer {
public:
    template<class T>
    struct Value {
        Value(T const& value) :
            value(value) {
        }

        auto get_value() const {
            return test_print_helper(value);
        }

    private:
        T value;
    };

    auto str() const { return printer.str(); }
    const auto& colors() const { return printer.colors(); }

    template <class T>
    auto& operator<<(T const& t) {
        printer << boost::ut::detail::get(t);
        return *this;
    }

    template <class T>
    auto& operator<<(Value<T> const& t) {
        *this << t.get_value();
        return *this;
    }

    auto& operator<<(TestPrintHelperData const& t) {
        *this << t.str();
        return *this;
    }

    auto& operator<<(std::string_view sv) {
        printer << sv;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::eq_<TLhs, TRhs> const& op) {
        boost::ut::detail::eq_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs, class TEpsilon>
    auto& operator<<(boost::ut::detail::approx_<TLhs, TRhs, TEpsilon> const& op) {
        boost::ut::detail::approx_ value_op(Value(op.lhs()), Value(op.rhs()), Value(op.epsilon()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::neq_<TLhs, TRhs> const& op) {
        boost::ut::detail::neq_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::gt_<TLhs, TRhs> const& op) {
        boost::ut::detail::gt_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::ge_<TLhs, TRhs> const& op) {
        boost::ut::detail::ge_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::lt_<TRhs, TLhs> const& op) {
        boost::ut::detail::lt_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::le_<TRhs, TLhs> const& op) {
        boost::ut::detail::le_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::and_<TLhs, TRhs> const& op) {
        boost::ut::detail::and_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs>
    auto& operator<<(boost::ut::detail::or_<TLhs, TRhs> const& op) {
        boost::ut::detail::or_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class T>
    auto& operator<<(boost::ut::detail::not_<T> const& op) {
        boost::ut::detail::not_ value_op(Value(op.value()));
        printer << value_op;
        return *this;
    }

    template <class T>
    auto& operator<<(boost::ut::detail::fatal_<T>& fatal) {
        boost::ut::detail::fatal_ value_fatal(Value(fatal.get()));
        printer << value_fatal;
        return *this;
    }

private:
    boost::ut::printer printer;
};

//template<class TLhs, class TRhs>
//auto operator<=>(Printer::Value<TLhs> const& lhs, Printer::Value<TRhs> const& rhs) {
//    return lhs.get_value() <=> rhs.get_value();
//}

template<class TLhs, class TRhs>
auto operator==(Printer::Value<TLhs> const& lhs, Printer::Value<TRhs> const& rhs) {
    return lhs.get_value() == rhs.get_value();
}

template<class TLhs, class TRhs>
auto operator<(Printer::Value<TLhs> const& lhs, Printer::Value<TRhs> const& rhs) {
    return lhs.get_value() < rhs.get_value();
}

template<class T>
auto& operator<<(std::ostream& stream, Printer::Value<T> const& value) {
    stream << value.get_value();
    return stream;
}

}  // namespace cfg

template <>
auto boost::ut::cfg<boost::ut::override> = boost::ut::runner<boost::ut::reporter<cfg::Printer>>{};

// ----------------------------------------------------------------------------

const boost::ut::suite rules_suite = [] {

using namespace boost::ut;
using namespace boost::ut::bdd;

"PrintHelper"_test = [] {

    given("Given an empty structure") = []{
        S s;

        when("When retrieving printer output") = [&s] {
            cfg::Printer printer;
            printer << cfg::Printer::Value(s);
            auto const result = printer.str();

            then("Then the ouput is equivalent to an empty json") = [&result] {
                auto const reference = "{}";

                expect(that % result == reference);
            };
        };
    };

    given("Given a structure with an integer variable") = [] {
        Si s{ 1 };

        when("When retrieving printer output") = [&s] {
            cfg::Printer printer;
            printer << cfg::Printer::Value(s);
            auto const result = printer.str();

            then("Then the ouput is equivalent to an empty json") = [&result] {
                auto const reference = 
R"({
  "integer": 1
})";

                expect(that % result == reference);
            };
        };
    };

    given("Given a structure with a complexe variable") = [] {
        Sc s{ Si(1), 2.0 };

        when("When retrieving printer output") = [&s] {
            cfg::Printer printer;
            printer << cfg::Printer::Value(s);
            auto const result = printer.str();

            then("Then the ouput is equivalent to an empty json") = [&result] {
                auto const reference =
R"({
  "d": 2.0,
  "i": {
    "integer": 1
  }
})";

                expect(that % result == reference);
            };
        };
    };
};

};

// ----------------------------------------------------------------------------

int main() {}
