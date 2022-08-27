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

// Let compute the piece_corner of a specific piece
// First we need a test case, start with the single square
// In this case, the piece_corners will be {0,0,NW}, {0,0,NE}, {0,0,SW}, and {0,0,SE}
// The generalization would be that the piece_corners are the unique corner from all the piece squares' corners
// For this we need to find the equivalent corner of 2 adjacent squares

// The next step is to gather all the squares of a piece that can be placed at a specific corner
// like, for the 1 square piece at (0,0), for the NW corner, the 2 squares piece can be placed at (-2,1)

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

class Piece {
public:
    Piece(std::vector<Position> squares)
        : squares(std::move(squares))
    {}

    std::vector<Position> const& get_squares() const { return squares; }
private:
    std::vector<Position> squares;
};

std::vector<Corner> create_corners(Position const& position) {
    return { {position, CornerId::NW}, {position, CornerId::NE}, {position, CornerId::SE}, {position, CornerId::SW} };
}

auto get_all_corners(const Piece& piece) {
    const auto& squares = piece.get_squares();

    return ranges::views::join(
        squares |
        ranges::views::transform([](Position const& position) { return create_corners(position); })
    );
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

template<class InRng>
bool is_unique(Corner const& cornerToValidate, InRng&& corners) {
    return ranges::none_of(std::forward<InRng>(corners), [&cornerToValidate](const Corner& corner) {
        return are_equivalent(corner, cornerToValidate);
    });
}

template<class InRng>
auto get_unique_corners(InRng corners) {
    return corners | ranges::views::filter([&corners](Corner const& corner) { return is_unique(corner, corners | ranges::views::remove(corner)); });
}

std::vector< Corner > get_piece_corners(Piece const& piece) {
    auto const all_corners = get_all_corners(piece);
    auto unique_corners = get_unique_corners(all_corners);

    return unique_corners | ranges::to<std::vector>();
}

std::vector<Position> get_available_place_positions(const Corner& corner, const Piece& piece) {
    // TODO
    corner;
    piece;
    return {};
}

template<class InRng, class OutRng = std::remove_cvref_t<InRng>>
auto sort(InRng&& rng) -> OutRng {
    OutRng sortedRng = std::forward<InRng>(rng);
    ranges::sort(sortedRng);
    return sortedRng;
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

//"is_unique"_test = [] {
//
//    given("Given a corner and a list") = [](const std::pair<Piece, std::vector<Corner>>& input) {
//        const auto& [piece, reference] = input;
//
//        when("When getting piece's corners") = [&piece, &reference] {
//            const auto result = get_piece_corners(piece);
//
//            then("Then the piece's corners are the corners that are only on 1 square of the piece") = [&result, &reference] {
//                const auto sorted_result = sort(result);
//                const auto sorted_reference = sort(reference);
//
//                expect(that % sorted_result == sorted_reference);
//
//            };
//        };
//    } | std::vector<std::pair<Piece, std::vector<Corner>>>({
//        { { { { 0,0 } } },        { {{0, 0}, CornerId::NW}, {{0, 0}, CornerId::NE}, {{0, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} } },
//        { { { { 0,0 }, {1,0} } }, { {{0, 0}, CornerId::NW}, {{1, 0}, CornerId::NE}, {{1, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} } },
//        });
//};

"get_piece_corners"_test = [] {

    given("Given a piece") = [](test::Data<Piece, std::vector<Corner>> const& data) {
        auto const& [piece, reference] = data;

        when("When getting piece's corners") = [&piece, &reference] {
            auto const result = get_piece_corners(piece);

            then("Then the piece's corners are the corners that are only on 1 square of the piece") = [&result, &reference] {
                auto const sorted_result = sort(result);
                auto const sorted_reference = sort(reference);

                expect(that % sorted_result == sorted_reference);

            };
        };
    } | std::vector<test::Data<Piece, std::vector<Corner>>>({
        { { { { 0,0 } } },        { {{0, 0}, CornerId::NW}, {{0, 0}, CornerId::NE}, {{0, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} } },
        { { { { 0,0 }, {1,0} } }, { {{0, 0}, CornerId::NW}, {{1, 0}, CornerId::NE}, {{1, 0}, CornerId::SE}, {{0, 0}, CornerId::SW} } },
        });
};

"get_available_place_positions"_test = [] {

    given("Given a corner and a piece") = [] {
        Corner const corner({ 0,0 }, CornerId::NW);
        Piece const piece({ { 0,0 }, { 1,0 } });

        when("When getting available place positions") = [&corner, &piece] {
            auto const place_positions = get_available_place_positions(corner, piece);

            then("Then the available place positions correspond to the list of the piece's opposite corners") = [&place_positions] {
                std::vector<Position> const reference = { {-2, 1} };
                expect(that% place_positions == reference);

            };
        };
    };
};

};

// ----------------------------------------------------------------------------

int main() {
    test_are_equivalent();
    test_is_unique();
}
