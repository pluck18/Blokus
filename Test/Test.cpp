// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "boost/ut.hpp"

using namespace boost::ut;
using namespace boost::ut::bdd;


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


// We can easily deduce that for the single square all orientations are exactly the same so there is no need to consider 8 different moves.
// So the real number of moves is not 168. We will come back to this later


constexpr unsigned int nbPieces = 21;

enum class PlayerColor { Red };

class Board {

};

class Move {

};

class Piece {

};

std::vector<Move> GetAvailableMoves(const Board& /*board*/, const PlayerColor /*turn*/, const std::vector< Piece >& remainingPieces) {
    const auto nbOrientations = 8;
    const auto count = std::size(remainingPieces) * nbOrientations;
    return std::vector<Move>( count );
}

Board CreateEmptyBoard() {
    return {};
}

const suite available_movement_suite = [] {

    "Empty board"_test = [] {
        given("Given an empty board") = [] {
            const auto board = CreateEmptyBoard();
            when("When getting available movement") = [&board] {
                const auto moves = GetAvailableMoves(board, PlayerColor::Red, std::vector< Piece >(nbPieces));
                then("Then the total number of pieces times the number of orientations") = [&moves] {
                    expect( that % moves.size() == 168);
                };
            };
        };
    };

};


int main() {}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
