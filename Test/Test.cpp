// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <vector>

#include "magic_enum.hpp"
#include "range/v3/all.hpp"


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
// In this case, the piece_corners will be {0,0,NW}, {0,0,NE}, {0,0,SW}, and {0,0,SE}
// The generalization would be that the piece_corners are the unique corner from all the piece squares' corners
// For this we need to find the equivalent corner of 2 adjacent squares

class Position {
public:
    constexpr Position(int x, int y)
        : x(x)
        , y(y)
    {}

    int get_x() const { return x; }
    int get_y() const { return y; }

private:
    int x;
    int y;

    friend auto operator<=>(const Position&, const Position&) = default;
};

class PositionDelta {
public:
    constexpr PositionDelta(int x, int y)
        : x(x)
        , y(y)
    {}

    int get_x() const { return x; }
    int get_y() const { return y; }

private:
    int x;
    int y;

    friend auto operator<=>(const PositionDelta&, const PositionDelta&) = default;
};

constexpr Position operator+(const Position& position, const PositionDelta& delta) {
    return { position.get_x() + delta.get_x(), position.get_y() + delta.get_y() };
}

enum class CornerId { NW, NE, SE, SW };

class Corner {
public:
    constexpr Corner( Position position, CornerId cornerId)
        : position(std::move(position))
        , corner_id(cornerId)
    {}

    const Position& get_position() const { return position; }
    CornerId get_corner_id() const { return corner_id; }

private:
    Position position;
    CornerId corner_id;

    friend auto operator<=>(const Corner&, const Corner&) = default;
};

class Piece {
public:
    Piece(std::vector<Position> squares)
        : squares(std::move(squares))
    {}

    const std::vector<Position>& get_squares() const { return squares; }
private:
    std::vector<Position> squares;
};

std::vector<Corner> create_corners(const Position& position) {
    return { {position, CornerId::NW}, {position, CornerId::NE}, {position, CornerId::SE}, {position, CornerId::SW} };
}

auto get_all_corners(const Piece& piece) {
    const auto& squares = piece.get_squares();

    return ranges::views::join(
        squares |
        ranges::views::transform([](const Position& position) { return create_corners(position); })
    );
}

bool are_equivalent(const Corner& lhs, const Corner& rhs) {
    // Same position, validate corner_id
    if (lhs.get_position() == rhs.get_position()) {
        return lhs.get_corner_id() == rhs.get_corner_id();
    }

    constexpr PositionDelta left( -1,  0);
    constexpr PositionDelta up(    0,  1);
    constexpr PositionDelta right( 1,  0);
    constexpr PositionDelta down(  0, -1);

    static_assert(magic_enum::enum_count<CornerId>() == 4, "New case needs to be added here");
    switch (lhs.get_corner_id())
    {
    case CornerId::NW:
        return
            rhs.get_corner_id() == CornerId::NE && lhs == Corner{ rhs.get_position() + right, CornerId::NW } ||
            rhs.get_corner_id() == CornerId::SW && lhs == Corner{ rhs.get_position() + down,  CornerId::NW };
    case CornerId::NE:
        return
            rhs.get_corner_id() == CornerId::SE && lhs == Corner{ rhs.get_position() + down,  CornerId::NE } ||
            rhs.get_corner_id() == CornerId::NW && lhs == Corner{ rhs.get_position() + left,  CornerId::NE };
    case CornerId::SE:
        return
            rhs.get_corner_id() == CornerId::SW && lhs == Corner{ rhs.get_position() + left,  CornerId::SE } ||
            rhs.get_corner_id() == CornerId::NE && lhs == Corner{ rhs.get_position() + up,    CornerId::SE };
    case CornerId::SW:
        return
            rhs.get_corner_id() == CornerId::NW && lhs == Corner{ rhs.get_position() + up,    CornerId::SW } ||
            rhs.get_corner_id() == CornerId::SE && lhs == Corner{ rhs.get_position() + right, CornerId::SW };
    default:
        assert(false && "Invalid CornerId");
        return false;
    }
}

template<class Rng>
bool is_unique(const Corner& cornerToValidate, Rng&& corners) {
    return ranges::none_of(std::forward<Rng>(corners), [&cornerToValidate](const Corner& corner) {
        const bool isSame = corner == cornerToValidate;
        return !isSame && are_equivalent(corner, cornerToValidate);
    });
}

template<class Rng>
auto get_unique_corners(Rng&& corners) {
    return corners | ranges::views::filter([&corners](const Corner& corner) { return is_unique(corner, corners); });
}

std::vector< Corner > compute_piece_corner(const Piece& piece) {
    auto allCorners = get_all_corners(piece);
    auto uniqueCorners = get_unique_corners(allCorners);

    return uniqueCorners | ranges::to<std::vector>();
}

// ----------------------------------------------------------------------------

void test_are_equivalent() {
    assert(are_equivalent({ {0, 0}, CornerId::NW }, { {0, 0}, CornerId::NW }));
    
    assert(!are_equivalent({ {0, 0}, CornerId::NW }, { {0, 0}, CornerId::NE }));

    assert(are_equivalent({ {0, 0}, CornerId::NW }, { {-1,  0}, CornerId::NE }));
    assert(are_equivalent({ {0, 0}, CornerId::NW }, { { 0,  1}, CornerId::SW }));
    assert(are_equivalent({ {0, 0}, CornerId::NE }, { { 0,  1}, CornerId::SE }));
    assert(are_equivalent({ {0, 0}, CornerId::NE }, { { 1,  0}, CornerId::NW }));
    assert(are_equivalent({ {0, 0}, CornerId::SE }, { { 1,  0}, CornerId::SW }));
    assert(are_equivalent({ {0, 0}, CornerId::SE }, { { 0, -1}, CornerId::NE }));
    assert(are_equivalent({ {0, 0}, CornerId::SW }, { { 0, -1}, CornerId::NW }));
    assert(are_equivalent({ {0, 0}, CornerId::SW }, { {-1,  0}, CornerId::SE }));

    assert(!are_equivalent({ {0, 0}, CornerId::NW }, { {1, 0}, CornerId::SE }));
}

void test_is_unique() {
    assert(is_unique({ {0, 0}, CornerId::NW }, std::vector<Corner>{}));
    assert(is_unique({ {0, 0}, CornerId::NW }, std::vector<Corner>{ { {0, 0}, CornerId::NE } }));

    assert(!is_unique({ {0, 0}, CornerId::NW }, std::vector<Corner>{ { {0, 0}, CornerId::NW } }));
    assert(!is_unique({ {0, 0}, CornerId::NW }, std::vector<Corner>{ { {0, 1}, CornerId::SW } }));
}

void test_get_piece_corner() {
    using namespace ranges::actions;

    assert(sort(compute_piece_corner({ { {0,0} } })) == sort(std::vector< Corner >({ {{0, 0}, CornerId::NW}, {{0, 0}, CornerId::NE}, {{0, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} })));
    assert(sort(compute_piece_corner({ { {0,0}, {1,0} } })) == sort(std::vector< Corner >({ {{0, 0}, CornerId::NW}, {{1, 0}, CornerId::NE}, {{1, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} })));
}

int main() {
    test_are_equivalent();
    test_get_piece_corner();
}
