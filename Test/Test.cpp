// Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>

#include "boost/ut.hpp"
#include "magic_enum.hpp"
#include "range/v3/all.hpp"
#include "fmt/format.h"


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

// The next step is to gather all the squares of an oriented_piece that can be placed at a specific corner
// like, for the 1 square oriented_piece at (0,0), for the NW corner, the 2 squares oriented_piece can be placed at (-2,1)

// I've skipped the big idea.
// Let restart from the beginning.
// What I want is to play a regular game where I can keep the state of every player moves
// Something like the state at the beginning, the state after Player 1 move. In order to go through all the possible games,
// I would need either a way to keep track of all the possible other moves so that all siblings can be created right away
// or to keep only an index of the current move and recompute the list of possible moves and increment the index for the next sibling.
// I need to keep in mind that this are volatile points.
// My first guess would be to keep all the possible moves which would be composed of a piece, an orientation and a position.
// This would be equivalent to 2 enums (shorts) and 2 ints which would be equivalent to 3 ints.
// Re-reading it make me realize that my state database should be event sourcing style.
// A state in this case would be the delta to the previous one probably, but this is to be memory efficient. Otherwise, we would have to 
// keep the state as the full knowledge of all pieces
// There is a lot of optimization here, but I should stick to keeping all states first and then optimize memory foot print
// So, going back to the problem, what should be the first step?
// From the initial board game state, the first step would be to find all the available moves for Player 1
// After that it would be to store all the moves for that state, apply the move and store the new state
// After that it would be to repeat for Player 2, then Player 3, then Player 4, and then go back to Player 1.
// Let start with the initial state

class Position {
public:
    constexpr Position(int x, int y)
        : x(x)
        , y(y)
    {}

    constexpr int get_x() const { return x; }
    constexpr int get_y() const { return y; }

private:
    int x;
    int y;

    friend auto operator<=>(Position const&, Position const&) = default;
};

namespace fmt {

template <>
class formatter<Position> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
        //    if (i != end && (*i == 'f' || *i == 'e')) {
        //        presentation_ = *i++;
        //    }
        //    if (i != end && *i != '}') {
        //        throw format_error("invalid format");
        //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(Position const& value, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "{{ x : {}, y : {} }}", value.get_x(), value.get_y());
    }
};

}

std::ostream& operator<<(std::ostream& output, Position const& value) {
    output << fmt::format("{}", value);
    return output;
}

class PositionDelta {
public:
    constexpr PositionDelta(int x, int y)
        : x(x)
        , y(y)
    {}

    constexpr int get_x() const { return x; }
    constexpr int get_y() const { return y; }

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

namespace fmt {

template <>
class formatter<CornerId> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
        //    if (i != end && (*i == 'f' || *i == 'e')) {
        //        presentation_ = *i++;
        //    }
        //    if (i != end && *i != '}') {
        //        throw format_error("invalid format");
        //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(CornerId const& value, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "{}", magic_enum::enum_name(value));
    }
};

}

std::ostream& operator<<(std::ostream& output, CornerId const& value) {
    output << fmt::format("{}", value);
    return output;
}

class Corner {
public:
    constexpr Corner( Position position, CornerId cornerId)
        : position(std::move(position))
        , corner_id(cornerId)
    {}

    constexpr Position const& get_position() const { return position; }
    constexpr CornerId get_corner_id() const { return corner_id; }

private:
    Position position;
    CornerId corner_id;

    friend auto operator<=>(Corner const&, Corner const&) = default;
};

namespace fmt {

template <>
class formatter<Corner> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
        //    if (i != end && (*i == 'f' || *i == 'e')) {
        //        presentation_ = *i++;
        //    }
        //    if (i != end && *i != '}') {
        //        throw format_error("invalid format");
        //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(Corner const& value, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "{{ Position : {}, CornerId : {} }}", value.get_position(), value.get_corner_id());
    }
};

}

std::ostream& operator<<(std::ostream& output, Corner const& value) {
    output << fmt::format("{}", value);
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

    friend auto operator<=>(OrientedPiece const&, OrientedPiece const&) = default;
};

std::ostream& operator<<(std::ostream& output, OrientedPiece const& /*value*/) {
    // TODO: to be completed
    return output;
}


class Piece {
public:

private:
    friend auto operator<=>(Piece const&, Piece const&) = default;
};

namespace fmt {

template <>
class formatter<Piece> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
    //    if (i != end && (*i == 'f' || *i == 'e')) {
    //        presentation_ = *i++;
    //    }
    //    if (i != end && *i != '}') {
    //        throw format_error("invalid format");
    //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(Piece const& /*p*/, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "{{}}");
    }
};

}

std::ostream& operator<<(std::ostream& output, Piece const& value) {
    output << fmt::format("{}", value);
    return output;
}

enum class PlayerId { Red, Green, Blue, Yellow };

namespace fmt {

template <>
class formatter<PlayerId> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
        //    if (i != end && (*i == 'f' || *i == 'e')) {
        //        presentation_ = *i++;
        //    }
        //    if (i != end && *i != '}') {
        //        throw format_error("invalid format");
        //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(PlayerId const& value, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "{}", magic_enum::enum_name(value));
    }
};

}

std::ostream& operator<<(std::ostream& output, PlayerId const& value) {
    output << fmt::format("{}", value);
    return output;
}

class Game {
public:
    static Game CreateNew( std::vector<PlayerId> players ) {
        return { std::move(players) };
    }

    std::vector<Piece> get_pieces_on_board() const {
        return {};
    }

    PlayerId get_current_player() const {
        return players[current_player];
    }

private:
    Game(std::vector<PlayerId> players) : players(std::move(players)) {
        assert(!this->players.empty());
    }

    std::vector<PlayerId> players;
    size_t current_player{ 0 };

    friend auto operator<=>(Game const&, Game const&) = default;
};

namespace fmt {

template <class Type, class Alloc>
class formatter<std::vector<Type, Alloc>> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
        //    if (i != end && (*i == 'f' || *i == 'e')) {
        //        presentation_ = *i++;
        //    }
        //    if (i != end && *i != '}') {
        //        throw format_error("invalid format");
        //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(std::vector<Type, Alloc> const& values, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "[{}]", fmt::join(values, ","));
    }
};

}

template<class Type, class Alloc>
std::ostream& operator<<(std::ostream& output, std::vector<Type, Alloc> const& values) {
    output << fmt::format("{}", values);
    return output;
}

namespace fmt {

template <>
class formatter<Game> {
    // format specification storage
    //char presentation_ = 'f';
public:
    // parse format specification and store it:
    constexpr auto parse(format_parse_context& ctx) {
        auto i = ctx.begin()/*, end = ctx.end()*/;
        //    if (i != end && (*i == 'f' || *i == 'e')) {
        //        presentation_ = *i++;
        //    }
        //    if (i != end && *i != '}') {
        //        throw format_error("invalid format");
        //    }
        return i;
    }

    // format a value using stored specification:
    template <typename FmtContext>
    constexpr auto format(Game const& value, FmtContext& ctx) const {
        //// note: we can't use ternary operator '?:' in a constexpr
        //switch (presentation_) {
        //default:
        //    // 'ctx.out()' is an output iterator
        //case 'f': return format_to(ctx.out(), "({:f}, {:f})", p.x, p.y);
        //case 'e': return format_to(ctx.out(), "({:e}, {:e})", p.x, p.y);
        //}
        return format_to(ctx.out(), "{{ Pieces on board : {}, Current player : {} }}", value.get_pieces_on_board(), value.get_current_player());
    }
};

}

std::ostream& operator<<(std::ostream& output, Game const& value) {
    output << fmt::format("{}", value);
    return output;
}

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
    return ranges::any_of(std::forward<Rng>(corners), [&corner_to_validate](const Corner& corner) {
        return are_equivalent(corner_to_validate, corner);
        });
}

namespace ranges {

template<class Rng>
auto get_unique_corners(Rng&& corners) {
    return std::forward<Rng>(corners) | ranges::views::filter([corners](Corner const& corner) mutable {
        auto corners_copy = ranges::copy(corners);
        return !has_equivalence(corner, corners_copy | ranges::views::remove(corner));
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
    return std::move(unique_corners) | ranges::to<std::vector>();
}

PositionDelta get_displacement(Corner const& corner) {
    auto const& position = corner.get_position();
    auto x = position.get_x();
    auto y = position.get_y();

    return { -x, -y };
}

namespace ranges {

template<class Rng>
auto get_corresponding_corners(Rng&& corners, CornerId const& corner_id)
{
    auto corresponding_corners = std::forward<Rng>(corners) | ranges::views::filter([corner_id](Corner const& corner_to_validate) {
        return corner_to_validate.get_corner_id() == corner_id;
        });

    return corresponding_corners;
}

template<class Rng>
auto get_moves_displacement(Rng&& corners)
{
    auto displacements = std::forward<Rng>(corners) | ranges::views::transform([](Corner const& corner) {
        return get_displacement(corner);
        });

    return displacements;
}

auto get_all_oriented_piece_moves_displacement(OrientedPiece const& oriented_piece, CornerId const& corner_id) {
    auto corners = ranges::get_piece_corners(oriented_piece);
    auto valid_corners = ranges::get_corresponding_corners(std::move(corners), corner_id);
    auto displacements = ranges::get_moves_displacement(std::move(valid_corners));

    return displacements;
}

}

std::vector<PositionDelta> get_all_oriented_piece_moves_displacement(OrientedPiece const& oriented_piece, CornerId const& corner_id) {
    auto displacements = ranges::get_all_oriented_piece_moves_displacement(oriented_piece, corner_id);
    return std::move(displacements) | ranges::to<std::vector>();
}

namespace ranges {

template<class Rng>
auto translate_position(Position const& position, Rng&& displacements) {
    auto translated_position = std::forward<Rng>(displacements) | ranges::views::transform([position](PositionDelta const& displacement) {
        return position + displacement;
        });
    return translated_position;
}

}

template<class Rng>
std::vector<Position> translate_position(Position const& position, Rng&& displacements) {
    auto translated_position = ranges::translate_position(position, displacements);
    return std::move(translated_position) | ranges::to<std::vector>();
}

namespace ranges {

auto get_all_oriented_piece_moves_position(OrientedPiece const& oriented_piece, Corner const& corner) {
    auto displacements = ranges::get_all_oriented_piece_moves_displacement(oriented_piece, corner.get_corner_id());
    auto moves_position = ranges::translate_position(corner.get_position(), std::move(displacements));

    return moves_position;
}

}

std::vector<Position> get_all_oriented_piece_moves_position(OrientedPiece const& oriented_piece, Corner const& corner) {
    auto moves_position = ranges::get_all_oriented_piece_moves_position(oriented_piece, corner);
    return std::move(moves_position) | ranges::to<std::vector>();
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


"BoardState"_test = [] {

    given("Given a new game and players") = []{
        PlayerId first_player = PlayerId::Yellow;
        auto const game = Game::CreateNew({ first_player, PlayerId::Red, PlayerId::Green, PlayerId::Blue });

        when("When getting the board state") = [&game] {
            auto const result = game.get_pieces_on_board();

            then("Then there is no pieces on the board") = [&result] {
                decltype(result) reference = {};

                expect(that % result == reference);

            };
        };

        when("When getting the current player") = [&game, &first_player] {
            auto const result = game.get_current_player();

            then("Then the current player is the first player") = [&result, &first_player] {
                auto const reference = first_player;

                expect(that % result == reference);

            };
        };
    };

    //given("Given a player move")
};

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
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::SE } }, { { -1,  0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::SW } }, { {  0,  0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::NW } }, { {  0,  0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 0, 0 }, CornerId::NE } }, { { -1,  0 } } },
        { { { { { 0,0 }, { 1,0 } } }, { { 1, 2 }, CornerId::SE } }, { {  0,  2 } } },
        { { { { { 1,2 }, { 2,2 } } }, { { 0, 0 }, CornerId::SE } }, { { -2, -2 } } },
        { { { { { 0,0 }, { 0,1 }, { 1,1 } } }, { { 0, 0 }, CornerId::SE } }, { { 0, 0 }, { -1, -1 } } },
        });
};

};

// TODO: Add the real place position from the corner, like {{0,0},CornerId::NW} should give {{-1,-1},CornerId::SE}
// Something like get_all_place_positions
// get_all_oriented_piece_move(oriented_piece, possible_places) -> move, where a move is a placed_oriented_piece (an oriented piece with a translation applied)

// ----------------------------------------------------------------------------

int main() {}
