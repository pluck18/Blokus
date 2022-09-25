#pragma once

#include <any>
#include <type_traits>

#include "gsl/gsl"
#include "boost/ut.hpp"

#include "Api.h"

// ----------------------------------------------------------------------------

namespace unit_testing {

template<class... Args>
auto erase_type(std::pair<std::string, Args>... list) {
    auto data = std::make_unique<std::vector<std::pair<std::string, std::any>>>();
    ((data->emplace_back(std::move(list))), ...);

    return data.release();
}

class UNITTESTING_API TestPrintHelperData {
public:
    template<class... Args>
    TestPrintHelperData(std::pair<std::string, Args>... list) :
        data(erase_type(std::move(list)...)) {}

private:
    std::vector<std::pair<std::string, std::any>>* data;

    friend std::vector<std::pair<std::string, std::any>> const& get_data(TestPrintHelperData const&);

    friend auto operator<=>(TestPrintHelperData const&, TestPrintHelperData const&) = default;
};

// ----------------------------------------------------------------------------

UNITTESTING_API std::string to_string(TestPrintHelperData const& data);
UNITTESTING_API std::ostream& operator<<(std::ostream& stream, TestPrintHelperData const& data);

// ----------------------------------------------------------------------------

template<typename T> concept to_string_exists = requires (T t) {
    std::to_string(t);
};

template<class T>
    requires to_string_exists<T>
std::string test_print_helper(T const& value) {
    return std::to_string(value);
}

UNITTESTING_API std::string test_print_helper(std::string const& value);
UNITTESTING_API std::string test_print_helper(std::string_view value);
UNITTESTING_API std::string test_print_helper(char const* const value);

// ----------------------------------------------------------------------------

template<class T>
struct Value {
    Value(T const& value);

    T const& get_value() const;

private:
    T value;
};

template<class T>
Value<T>::Value(T const& value) :
    value(value) {
}

template<class T>
T const& Value<T>::get_value() const {
    return value;
}

template<class TLhs, class TRhs>
auto operator<=>(Value<TLhs> const& lhs, Value<TRhs> const& rhs) {
    return lhs.get_value() <=> rhs.get_value();
}

template<class TLhs, class TRhs>
auto operator<=>(Value<TLhs> const& lhs, TRhs const& rhs) {
    return lhs.get_value() <=> rhs;
}

template<class TLhs, class TRhs>
auto operator<=>(TLhs const& lhs, Value<TRhs> const& rhs) {
    return lhs <=> rhs.get_value();
}

template<class TLhs, class TRhs>
auto operator==(Value<TLhs> const& lhs, Value<TRhs> const& rhs) {
    return lhs.get_value() == rhs.get_value();
}

template<class T>
std::ostream& operator<<(std::ostream& stream, Value<T> const& value) {
    stream << test_print_helper(value.get_value());
    return stream;
}

// ----------------------------------------------------------------------------

struct Printer {
public:
    auto str() const { return printer.str(); }
    const auto& colors() const { return printer.colors(); }

    template <class T>
    auto& operator<<(T const& t) {
        printer << boost::ut::detail::get(t);
        return *this;
    }

    //template <class T>
    //auto& operator<<(Value<T> const& t) {
    //    *this << test_print_helper(t.get_value());
    //    return *this;
    //}

    //auto& operator<<(TestPrintHelperData const& t) {
    //    *this << to_string(t);
    //    return *this;
    //}

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

}

template <>
auto boost::ut::cfg<boost::ut::override> = boost::ut::runner<boost::ut::reporter<unit_testing::Printer>>{};
