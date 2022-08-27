// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <vector>


//Blokus Game Board(400 squares)
//84 game pieces(four 21 - piece sets of red, green, blue, and yellow)
//Each color inlcudes :
//1 one - square piece
//1 piece with 2 squares
//2 pieces with 3 squares
//5 pieces with 4 squares
//12 pieces with 5 squares

// All pieces
// 1  | 2  | 3   | 4  | 5    | 6   | 7   | 8  | 9   | 10    | 11   | 12   | 13  | 14   | 15  | 16  | 17  | 18 | 19  | 20  | 21  |
// 1a | 2a | 3a  | 3b | 4a   | 4b  | 4c  | 4d | 4e  | 5a    | 5b   | 5c   | 5d  | 5e   | 5f  | 5g  | 5h  | 5i | 5j  | 5k  | 5l  |
// X  | XX | XXX | XX | XXXX | XXX | XXX | XX | XX  | XXXXX | XXXX | XXXX | XXX | XXX  | XXX | XXX |  X  | XX | XX  | XX  | XX  |
//    |    |     | X  |      | X   |  X  | XX |  XX |       | X    |  X   | XX  |   XX | X   |  X  | XXX | X  |  X  |  XX |  XX |
//    |    |     |    |      |     |     |    |     |       |      |      |     |      | X   |  X  |  X  | XX |  XX |  X  |   X |

// pieces: id, squares
// [piece_id, piece_corners] -> piece_corners are the playable position of this piece
// [piece_id, piece_adjacent_positions] -> piece_adjacent_position, this is where other pieces can be played
// player: id, color

// Let assume that for a specific square, the coners are specified as follow
// NW--NE
// |    |
// SW--SE
// So, to define a corner, we need to specify the square position and the corner on this square

// Let compute the piece_corner of a specific piece
// First we need a test case, start with the single square
// In this case, the piece_corner will be {0,0,NW}, {0,0,NE}, {0,0,SW}, and {0,0,SE}

class Position {
public:
    Position(int x, int y)
        : x(x)
        , y(y)
    {}

private:
    int x;
    int y;

    friend auto operator<=>(const Position&, const Position&) = default;
};

enum class CornerId { NW, NE, SE, SW };

class Corner {
public:
    Corner( Position position, CornerId cornerId)
        : position(std::move(position))
        , cornerId(cornerId)
    {}

private:
    Position position;
    CornerId cornerId;

    friend auto operator<=>(const Corner&, const Corner&) = default;
};


std::vector< Corner > compute_piece_corner() {
    return { {{0, 0}, CornerId::NW}, {{0, 0}, CornerId::NE}, {{0, 0}, CornerId::SW}, {{0, 0}, CornerId::SE} };
}


void test_get_piece_corner() {
    assert(compute_piece_corner() == std::vector< Corner >({ {{0, 0}, CornerId::NW}, {{0, 0}, CornerId::NE}, {{0, 0}, CornerId::SW}, {{0, 0}, CornerId::SE} }));
}

int main() {
    test_get_piece_corner();
}
