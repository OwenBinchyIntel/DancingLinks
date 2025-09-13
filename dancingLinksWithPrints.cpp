#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

//          RC -------------------------    R# -------------------------    C# -------------------------
//          0       1       2       3       4       5       6       7       8       9       10      11  
//          R1C1    R1C2    R2C1    R2C2    R1#1    R1#2    R2#1    R2#2    C1#1    C1#2    C2#1    C2#2
// R1C1#1   1       0       0       0       1       0       0       0       1       0       0       0
// R1C1#2   1       0       0       0       0       1       0       0       0       1       0       0
// R1C2#1   0       1       0       0       1       0       0       0       0       0       1       0
// R1C2#2   0       1       0       0       0       1       0       0       0       0       0       1
// R2C1#1   0       0       1       0       0       0       1       0       1       0       0       0
// R2C1#2   0       0       1       0       0       0       0       1       0       1       0       0
// R2C2#1   0       0       0       1       0       0       1       0       0       0       1       0
// R2C2#2   0       0       0       1       0       0       0       1       0       0       0       1

template<std::size_t t_Width>
class ConstraintMatrix {

private:
    static constexpr int k_MaxPosibilities{t_Width * t_Width * t_Width};
    static constexpr int k_RCConstraints{t_Width * t_Width};
    static constexpr int k_RNConstriants{t_Width * t_Width};
    static constexpr int k_CNConstraints{t_Width * t_Width};
    static constexpr int k_MaxConstraints{k_RCConstraints + k_RNConstriants + k_CNConstraints};
    static constexpr int k_MaxNodes = k_MaxConstraints * t_Width;

public:
    struct Header;
    struct Node {
        Node *left{nullptr};
        Node *right{nullptr};
        Node *up{nullptr};
        Node *down{nullptr};

        Header *col{nullptr};
    };

    struct Header : public Node {
        std::string name;
        int ix;
        int count;
    };

    ConstraintMatrix() {
        ConnectColHeaders();
        PopulateGraph();
    }

    int Solutions() {
        if (m_RootNode.right == &m_RootNode) {
            PrintSolution();
            return 1;
        }

        int lowestPossibilities = k_MaxPosibilities + 1;
        Header* lowestCol = nullptr;

        // Find constraint with fewest possibilities
        for (Node* colH = m_RootNode.right; colH != &m_RootNode; colH = colH->right) {
            Header* h = reinterpret_cast<Header*>(colH);
            if (h->count < lowestPossibilities) {
                lowestPossibilities = h->count;
                lowestCol = h;
            }
        }

        if (lowestPossibilities == 0) {
            // Unsatisfiable
            return 0;
        }

        Cover(lowestCol);

        int solutions = 0;
        // Iterate through possibilities for given constraint
        for (Node* r = lowestCol->down; r != lowestCol; r = r->down) {
            // Select the given possibility
            Select(r);

            // Accumalate the new possiblities
            solutions += Solutions();

            // Unselect the given possibility
            UnSelect(r);
        }

        UnCover(lowestCol);
        return solutions;
    }

    bool Set(int row, int col, int num) {
        --num;
        if (row < 0 || row >= t_Width || col < 0 || col >= t_Width || num < 0 || num >= t_Width) {
            return false;
        }
        int rci = (row * t_Width) + col;
        int rni = (t_Width * t_Width) + (row * t_Width) + num;

        for (Node* h = m_RootNode.right; h != &m_RootNode; h = h->right) {
            Header* col = reinterpret_cast<Header*>(h);
            if (col->ix == rci) {
                for (Node* n = h->down; n != h; n = n->down) {
                    if (n->right->col->ix == rni) {
                        Cover(n->col);
                        Select(n);
                        return true;
                    }
                }

            }
        }
        return false;
    }

    void Print() const {
        auto GetNode = [&](int row, int col) -> const Node * {
            Node *down = m_Headers[col].down;
            Node *leftmost = m_LeftmostNode[row];
            if (!leftmost) {
                return nullptr;
            }
            while (down != &m_Headers[col]) {
                Node* right = leftmost;
                do {
                    if (down == right) {
                        return down;
                    }
                    right = right->right;
                } while(right != leftmost);
                down = down->down;
            }
            return nullptr;
        };

        int rows = 0, cols = 0, nodes = 0;
        for (int i = 0; i < k_MaxPosibilities; ++i) {
            Node* left = m_LeftmostNode[i];
            // if (!left) {
            //     continue;
            // }
            ++rows;
            Node* col = m_RootNode.right;
            cols = 0;
            for (int j = 0; j < k_MaxConstraints; ++j) {
                // if (col != &m_Headers[j]) {
                //     continue;
                // }
                ++cols;
                col = col->right;
                const Node *currentNode = GetNode(i, j);
                if (currentNode != nullptr) {
                    std::cout << "1 ";
                    ++nodes;
                } else {
                    std::cout << "0 ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "Rows : " << rows << "\tCols : " << cols << "\tNodes : " << nodes << '\n';
    }

    void PrintLatinSquare() const {
        int grid[t_Width][t_Width];

        for (int i = 0; i < m_Selections; ++i) {
            // Get Row
            int rci = k_MaxConstraints;
            Node* right = m_O[i];
            Node* leftmost = nullptr;
            do {
                if (right->col->ix < rci) {
                    rci = right->col->ix;
                    leftmost = right;
                }
                right = right->right;
            } while (right != m_O[i]);

            int row = rci / t_Width;

            // Get Col
            int col = rci % t_Width;

            // Get Num
            int rni = leftmost->right->col->ix;
            int num = rni % t_Width + 1;

            // Fill grid
            grid[row][col] = num;
        }

        // Print grid
        for (int i = 0; i < t_Width; ++i) {
            for (int j = 0; j < t_Width; ++j) {
                std::cout << grid[i][j] << ' ';
            }
            std::cout << '\n';
        }
    }

    void PrintSolution() const {
        auto PrintRow = [&](Node* row) {
            Node* right = row;
            do {
                std::cout << right->col->name << ' ';
                right = right->right;
            } while (right != row);
        };

        for (int i = 0; i < m_Selections; ++i) {
            PrintRow(m_O[i]);
            std::cout << '\n';
        }

        std::cout << '\n';

        PrintLatinSquare();

        std::cout << std::endl;
    }

private:
    void Cover(Header* c) {
        // Remove c from header list
        c->right->left = c->left;
        c->left->right = c->right;
        // Remove all rows from c from other columns that they are in
        for (Node* i = c->down; i != c; i = i->down) {
            for (Node* j = i->right; j != i; j = j->right) {
                j->up->down = j->down;
                j->down->up = j->up;
                --j->col->count;
            }
        }
    }

    void UnCover(Header* c) {
        // Reverse operation of cover
        for (Node* i = c->up; i != c; i = i->up) {
            for (Node* j = i->left; j != i; j = j->left) {
                ++j->col->count;
                j->down->up = j;
                j->up->down = j;
            }
        }
        c->left->right = c;
        c->right->left = c;
    }

    void Select(Node* n) {
        m_O[m_Selections] = n;
        for (Node* j = n->right; j != n; j = j->right) {
            Cover(j->col);
        }   
        ++m_Selections; 
    }

    

    void UnSelect(Node* n) {
        for (Node* j = n->left; j != n; j = j->left) {
            UnCover(j->col);
        }
        --m_Selections;
    }

    void ConnectColHeaders() {
        for (int i = 0; i < m_Headers.size(); ++i) {
            // Left/right will wrap
            std::size_t lix = (m_Headers.size() + i - 1) % m_Headers.size();
            std::size_t rix = (m_Headers.size() + i + 1) % m_Headers.size();
            m_Headers[i].left = &m_Headers[lix];
            m_Headers[i].right = &m_Headers[rix];

            // Connect up/down to self
            m_Headers[i].up = &m_Headers[i];
            m_Headers[i].down = &m_Headers[i];

            // Initialize count
            m_Headers[i].count = 0;
            m_Headers[i].name = std::to_string(i);
            m_Headers[i].ix = i;
        }

        // Connect root node.
        m_RootNode.right = &m_Headers.front();
        m_Headers.front().left = &m_RootNode;
        m_RootNode.left = &m_Headers.back();
        m_Headers.back().right = &m_RootNode;
    }

    void PopulateGraph() {
        std::array<Node*, k_MaxPosibilities> rightmostNodes{nullptr};
        int constraint = 0;
        // RC
        for (int r = 0; r < t_Width; ++r) {
            for (int c = 0; c < t_Width; ++c) {
                for (int n = 0; n < t_Width; ++n) {
                    int possibility = r * t_Width * t_Width + c * t_Width + n;
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
        // RN
        for (int r = 0; r < t_Width; ++r) {
            for (int n = 0; n < t_Width; ++n) {
                for (int c = 0; c < t_Width; ++c) {
                    int possibility = r * t_Width * t_Width + c * t_Width + n;
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
        // CN
        for (int c = 0; c < t_Width; ++c) {
            for (int n = 0; n < t_Width; ++n) {
                for (int r = 0; r < t_Width; ++r) {
                    int possibility = r * t_Width * t_Width + c * t_Width + n;
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
    }

    void CreateNewNode(int possibility, int constraint, std::array<Node*, k_MaxPosibilities> &rightmostNodes) {
        Node *node = &m_Nodes[m_NodeCount++];

        node->col = &m_Headers[constraint];
        node->down = node->col;
        node->up = node->col->up;
        node->down->up = node;
        node->up->down = node;
        ++node->col->count;

        Node* rightmost = rightmostNodes[possibility];
        if (rightmost) {
            Node *leftmost = rightmost->right;
            node->left = rightmost;
            node->right = leftmost;
            leftmost->left = node;
            rightmost->right = node;
        } else {
            node->left = node;
            node->right = node;
        }
        rightmostNodes[possibility] = node;

        // TODO - for now printing is easier with this:
        m_LeftmostNode[possibility] = node->right;
    }


private:
    std::array<Node*, k_MaxPosibilities> m_LeftmostNode;

    std::array<Header, k_MaxConstraints> m_Headers;
    std::array<Node, k_MaxNodes> m_Nodes;
    int m_NodeCount = 0;

    static constexpr int k_MaxSelections = t_Width * t_Width;

    Node m_RootNode;

    std::array<Node*, k_MaxSelections> m_O {nullptr};
    int m_Selections = 0;
};

int main() {
    ConstraintMatrix<2> cm;

    // cm.Print();

    // const auto time_s = std::chrono::high_resolution_clock::now();
    // int solutions = cm.Solutions();
    // const auto time_e = std::chrono::high_resolution_clock::now();
    // const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    // std::cout << "Total solutions : " << solutions << '\n';
    // std::cout << "Search took : " << time_ms << "ms\n";

    // cm.Select(cm.m_RootNode.right->down);
    if (!cm.Set(0, 0, 2)) {
        std::cerr << "Failed to set\n";
    }
    if (!cm.Set(1, 1, 2)) {
        std::cerr << "Failed to set\n";
    }

    int solutions = cm.Solutions();
    std::cout << "Solutions after selection : " << solutions << '\n';

    return 0;
}
