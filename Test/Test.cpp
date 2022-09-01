// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

#include "magic_enum.hpp"
#include "range/v3/all.hpp"
#include "boost/ut.hpp"


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

// Let compute the piece_corner of a specific oriented_piece
// First we need a test case, start with the single square
// In this case, the piece_corners will be {0,0,NW}, {0,0,NE}, {0,0,SW}, and {0,0,SE}
// The generalization would be that the piece_corners are the unique corner from all the oriented_piece squares' corners
// For this we need to find the equivalent corner of 2 adjacent squares

// The next step is to gather all the squares of a oriented_piece that can be placed at a specific corner
// like, for the 1 square oriented_piece at (0,0), for the NW corner, the 2 squares oriented_piece can be placed at (-2,1)

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

    friend auto operator<=>(Position const&, Position const&) = default;
};

std::ostream& operator<<(std::ostream& output, Position const& value) {
    output << "{ x : " << value.get_x() << ", y : " << value.get_y() << " }";
    return output;
}

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

    friend auto operator<=>(PositionDelta const&, PositionDelta const&) = default;
};

constexpr Position operator+(Position const& position, PositionDelta const& delta) {
    return { position.get_x() + delta.get_x(), position.get_y() + delta.get_y() };
}

constexpr Position operator-(Position const& position, PositionDelta const& delta) {
    return { position.get_x() - delta.get_x(), position.get_y() - delta.get_y() };
}

enum class CornerId { NW, NE, SE, SW };
std::ostream& operator<<(std::ostream& output, CornerId const& value) {
    output << magic_enum::enum_name(value);
    return output;
}

class Corner {
public:
    constexpr Corner( Position position, CornerId cornerId)
        : position(std::move(position))
        , corner_id(cornerId)
    {}

    Position const& get_position() const { return position; }
    CornerId get_corner_id() const { return corner_id; }

private:
    Position position;
    CornerId corner_id;

    friend auto operator<=>(Corner const&, Corner const&) = default;
};

std::ostream& operator<<(std::ostream& output, Corner const& value) {
    output << "{ Position : " << value.get_position() << ", ConerId : " << value.get_corner_id() << " }";
    return output;
}

class OrientedPiece {
public:
    OrientedPiece(std::vector<Position> squares)
        : squares(std::move(squares))
    {}

    std::vector<Position> const& get_squares() const { return squares; }
private:
    std::vector<Position> squares;
};

std::vector<Corner> create_corners(Position const& position) {
    return { {position, CornerId::NW}, {position, CornerId::NE}, {position, CornerId::SE}, {position, CornerId::SW} };
}

namespace ranges {

auto get_all_corners(OrientedPiece const& oriented_piece) {
    auto const& squares = oriented_piece.get_squares();

    auto corners = ranges::views::join(
        squares |
        ranges::views::transform([](Position const& position) { return create_corners(position); })
    );

    return corners;
}

}

std::vector<Corner> get_all_corners(OrientedPiece const& oriented_piece) {
    auto corners = ranges::get_all_corners(oriented_piece);
    return corners | ranges::to<std::vector>();
}

bool are_equivalent(Corner const& lhs, Corner const& rhs) {
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
bool has_equivalence(Corner const& corner_to_validate, Rng&& corners) {
    return ranges::any_of(corners, [corner_to_validate](const Corner& corner) {
        return are_equivalent(corner_to_validate, corner);
        });
}

namespace ranges {

template<class Rng>
auto get_unique_corners(Rng&& corners) {
    return corners | ranges::views::filter([corners](Corner const& corner) mutable {
        return !has_equivalence(corner, corners | ranges::views::remove(corner));
        });
}

}

namespace ranges {

auto get_piece_corners(OrientedPiece const& oriented_piece) {
    auto all_corners = ranges::get_all_corners(oriented_piece);
    auto unique_corners = ranges::get_unique_corners(all_corners);

    return unique_corners;
}

}

std::vector< Corner > get_piece_corners(OrientedPiece const& oriented_piece) {
    auto unique_corners = ranges::get_piece_corners(oriented_piece);
    return unique_corners | ranges::to<std::vector>();
}

PositionDelta get_displacement(Corner const& corner) {
    auto const& position = corner.get_position();
    auto const& x = position.get_x();
    auto const& y = position.get_y();

    static_assert(magic_enum::enum_count<CornerId>() == 4, "New case needs to be added here");
    switch (corner.get_corner_id())
    {
    case CornerId::NW:
        return { x, -y };
    case CornerId::NE:
        return { -x, -y };
    case CornerId::SE:
        return { -x, y };
    case CornerId::SW:
        return { x, y };
    default:
        assert(false && "Invalid CornerId");
        return { 0, 0 };
    }
}

namespace ranges {

template<class Rng>
auto get_corresponding_corner(Rng&& corners, CornerId const& corner_id)
{
    auto corresponding_corners = std::forward<Rng>(corners) | ranges::views::filter([corner_id](Corner const& corner_to_validate) {
        return corner_to_validate.get_corner_id() == corner_id;
        });

    return corresponding_corners;
}

template<class Rng>
auto get_moves_displacement(Rng&& corners)
{
    auto displacements = corners | ranges::views::transform([](Corner const& corner) {
        return get_displacement(corner);
        });

    return displacements;
}

auto get_all_oriented_piece_displacement(OrientedPiece const& oriented_piece, CornerId const& corner_id) {
    auto corners = ranges::get_piece_corners(oriented_piece);
    auto valid_corners = ranges::get_corresponding_corner(corners, corner_id);
    auto displacements = ranges::get_moves_displacement(valid_corners);

    return displacements;
}

}

std::vector<PositionDelta> get_all_oriented_piece_displacement(OrientedPiece const& oriented_piece, CornerId const& corner_id) {
    auto displacements = ranges::get_all_oriented_piece_displacement(oriented_piece, corner_id);
    return displacements | ranges::to<std::vector>();
}

namespace ranges {

template<class Rng>
auto translate_position(Position const& position, Rng&& displacements) {
    auto translated_position = displacements | ranges::views::transform([position](PositionDelta const& displacement) {
        return position + displacement;
        });
    return translated_position;
}

}

template<class Rng>
std::vector<Position> translate_position(Position const& position, Rng&& displacements) {
    auto translated_position = ranges::translate_position(position, displacements);
    return translated_position | ranges::to<std::vector>();
}

namespace ranges {

auto get_all_oriented_piece_moves_position(OrientedPiece const& oriented_piece, Corner const& corner) {
    auto displacements = ranges::get_all_oriented_piece_displacement(oriented_piece, corner.get_corner_id());
    auto moves_position = ranges::translate_position(corner.get_position(), displacements);

    return moves_position;
}

}

std::vector<Position> get_all_oriented_piece_moves_position(OrientedPiece const& oriented_piece, Corner const& corner) {
    auto moves_position = ranges::get_all_oriented_piece_moves_position(oriented_piece, corner);
    return moves_position | ranges::to<std::vector>();
}

template<class InRng, class OutRng = std::remove_cvref_t<InRng>>
auto sort(InRng&& rng) -> OutRng {
    OutRng sortedRng = std::forward<InRng>(rng);
    ranges::sort(sortedRng);
    return sortedRng;
}

// ----------------------------------------------------------------------------

namespace test {

template<class Input, class Reference>
struct Data {
private:
    using input_type = std::remove_cvref_t<Input>;
    using reference_type = std::remove_cvref_t<Reference>;

public:
    Data(Input input, Reference reference)
        : input(std::move(input))
        , reference(std::move(reference))
    {}

    input_type const& get_input() const { return input; }
    reference_type const& get_reference() const { return reference; }

private:
    input_type input;
    reference_type reference;
};

template<std::size_t Index, class Input, class Reference>
std::tuple_element_t<Index, Data<Input, Reference> const>& get(Data<Input, Reference> const& data)
{
    if constexpr (Index == 0) return data.get_input();
    if constexpr (Index == 1) return data.get_reference();
}

template<class Input, class Reference>
using DataSet = std::vector<Data<Input, Reference>>;

}

namespace std {

template<class Input, class Reference>
struct tuple_size<test::Data<Input,Reference>>
    : integral_constant<size_t, 2> {};

template<size_t Index, class Input, class Reference>
struct tuple_element<Index, test::Data<Input, Reference>>
    : conditional<Index == 0, Input, Reference>
{
    static_assert(Index < 2, "Index out of test::Data");
};

}

const boost::ut::suite rules_suite = [] {

using namespace boost::ut;
using namespace boost::ut::bdd;

"are_equivalent"_test = [] {

    given("Given 2 corners") = [](test::Data<std::pair<Corner, Corner>, bool> const& data) {
        const auto& [input, reference] = data;

        when("When validating if 2 corners are equivalent") = [&input, &reference] {
            auto const& [lhs, rhs] = input;
            auto const result = are_equivalent(lhs, rhs);

            then("Then the oriented piece's corners are the corners that are only on 1 square of the oriented piece") = [&result, &reference] {
                expect(that % result == reference);

            };
        };
    } | std::vector<test::Data<std::pair<Corner, Corner>, bool>>({
        { { { {0, 0}, CornerId::NW }, { { 0,  0}, CornerId::NW } }, true },
        { { { {0, 0}, CornerId::NW }, { { 0,  0}, CornerId::NE } }, false },
        { { { {0, 0}, CornerId::NW }, { {-1,  0}, CornerId::NE } }, true },
        { { { {0, 0}, CornerId::NW }, { { 0,  1}, CornerId::SW } }, true },
        { { { {0, 0}, CornerId::NE }, { { 0,  1}, CornerId::SE } }, true },
        { { { {0, 0}, CornerId::NE }, { { 1,  0}, CornerId::NW } }, true },
        { { { {0, 0}, CornerId::SE }, { { 1,  0}, CornerId::SW } }, true },
        { { { {0, 0}, CornerId::SE }, { { 0, -1}, CornerId::NE } }, true },
        { { { {0, 0}, CornerId::SW }, { { 0, -1}, CornerId::NW } }, true },
        { { { {0, 0}, CornerId::SW }, { {-1,  0}, CornerId::SE } }, true },
        { { { {0, 0}, CornerId::NW }, { { 1,  0}, CornerId::SE } }, false },
        });
};

"has_equivalence"_test = [] {

    given("Given a corner and a list") = [](test::Data<std::tuple<Corner, std::vector<Corner>>,bool> const& data) {
        const auto& [input, reference] = data;

        when("When validating if the corner has an equivalence") = [&input, &reference] {
            auto const& [corner_to_validate, corners] = input;
            auto const result = has_equivalence(corner_to_validate, corners);

            then("Then the oriented piece's corners are the corners that are only on 1 square of the oriented piece") = [&result, &reference] {
                expect(that % result == reference);

            };
        };
    } | std::vector<test::Data<std::tuple<Corner, std::vector<Corner>>,bool>>({
        { { { {0, 0}, CornerId::NW }, { { {0, 0}, CornerId::NW } } }, true },
        { { { {0, 0}, CornerId::NW }, { { {0, 1}, CornerId::SW } } }, true },
        { { { {0, 0}, CornerId::NW }, {} },                           false },
        { { { {0, 0}, CornerId::NW }, { { {0, 0}, CornerId::NE } } }, false },
        });
};

"get_piece_corners"_test = [] {

    given("Given a oriented piece") = [](test::Data<OrientedPiece, std::vector<Corner>> const& data) {
        auto const& [oriented_piece, reference] = data;

        when("When getting oriented piece's corners") = [&oriented_piece, &reference] {
            auto const result = get_piece_corners(oriented_piece);

            then("Then the oriented piece's corners are the corners that are only on 1 square of the oriented piece") = [&result, &reference] {
                auto const sorted_result = sort(result);
                auto const sorted_reference = sort(reference);

                expect(that % sorted_result == sorted_reference);

            };
        };
    } | std::vector<test::Data<OrientedPiece, std::vector<Corner>>>({
        { { { { 0,0 } } },        { {{0, 0}, CornerId::NW}, {{0, 0}, CornerId::NE}, {{0, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} } },
        { { { { 0,0 }, {1,0} } }, { {{0, 0}, CornerId::NW}, {{1, 0}, CornerId::NE}, {{1, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} } },
        });
};

"get_all_oriented_piece_moves_position"_test = [] {

    given("Given a corner and a oriented piece") = [](test::Data<std::tuple<OrientedPiece, Corner>, std::vector<Position>> const& data) {
        auto const& [input, reference] = data;

        when("When getting all oriented piece moves displacement") = [&input, &reference] {
            auto const& [oriented_piece, corner] = input;
            auto const result = get_all_oriented_piece_moves_position(oriented_piece, corner);

            then("Then getting all place positions correspond to the list of the oriented piece's opposite corners") = [&result, &reference] {
                expect(that% result == reference);

            };
        };
    } | std::vector< test::Data<std::tuple<OrientedPiece, Corner>, std::vector<Position>>>({
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::SE } }, { { -1, 0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::SW } }, { {  0, 0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::NW } }, { {  0, 0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::NE } }, { { -1, 0 } } },
        // TODO: Add tests on get_all_oriented_piece_displacement
        // TODO: Add tests with pieces of 3 squares in get_all_oriented_piece_displacement
        // TODO: Move those tests to get_all_oriented_piece_displacement, duplicate one here
        // TODO: Add tests with a different corner position
        });
};

};

// TODO: Add the real place position from the corner, like {{0,0},CornerId::NW} should give {{-1,-1},CornerId::SE}
// Something like get_all_place_positions
// get_all_oriented_piece_move(oriented_piece, possible_places) -> move, where a move is a placed_oriented_piece (an oriented piece with a translation applied)

// ----------------------------------------------------------------------------

int main() {}
