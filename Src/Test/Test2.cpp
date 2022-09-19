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



//template<typename T>
//concept test_print_helper_exists = requires(T const& v) {
//    { test_print_helper(v) }/* -> std::same_as<TestPrintHelperData>*/;
//};

//requires test_print_helper_exists<T>

struct S {};

TestPrintHelperData test_print_helper(S const& /*value*/) {
    return {};
}

auto test_print_helper(std::string_view value) {
    return value;
}

auto test_print_helper(std::string const& value) {
    return value;
}

auto test_print_helper(char const * const value) {
    return value;
}

// ----------------------------------------------------------------------------

//namespace details {
//    class ReferenceCounterHelper {
//    public:
//        ReferenceCounterHelper(int& counter) :
//            counter(counter) {
//            assert(counter >= 0);
//            ++counter;
//        }
//
//        ~ReferenceCounterHelper() {
//            assert(counter >= 0);
//            --counter;
//        }
//
//    private:
//        int& counter;
//    };
//}
//
//class ReferenceCounter {
//public:
//    ~ReferenceCounter() { assert(counter == 0); }
//
//    details::ReferenceCounterHelper add() { return { counter }; }
//
//    bool is_zero() const { return counter == 0; }
//
//    int counter = 0;
//};

// ----------------------------------------------------------------------------

namespace cfg {

namespace detail {
    template <class T>
    [[nodiscard]] constexpr auto get_value_impl(const T& t, int) -> decltype(t.get_value()) {
        return t.get_value();
    }
    template <class T>
    [[nodiscard]] constexpr auto get_value_impl(const T& t, ...) -> decltype(auto) {
        return t;
    }
    template <class T>
    [[nodiscard]] constexpr auto get_value(const T& t) {
        return get_value_impl(t, 0);
    }
}

struct Printer {
public:
    template<class T>
    struct Value {
        Value(T const& value) : value(value) {}

        auto get_value() const {
            return test_print_helper(value);
        }

    private:
        T const& value;
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
        *this << detail::get_value(t);
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
        //requires test_print_helper_exists<TLhs> and test_print_helper_exists<TRhs>
    auto& operator<<(boost::ut::detail::eq_<TLhs, TRhs> const& op) {
        boost::ut::detail::eq_ value_op(Value(op.lhs()), Value(op.rhs()));
        printer << value_op;
        return *this;
    }

    template <class TLhs, class TRhs, class TEpsilon>
        //requires test_print_helper_exists<TLhs>and test_print_helper_exists<TRhs> and test_print_helper_exists<TEpsilon>
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

}  // namespace cfg

template <>
auto boost::ut::cfg<boost::ut::override> = boost::ut::runner<boost::ut::reporter<cfg::Printer>>{};

// ----------------------------------------------------------------------------

//const boost::ut::suite rules_suite = [] {
//
//using namespace boost::ut;
//using namespace boost::ut::bdd;
//
//"PrintHelper"_test = [] {
//
//    given("Given an empty structure") = []{
//        S s;
//
//        when("When retrieving printer output") = [&s] {
//            cfg::Printer printer;
//            printer << cfg::Printer::Value(s);
//            auto const result = printer.str();
//
//            then("Then the ouput is equivalent to an empty json") = [&result] {
//                auto const reference = "";
//
//                expect(that % result == reference);
//            };
//        };
//    };
//};
//
//};

// ----------------------------------------------------------------------------

int main() {
    assert(test_print_helper(S()).str() == "{}");
    assert((cfg::Printer() << cfg::Printer::Value(S())).str() == "{}");
}
