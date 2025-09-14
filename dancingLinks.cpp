#include <array>
#include <iostream>
#include <string>

#include <chrono>
#include <vector>

class ConstraintMatrix {
private:
    static constexpr int k_MaxWidth = 9;
    static constexpr int k_MaxPosibilities{k_MaxWidth * k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxRCConstraints{k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxRNConstriants{k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxCNConstraints{k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxConstraints{k_MaxRCConstraints + k_MaxRNConstriants + k_MaxCNConstraints};
    static constexpr int k_MaxNodes = k_MaxConstraints * k_MaxWidth;
    static constexpr int k_MaxSelections = k_MaxWidth * k_MaxWidth;

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
        int ix;
        int count;
    };

    ConstraintMatrix(int width)
        : m_Width{width} {
        ConnectColHeaders();
        PopulateGraph();
    }

    int Solutions() {
        if (m_RootNode.right == &m_RootNode) {
            return 1;
        }

        int lowestPossibilities = k_MaxPosibilities + 1;
        Header *lowestCol = nullptr;

        // Find constraint with fewest possibilities
        for (Header *colH = reinterpret_cast<Header *>(m_RootNode.right); colH != &m_RootNode;
             colH = reinterpret_cast<Header *>(colH->right)) {
            if (colH->count < lowestPossibilities) {
                lowestPossibilities = colH->count;
                lowestCol = colH;
            }
        }

        if (lowestPossibilities == 0) {
            // Unsatisfiable
            return 0;
        }

        // Consider constraint satisfied and iterate through its possibilities.
        Cover(lowestCol);
        int solutions = 0;
        for (Node *r = lowestCol->down; r != lowestCol; r = r->down) {
            Select(r);
            solutions += Solutions();
            UnSelect(r);
        }

        UnCover(lowestCol);
        return solutions;
    }

    bool Set(int row, int col, int num) {
        --num;
        if (row < 0 || row >= m_Width || col < 0 || col >= m_Width || num < 0 || num >= m_Width) {
            return false;
        }
        int rci = (row * m_Width) + col;
        int rni = (m_Width * m_Width) + (row * m_Width) + num;

        for (Node *h = m_RootNode.right; h != &m_RootNode; h = h->right) {
            Header *col = reinterpret_cast<Header *>(h);
            if (col->ix == rci) {
                for (Node *n = h->down; n != h; n = n->down) {
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

private:
    void Cover(Header *c) {
        // Remove c from header list
        c->right->left = c->left;
        c->left->right = c->right;
        // Remove all rows from c from other columns that they are in
        for (Node *i = c->down; i != c; i = i->down) {
            for (Node *j = i->right; j != i; j = j->right) {
                j->up->down = j->down;
                j->down->up = j->up;
                --j->col->count;
            }
        }
    }

    void UnCover(Header *c) {
        // Reverse operation of cover
        for (Node *i = c->up; i != c; i = i->up) {
            for (Node *j = i->left; j != i; j = j->left) {
                ++j->col->count;
                j->down->up = j;
                j->up->down = j;
            }
        }
        c->left->right = c;
        c->right->left = c;
    }

    void Select(Node *n) {
        for (Node *j = n->right; j != n; j = j->right) {
            Cover(j->col);
        }
    }

    void UnSelect(Node *n) {
        for (Node *j = n->left; j != n; j = j->left) {
            UnCover(j->col);
        }
    }

    void ConnectColHeaders() {
        // Connect root node.
        m_RootNode.right = &m_Headers[0];
        m_RootNode.right->left = &m_RootNode;
        m_RootNode.left = &m_Headers[m_NumConstraints - 1];
        m_RootNode.left->right = &m_RootNode;

        for (int i = 0; i < m_NumConstraints; ++i) {
            // Add rightwards connection
            if (i < m_NumConstraints - 1) {
                m_Headers[i].right = &m_Headers[i + 1];
                m_Headers[i].right->left = &m_Headers[i];
            }

            // Connect up/down to self
            m_Headers[i].up = &m_Headers[i];
            m_Headers[i].down = &m_Headers[i];

            // Initialize header fields
            m_Headers[i].count = 0;
            m_Headers[i].ix = i;
        }
    }

    int Possibility(int row, int col, int num) const { return (row * m_Width * m_Width) + (col * m_Width) + num; }

    void PopulateGraph() {
        std::array<Node *, k_MaxPosibilities> rightmostNodes{nullptr};
        int constraint = 0;
        // RC
        for (int r = 0; r < m_Width; ++r) {
            for (int c = 0; c < m_Width; ++c) {
                for (int n = 0; n < m_Width; ++n) {
                    int possibility = Possibility(r, c, n);
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
        // RN
        for (int r = 0; r < m_Width; ++r) {
            for (int n = 0; n < m_Width; ++n) {
                for (int c = 0; c < m_Width; ++c) {
                    int possibility = Possibility(r, c, n);
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
        // CN
        for (int c = 0; c < m_Width; ++c) {
            for (int n = 0; n < m_Width; ++n) {
                for (int r = 0; r < m_Width; ++r) {
                    int possibility = Possibility(r, c, n);
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
    }

    void CreateNewNode(int possibility, int constraint, std::array<Node *, k_MaxPosibilities> &rightmostNodes) {
        Node *node = &m_Nodes[m_NodeCount++];

        // Insert node into column (at lowest position)
        node->col = &m_Headers[constraint];
        node->down = node->col;
        node->up = node->col->up;
        node->down->up = node;
        node->up->down = node;
        ++node->col->count;

        // Connect node to neighbours left & right
        Node *rightmost = rightmostNodes[possibility];
        if (rightmost) {
            Node *leftmost = rightmost->right;
            node->left = rightmost;
            node->right = leftmost;
            leftmost->left = node;
            rightmost->right = node;
        } else {
            // No other entries on given row, connect to self
            node->left = node;
            node->right = node;
        }
        rightmostNodes[possibility] = node;
    }

private:
    const int m_Width;
    const int m_NumConstraints{3 * m_Width * m_Width};

    Node m_RootNode;
    std::array<Header, k_MaxConstraints> m_Headers;
    std::array<Node, k_MaxNodes> m_Nodes;
    int m_NodeCount = 0;
};

#if defined(_WIN32)
int main() {
    int n = 9;

    for (int iter = 0; iter < 100; ++iter) {
        // std::vector<std::vector<int>> init_grid = {
        //     {1, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 2, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 3, 0, 0, 0, 0, 0, 0},
        //     {0, 0, 0, 4, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 5, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 6, 0, 0, 0},
        //     {0, 0, 0, 0, 0, 0, 7, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 8, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 9},
        // };

        std::vector<std::vector<int>> init_grid = {
            {0, 8, 7, 6, 0, 4, 3, 2, 1}, {8, 0, 0, 0, 0, 0, 0, 0, 2}, {7, 0, 0, 0, 0, 0, 0, 0, 3},
            {6, 0, 0, 5, 2, 3, 0, 0, 0}, {0, 0, 0, 3, 0, 2, 0, 0, 5}, {4, 0, 1, 2, 3, 0, 0, 0, 6},
            {3, 0, 0, 0, 0, 0, 0, 0, 7}, {2, 0, 0, 0, 0, 0, 0, 0, 8}, {1, 0, 3, 0, 5, 0, 7, 8, 0},
        };

        const auto time_start = std::chrono::high_resolution_clock::now();

        ConstraintMatrix cm(n);
        for (int y = 0; y < n; ++y) {
            // std::string row;
            // getline(std::cin, row);
            for (int x = 0; x < n; ++x) {
                // grid.Set(Position{ x, y }, row[x] - '0');
                if (init_grid[y][x] != 0) {
                    if (!cm.Set(x, y, init_grid[y][x])) {
                        // Invalid grid
                        std::cout << "0\n";
                        return 0;
                    }
                }
            }
        }

        std::cout << cm.Solutions() << '\n';

        const auto time_end = std::chrono::high_resolution_clock::now();
        const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
        std::cerr << "Time : " << time_ms << "ms\n";
    }

    return 0;
}

#else

int main() {
    int n;
    std::cin >> n;
    std::cin.ignore();

    ConstraintMatrix cm(n);

    for (int y = 0; y < n; ++y) {
        std::string row;
        getline(std::cin, row);
        for (int x = 0; x < n; ++x) {
            int val = row[x] - '0';
            if (val != 0) {
                if (!cm.Set(x, y, val)) {
                    // Invalid grid
                    std::cout << "0\n";
                    return 0;
                }
            }
        }
    }

    std::cout << cm.Solutions() << '\n';

    return 0;
}
#endif