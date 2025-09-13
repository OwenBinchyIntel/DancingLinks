#include <cassert>
#include <bitset>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <array>

#ifdef _WIN32
    #include <immintrin.h>
#else // _WIN32
    #include <x86intrin.h>
#endif // _WIN32

namespace {

inline unsigned nthset(unsigned x, unsigned n) {
    return _pdep_u32(1UL << n, x);
}
inline std::uint64_t nthset(std::uint64_t x, unsigned n) {
    return _pdep_u64(1ULL << n, x);
}

// using Board = std::vector<std::vector<bool>>;

// template<std::size_t N>
// class Board {
// public:

// // private:
//     std::bitset<N*N> m_Cells;
// };

template <std::size_t N>
using Board = std::bitset<N * N>;

template <std::size_t N>
void SetBoardUp(Board<N> &board, unsigned n) {
    if (n < N)
        return;
    board.set(n - N);
    return;
}
template <std::size_t N>
void SetBoardDown(Board<N> &board, unsigned n) {
    if (n >= (N * (N - 1)))
        return;
    board.set(n + N);
    return;
}

template <std::size_t N>
void SetBoardLeft(Board<N> &board, unsigned n) {
    if (n % N == 0)
        return;
    board.set(n - 1);
    return;
}
template <std::size_t N>
void SetBoardRight(Board<N> &board, unsigned n) {
    if ((n + 1) % N == 0)
        return;
    board.set(n + 1);
    return;
}

template <std::size_t N>
Board<N> LightMaskFromButton(unsigned n) {
    assert(n < (N * N));
    Board<N> mask;

    mask.set(n);
    SetBoardUp<N>(mask, n);
    SetBoardDown<N>(mask, n);
    SetBoardLeft<N>(mask, n);
    SetBoardRight<N>(mask, n);

    return mask;
}

static std::array<Board<3>, (3 * 3)> g_ButtonMasksFromLights{{
    {Board<3>(0b011100101)},
    {Board<3>(0b111010000)},
    {Board<3>(0b110001101)},
    {Board<3>(0b100110100)},
    {Board<3>(0b010111010)},
    {Board<3>(0b001011001)},
    {Board<3>(0b101100011)},
    {Board<3>(0b000010111)},
    {Board<3>(0b101001110)},
}};

template <std::size_t N>
Board<N> ButtonMaskFromLight(unsigned n) {
    return g_ButtonMasksFromLights[n];
}

template <std::size_t N>
void PrintBoard(const Board<N> &board) {
    for (unsigned row = 0; row < N; ++row) {
        Board<N> sBoard = board >> (row * N);
        std::bitset<N> pBoard(sBoard.to_ullong());
        std::cout << pBoard << '\n';
    }
}

template <std::size_t N>
Board<N> ButtonsToLights(const Board<N> &buttons) {
    Board<N> lights;

    for (unsigned n = 0; n < (N * N); ++n) {
        if (buttons.test(n)) {
            Board<N> mask = LightMaskFromButton<N>(n);
            lights ^= mask;
        }
    }

    return lights;
}

template <std::size_t N>
Board<N> LightsToButtons(const Board<N> &lights) {
    Board<N> buttons;

    for (unsigned n = 0; n < (N * N); ++n) {
        if (lights.test(n)) {
            Board<N> mask = ButtonMaskFromLight<N>(n);
            buttons ^= mask;
        }
    }

    return buttons;
}

// Helpers
template <std::size_t N>
Board<N> Button(unsigned n) {
    Board<N> buttons;
    buttons.set(n);
    return buttons;
}
template <std::size_t N>
Board<N> Light(unsigned n) {
    return Button<N>(n);
}

void TestAssumption() {
    constexpr unsigned N = 3;
    for (unsigned l = 0; l < (1UL << 9); ++l) {
        Board<N> lights(l);
        for (unsigned b = 0; b < (1UL << 9); ++b) {
            Board<N> buttons(b);
            assert((LightsToButtons<N>(lights) ^ buttons) ==
                   (LightsToButtons<N>(lights ^ ButtonsToLights<N>(buttons))));

            assert((ButtonsToLights<N>(buttons) ^ lights) ==
                   (ButtonsToLights<N>(buttons ^ LightsToButtons<N>(lights))));
        }
    }
    std::cout << "Assumption true for all lights and buttons\n";
}

void BuildButtonMap() {
    constexpr unsigned N = 3;
    std::array<Board<N>, N * N> buttonMasksFromLights;

    for (unsigned l = 0; l < N; ++l) {
        // buttonMasksFromLights[l] = LightsToButtons<N>(Light<N>(l));

        // buttonMasksFromLights[l] = ButtonMaskFromLight<N>(l);
    }
}

} // namespace

int main() {
    // int n;
    // std::cin >> n; std::cin.ignore();
    // for (int i = 0; i < n; i++) {
    //     std::string row;
    //     getline(std::cin, row);
    // }
    // for (int i = 0; i < n; i++) {
    //     std::cout << "row #i" << std::endl;
    // }

    constexpr unsigned N = 3;
    for (unsigned n = 0; n < (N * N); ++n) {
        std::cout << "Bit : " << n << '\n';

        if (0) {
            Board<N> buttons;
            buttons.set(n);
            PrintBoard<N>(buttons);
            std::cout << '\n';

            Board<N> mask = LightMaskFromButton<N>(n);
            PrintBoard<N>(mask);
            std::cout << '\n';
        }
        if (0) {
            Board<N> lights;
            lights.set(n);
            PrintBoard<N>(lights);
            std::cout << '\n';

            Board<N> mask = ButtonMaskFromLight<N>(n);
            PrintBoard<N>(mask);
            std::cout << '\n';
        }

        if (0) {
            // Board<N> buttons(0b011100101);
            Board<N> buttons;
            buttons.set(1);
            // buttons.set(3);
            buttons.set(4);
            // buttons.set(5);
            // buttons.set(7);
            PrintBoard<3>(buttons);
            std::cout << '\n';

            Board<N> lights = ButtonsToLights<3>(buttons);
            PrintBoard<3>(lights);
            std::cout << '\n';

            Board<N> buttonsResult = LightsToButtons<3>(lights);
            PrintBoard<3>(buttonsResult);
            std::cout << '\n';
        }
    }
    for (unsigned x = 0; x < (1UL << 9); ++x) {
        Board<N> buttons(x);
        Board<N> lights = ButtonsToLights<3>(buttons);
        Board<N> buttonsResult = LightsToButtons<3>(lights);
        assert(buttons == buttonsResult);
    }

    TestAssumption();

    return 0;
}
