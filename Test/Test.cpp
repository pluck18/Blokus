// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <sstream>
#include <tuple>
#include <vector>

#include "boost/ut.hpp"
//#include "magic_enum/magic_enum.h"


//Blokus Game Board(400 squares)
//84 game pieces(four 21 - piece sets of red, green, blue, and yellow)
//Each color inlcudes :
//1 one - square piece
//1 piece with 2 squares
//2 pieces with 3 squares
//5 pieces with 4 squares
//12 pieces with 5 squares

// What is the simplest thing we can test.
// The first possible moves are all the pieces in there respective corner in all there orientations.
// We can simplify this by selecting red as the first color, then green, blue, then yellow.
// Also, we need to fix the corner for each color, so red is the upper left corner, and the rest follows a clockwise pattern.
// What we are constraining is substitution of colors and starting positions.
// This means that for each solution you can rotate the solution by 90 deg and it will be a valid solution

// So the first move is every pieces in all there 8 orientations (4 orientations, 2 side), which correspond to 8 * 21 = 168.
// So, the first test should be that all the number of possible moves for the first move is 168

// The next step would be to validate the moves for a random board, but this is a very huge step.
// Lets try to reduce the problem.
// We could get the adjacent position of a piece and then we will know where to put the following piece.
// After that we will need to validate for each piece-position-orientation if it fit into the board


// We can easily deduce that for the single square all orientations are exactly the same so there is no need to consider 8 different moves.
// So the real number of moves is not 168. We will come back to this later


constexpr unsigned int nbPieces = 21;

enum class PlayerColor { Red };

class Board {

};

class Move {

};

class Position {
    friend auto operator<=>(const Position&, const Position&) = default;
};

void operator<<(std::ostream& /*stream*/, const Position& /*value*/) {

}

void PrintHelper(std::ostream& stream, const Position& /*value*/) {
    stream << "PrintHelper!!!\n";
}

enum class OrientationId { Front_0, Front_90, Front_180, Front_270, Back_0, Back_90, Back_180, Back_270 };

// P1a X
// P2a XX
// P3a XXX
// P3b XX
//     X
// P4a XXXX
// P4b XXX
//     X
// P4c XXX
//      X
// P4d XX
//     XX
enum class PieceId { P1a, P2a, P3a, P3b };

std::vector<Move> GetAvailableMoves(const Board& /*board*/, const PlayerColor /*turn*/, const std::vector<PieceId>& remainingPieces) {
    const auto nbOrientations = 8;
    const auto count = std::size(remainingPieces) * nbOrientations;
    return std::vector<Move>(count);
}

std::vector<Position> GetAdjacentPositions(PieceId /*pieceId*/, OrientationId /*orientationId*/) {
    return {};
}

Board CreateEmptyBoard() {
    return {};
}

namespace ut_custom {

namespace type_traits {
template <class T>
concept has_custom_user_print = requires(std::ostream & stream, const T & t) { PrintHelper(stream, t); };
}  // namespace type_traits

static_assert(type_traits::has_custom_user_print<Position>);

namespace cfg {

struct Printer : boost::ut::printer {
    template <class T>
        requires ut_custom::type_traits::has_custom_user_print<T>
    auto& operator<<(const T& t) {
        static_assert(std::is_same_v< T, Position >);
        std::ostringstream stream;
        PrintHelper(stream, t);
        operator<<(stream.str());
        return *this;
    }

    template <class T>
    auto& operator<<(const T& t) {
        static_assert(not std::is_same_v< T, Position >);
        std::cout << t;
        boost::ut::printer::operator<<(t);
        return *this;
    }
};
}  // namespace boost::ut::cfg
}  // namespace boost::ut

template <>
auto boost::ut::cfg<boost::ut::override> = boost::ut::runner<boost::ut::reporter<ut_custom::cfg::Printer>>{};



using namespace boost::ut;
using namespace boost::ut::bdd;


const suite AdjacentPosition_suite = [] {

    using AdjacentPositionTestDataSetType = std::tuple < PieceId, OrientationId, const std::vector<Position>&>;
    "AdjacentPosition"_test = [](const AdjacentPositionTestDataSetType& input) {
        auto& [pieceId, orientationId, reference] = input;
        given("Given a piece and an orientation") = [pieceId, orientationId, &reference] {
            when("When getting adjacent positions") = [pieceId, orientationId, &reference] {
                const auto result = GetAdjacentPositions(pieceId, orientationId);
                then("Then the total number of pieces times the number of orientations") = [&result, &reference] {
                    expect(that % result == reference);
                };
            };
        };
    } | std::vector<AdjacentPositionTestDataSetType>( {
        { PieceId::P1a, OrientationId::Front_0, std::vector<Position>{} }
    } );

};


const suite AvailableMovements_suite = [] {

    "AvailableMovements"_test = [] {
        given("Given an empty board") = [] {
            const auto board = CreateEmptyBoard();
            when("When getting available movements") = [&board] {
                const auto moves = GetAvailableMoves(board, PlayerColor::Red, std::vector< PieceId >(nbPieces));
                then("Then the total number of moves is the number of pieces times the number of orientations") = [&moves] {
                    expect( that % moves.size() == 168);
                };
            };
        };
    };

};


int main() {}
