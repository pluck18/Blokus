
#include "UnitTesting/UnitTest.h"

// ----------------------------------------------------------------------------

struct S {
    friend auto operator<=>(S const&, S const&) = default;
};

unit_testing::TestPrintHelperData test_print_helper(S const& /*value*/) {
    return {};
}

// ----------------------------------------------------------------------------

struct Si {
    int i;

    friend auto operator<=>(Si const&, Si const&) = default;
};

unit_testing::TestPrintHelperData test_print_helper(Si const& value) {
    using namespace std::string_literals;
    return { std::pair{ "integer"s, value.i } };
}

// ----------------------------------------------------------------------------

struct Sc {
    Si i;
    double d;

    friend auto operator<=>(Sc const&, Sc const&) = default;
};

unit_testing::TestPrintHelperData test_print_helper(Sc const& value) {
    using namespace std::string_literals;
    return {
        std::pair{ "i"s, test_print_helper(value.i) },
        std::pair{ "d"s, value.d },
        };
}

// ----------------------------------------------------------------------------

//template <>
//auto boost::ut::cfg<boost::ut::override> = boost::ut::runner<boost::ut::reporter<unit_testing::Printer>>{};

const boost::ut::suite rules_suite = [] {

    using namespace boost::ut;
    using namespace boost::ut::bdd;

    "PrintHelper"_test = [] {

        given("Given an empty structure") = [] {
            S s;

            when("When retrieving printer output") = [&s] {
                unit_testing::Printer printer;
                printer << unit_testing::Value(s);
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
                unit_testing::Printer printer;
                printer << unit_testing::Value(s);
                auto const result = printer.str();

                then("Then the ouput is equivalent the integer type value") = [&result] {
                    auto const reference =
                        R"({
  "integer": 1
})";

                    expect(that % result == reference);
                };
            };
        };

        given("Given a structure with a complex variable") = [] {
            Sc s{ Si(1), 2.0 };

            when("When retrieving printer output") = [&s] {
                unit_testing::Printer printer;
                printer << unit_testing::Value(s);
                auto const result = printer.str();

                then("Then the ouput is a sub-object for the complex variable") = [&result] {
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
