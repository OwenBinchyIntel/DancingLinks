#include <algorithm>
#include <chrono>
#include <iostream>

#include "ConstraintMatrix.hpp"

namespace {

/*
Constraints:
    Each cell must contain exactly one letter
    Each row must be a word
    Each col must be a word

Example with alphabet = "acent"
and only showing possibilities which fill the solution:
cat
ace
ten

(note vertical cells are labelled differently here)

horizontal                                            vertical
1     2     3     4     5     6     7     8     9     1     2     3     4     5     6     7     8     9
acent acent acent acent acent acent acent acent acent acent acent acent acent acent acent acent acent acent
-----------------------------------------------------------------------------------------------------------
 x    x         x                                     x xxx              xxxx             xxxx
                  x      x      x                            xxxx             x xxx             xx xx
                                        x   x      x              xxxx              xx xx             xxx x
x xxx              xxxx             xxxx               x    x         x
       xxxx             x xxx             xx xx                         x      x      x
            xxxx              xx xx             xxx x                                         x   x      x
-----------------------------------------------------------------------------------------------------------
xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx xxxxx

Every row represents a word placed horizontally or vertically in a specific row or column; thus every word from the
dictionary will get 6 rows.

A horizontally placed word would have 1s in the columns
    Horizontal + occupied cell + letter in that cell
    Vertical + occupied cell + (every letter not in that cell)


Label cells:
0 1 2
3 4 5
6 7 8

*/

std::vector<std::string> g_Dictionary = {
    "ABA", "ABS", "ACE", "ACT", "ADD", "ADO", "AFT", "AGE", "AGO", "AHA", "AID", "AIM", "AIR", "ALA", "ALE", "ALL",
    "ALT", "AMP", "ANA", "AND", "ANT", "ANY", "APE", "APP", "APT", "ARC", "ARE", "ARK", "ARM", "ART", "ASH", "ASK",
    "ASP", "ASS", "ATE", "AVE", "AWE", "AXE", "AYE", "BAA", "BAD", "BAG", "BAN", "BAR", "BAT", "BAY", "BED", "BEE",
    "BEG", "BEL", "BEN", "BET", "BID", "BIG", "BIN", "BIO", "BIS", "BIT", "BIZ", "BOB", "BOG", "BOO", "BOW", "BOX",
    "BOY", "BRA", "BUD", "BUG", "BUM", "BUN", "BUS", "BUT", "BUY", "BYE", "CAB", "CAD", "CAM", "CAN", "CAP", "CAR",
    "CAT", "CHI", "COB", "COD", "COL", "CON", "COO", "COP", "COR", "COS", "COT", "COW", "COX", "COY", "CRY", "CUB",
    "CUE", "CUM", "CUP", "CUT", "DAB", "DAD", "DAL", "DAM", "DAN", "DAY", "DEE", "DEF", "DEL", "DEN", "DEW", "DID",
    "DIE", "DIG", "DIM", "DIN", "DIP", "DIS", "DOC", "DOE", "DOG", "DON", "DOT", "DRY", "DUB", "DUE", "DUG", "DUN",
    "DUO", "DYE", "EAR", "EAT", "EBB", "ECU", "EFT", "EGG", "EGO", "ELF", "ELM", "EMU", "END", "ERA", "ETA", "EVE",
    "EYE", "FAB", "FAD", "FAN", "FAR", "FAT", "FAX", "FAY", "FED", "FEE", "FEN", "FEW", "FIG", "FIN", "FIR", "FIT",
    "FIX", "FLU", "FLY", "FOE", "FOG", "FOR", "FOX", "FRY", "FUN", "FUR", "GAG", "GAL", "GAP", "GAS", "GAY", "GEE",
    "GEL", "GEM", "GET", "GIG", "GIN", "GOD", "GOT", "GUM", "GUN", "GUT", "GUY", "GYM", "HAD", "HAM", "HAS", "HAT",
    "HAY", "HEM", "HEN", "HER", "HEY", "HID", "HIM", "HIP", "HIS", "HIT", "HOG", "HON", "HOP", "HOT", "HOW", "HUB",
    "HUE", "HUG", "HUH", "HUM", "HUT", "ICE", "ICY", "IGG", "ILL", "IMP", "INK", "INN", "ION", "ITS", "IVY", "JAM",
    "JAR", "JAW", "JAY", "JET", "JEW", "JOB", "JOE", "JOG", "JOY", "JUG", "JUN", "KAY", "KEN", "KEY", "KID", "KIN",
    "KIT", "LAB", "LAC", "LAD", "LAG", "LAM", "LAP", "LAW", "LAX", "LAY", "LEA", "LED", "LEE", "LEG", "LES", "LET",
    "LIB", "LID", "LIE", "LIP", "LIT", "LOG", "LOT", "LOW", "MAC", "MAD", "MAG", "MAN", "MAP", "MAR", "MAS", "MAT",
    "MAX", "MAY", "MED", "MEG", "MEN", "MET", "MID", "MIL", "MIX", "MOB", "MOD", "MOL", "MOM", "MON", "MOP", "MOT",
    "MUD", "MUG", "MUM", "NAB", "NAH", "NAN", "NAP", "NAY", "NEB", "NEG", "NET", "NEW", "NIL", "NIP", "NOD", "NOR",
    "NOS", "NOT", "NOW", "NUN", "NUT", "OAK", "ODD", "OFF", "OFT", "OIL", "OLD", "OLE", "ONE", "OOH", "OPT", "ORB",
    "ORE", "OUR", "OUT", "OWE", "OWL", "OWN", "PAC", "PAD", "PAL", "PAM", "PAN", "PAP", "PAR", "PAS", "PAT", "PAW",
    "PAY", "PEA", "PEG", "PEN", "PEP", "PER", "PET", "PEW", "PHI", "PIC", "PIE", "PIG", "PIN", "PIP", "PIT", "PLY",
    "POD", "POL", "POP", "POT", "PRO", "PSI", "PUB", "PUP", "PUT", "RAD", "RAG", "RAJ", "RAM", "RAN", "RAP", "RAT",
    "RAW", "RAY", "RED", "REF", "REG", "REM", "REP", "REV", "RIB", "RID", "RIG", "RIM", "RIP", "ROB", "ROD", "ROE",
    "ROT", "ROW", "RUB", "RUE", "RUG", "RUM", "RUN", "RYE", "SAB", "SAC", "SAD", "SAE", "SAG", "SAL", "SAP", "SAT",
    "SAW", "SAY", "SEA", "SEC", "SEE", "SEN", "SET", "SEW", "SEX", "SHE", "SHY", "SIC", "SIM", "SIN", "SIP", "SIR",
    "SIS", "SIT", "SIX", "SKI", "SKY", "SLY", "SOD", "SOL", "SON", "SOW", "SOY", "SPA", "SPY", "SUB", "SUE", "SUM",
    "SUN", "SUP", "TAB", "TAD", "TAG", "TAM", "TAN", "TAP", "TAR", "TAT", "TAX", "TEA", "TED", "TEE", "TEN", "THE",
    "THY", "TIE", "TIN", "TIP", "TOD", "TOE", "TOM", "TON", "TOO", "TOP", "TOR", "TOT", "TOW", "TOY", "TRY", "TUB",
    "TUG", "TWO", "USE", "VAN", "VAT", "VET", "VIA", "VIE", "VOW", "WAN", "WAR", "WAS", "WAX", "WAY", "WEB", "WED",
    "WEE", "WET", "WHO", "WHY", "WIG", "WIN", "WIS", "WIT", "WON", "WOO", "WOW", "WRY", "WYE", "YEN", "YEP", "YES",
    "YET", "YOU", "ZIP", "ZOO",
};
constexpr int numWords = 500;

// std::vector<std::string> g_Dictionary = {
//     "ATE", "WIN", "LED", "BED", "OAR", "WRY", "OHM", "RUE", "BET", "PEA", "URN", "BAY", "TWO", "ION", "TEE",
//     "AWL", "TIE", "END", "BOW", "EAR", "DRY", "ORB", "HUE", "MET", "PUB", "ERA", "ANY", "TIT", "WOE", "ONE",
// };
// constexpr int numWords = 30;
// std::vector<std::string> g_Dictionary = {
//     "CAT",
//     "ACE",
//     "TEN",
// };
// constexpr int numWords = 3;

const std::string g_Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr int letters = 26;
// const std::string g_Alphabet = "ACENT";
// constexpr int letters = 5; // (int)g_Alphabet.size();
constexpr int vOff = letters * 3 * 3;
ConstraintMatrix<18 * letters, numWords * 6 * 3 * letters, 0> g_ConstraintMatrix;

int CIX(char c) {
    for (int i = 0; i < g_Alphabet.size(); ++i) {
        if (g_Alphabet[i] == c) {
            return i;
        }
    }
    return -1;
}

void PrintFunction(std::vector<std::vector<int>> selections) {
    std::cout << "Solution found :\n";

    std::array<std::array<int, 3>, 3> grid;

    for (auto &selection : selections) {
        assert(selection.size() == letters * 3);
        std::sort(selection.begin(), selection.end());
        // for (int i : selection) {
        //     std::cout << i << ' ';
        // }
        // std::cout << '\n';
        // Determine if horizontally or vertically placed cell:
        if (selection[3] >= vOff) {
            // Horizontal
            int row = selection[0] / (3 * letters);
            grid[row][0] = selection[0] % letters;
            grid[row][1] = selection[1] % letters;
            grid[row][2] = selection[2] % letters;
        } else {
            continue;
            // Vertical
            int col = (selection[12] - vOff) / (1 * letters);
            grid[0][col] = selection[12] % letters;
            grid[1][col] = selection[13] % letters;
            grid[2][col] = selection[14] % letters;
        }
    }

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            std::cout << g_Alphabet[grid[row][col]];
        }
        std::cout << '\n';
    }

    // Only print one
    // static int count = 0;
    // if (++count > 10) {
        // g_ConstraintMatrix.SetPrintFunction(nullptr);
    // }
}

}; // namespace

int main() {
    // Populate constraint matrix
    for (const auto &word : g_Dictionary) {
        // Placed horizontally
        for (int row = 0; row < 3; ++row) {
            char c1 = word[0], c2 = word[1], c3 = word[2];
            int ix1 = (letters * 3 * row) + (letters * 0);
            int ix2 = (letters * 3 * row) + (letters * 1);
            int ix3 = (letters * 3 * row) + (letters * 2);

            std::vector<int> constraints;

            constraints.push_back(ix1 + CIX(c1));
            constraints.push_back(ix2 + CIX(c2));
            constraints.push_back(ix3 + CIX(c3));

            for (const auto &letter : g_Alphabet) {
                if (letter != c1) {
                    constraints.push_back(vOff + ix1 + CIX(letter));
                }
                if (letter != c2) {
                    constraints.push_back(vOff + ix2 + CIX(letter));
                }
                if (letter != c3) {
                    constraints.push_back(vOff + ix3 + CIX(letter));
                }
            }

            g_ConstraintMatrix.AddPossibility(constraints);
        }
        // Placed vertically
        for (int col = 0; col < 3; ++col) {
            char c1 = word[0], c2 = word[1], c3 = word[2];
            int ix1 = (letters * col) + (letters * 0);
            int ix2 = (letters * col) + (letters * 3);
            int ix3 = (letters * col) + (letters * 6);

            std::vector<int> constraints;

            constraints.push_back(vOff + ix1 + CIX(c1));
            constraints.push_back(vOff + ix2 + CIX(c2));
            constraints.push_back(vOff + ix3 + CIX(c3));

            for (const auto &letter : g_Alphabet) {
                if (letter != c1) {
                    constraints.push_back(ix1 + CIX(letter));
                }
                if (letter != c2) {
                    constraints.push_back(ix2 + CIX(letter));
                }
                if (letter != c3) {
                    constraints.push_back(ix3 + CIX(letter));
                }
            }

            g_ConstraintMatrix.AddPossibility(constraints);
        }
    }

    // g_ConstraintMatrix.SetPrintFunction(PrintFunction);

    // Solve
    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = g_ConstraintMatrix.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
