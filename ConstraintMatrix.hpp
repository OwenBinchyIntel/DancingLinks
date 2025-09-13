#include <array>
#include <cassert>
#include <cstdint>
#include <unordered_set>
#include <vector>

#include <functional>

class Instrumentation {
public:
    void SetDepth(int depth) {
        m_Depth = depth;
        if (m_Depth + 1 > m_Updates.size()) {
            m_Updates.resize(m_Depth + 1);
            m_NodesVisited.resize(m_Depth + 1);
        }
    }
    void NodeVisited() { ++m_NodesVisited[m_Depth]; }
    void Update() { ++m_Updates[m_Depth]; }
    void PrintResults() {
        int total = 0;
        std::int64_t total_updates = 0;
        std::cout << "Level\tNodes\tUpdates\tUpdates per Node\n";
        for (int i = 0; i < m_Updates.size(); ++i) {
            std::cout << i << '\t' << m_NodesVisited[i] << '\t' << m_Updates[i] << '\t'
                      << float(m_Updates[i]) / m_NodesVisited[i] << '\n';
            total += m_NodesVisited[i];
            total_updates += m_Updates[i];
        }
        std::cout << "Total\t" << total << '\t' << total_updates << '\t' << float(total_updates) / total << '\n';
    }
    void Reset() {
        m_Depth = 0;
        m_Updates = {0};
        m_NodesVisited = {0};
    }

private:
    int m_Depth = 0;
    std::vector<std::int64_t> m_Updates{0};
    std::vector<int> m_NodesVisited{0};
};

Instrumentation g_Instrumentation;

class Header;
class Node {
public:
    inline void Remove();
    inline void Restore();

    Header *col;

    Node *left;
    Node *right;
    Node *up;
    Node *down;
};

class Header : public Node {
public:
    void Remove() {
        right->left = left;
        left->right = right;
        g_Instrumentation.Update();
    }
    void Restore() {
        left->right = this;
        right->left = this;
    }
    void Cover() {
        // Remove c from header list
        Remove();
        // Remove all rows from c from other columns that they are in
        for (Node *i = down; i != this; i = i->down) {
            for (Node *j = i->right; j != i; j = j->right) {
                j->Remove();
            }
        }
    }
    void UnCover() {
        // Reverse operation of cover
        for (Node *i = up; i != this; i = i->up) {
            for (Node *j = i->left; j != i; j = j->left) {
                j->Restore();
            }
        }
        Restore();
    }

    void Append(Node *node) {
        // Insert node into column (at lowest position)
        node->col = this;
        node->down = this;
        node->up = up;
        up->down = node;
        up = node;
        ++count;
    }

    int count;
    std::size_t ix;     // DEBUG
};

inline void Node::Remove() {
    up->down = down;
    down->up = up;
    --col->count;
    assert(col->count >= 0);
    g_Instrumentation.Update();
}

inline void Node::Restore() {
    ++col->count;
    down->up = this;
    up->down = this;
}

// TODO - Replace NumConstraints with MaxConstraints
// template <std::size_t t_Constraints, std::size_t t_MaxNodes, std::size_t t_OptionalConstraints = 0>
template <std::size_t t_MaxConstraints, std::size_t t_MaxNodes>
class ConstraintMatrix {
private:
    // static constexpr int k_NumConstraints{t_Constraints + t_OptionalConstraints};

    using PrintFunctionType = std::function<void(std::vector<std::vector<std::size_t>>)>;

public:
    ConstraintMatrix(std::size_t constraints, std::size_t optionalConstraints = 0) 
    // : m_NumConstraints = constraints
    // , m_OptionalConstraints = optionalConstraints
    : m_NumReqConstraints(constraints)
    , m_NumOptConstraints(optionalConstraints)
    , m_NumTotalConstraints(constraints + optionalConstraints)
    { ConnectColHeaders(); }

    void SetPrintFunction(PrintFunctionType printFunc) { m_PrintFunction = printFunc; }

    int Solutions(int depth = 0) {
        if (depth == 0) {
            g_Instrumentation.Reset();
        }

        // Check if already satisfied.
        if (m_RootNode.right == &m_RootNode) {
            PrintSolution();
            return 1;
        }

#if 1
        // Find constraint (col) with fewest possibilities
        Header *bestCol = nullptr;
        int fewestPossibilities = t_MaxNodes;
        for (Header *colH = reinterpret_cast<Header *>(m_RootNode.right); colH != &m_RootNode;
             colH = reinterpret_cast<Header *>(colH->right)) {
            if (colH->count < fewestPossibilities) {
                fewestPossibilities = colH->count;
                bestCol = colH;
            }
        }
        assert(fewestPossibilities >= 0);
#else
        // Set bestCol to the first col right of root node.
        Header *bestCol = reinterpret_cast<Header *>(m_RootNode.right);
#endif

#if 0
        // This does not modify the algorithm. We will also find 0 solutions if we proceed.
        // It does decrease the number of "Updates" measured in finding solutions.
        // It is hard to know if this is grants an improvement on performance.
        if (bestCol->count == 0) {
            // Unsatisfiable
            return 0;
        }
#endif

        // PrintColCounts();
        // std::cout << "Covering col " << bestCol->ix << " which contains " << bestCol->count << " nodes\n";

        // Consider constraint satisfied and iterate through its possibilities.
        g_Instrumentation.SetDepth(depth);
        bestCol->Cover();

        // PrintColCounts();
        // getchar();

        int solutions = 0;
        for (Node *r = bestCol->down; r != bestCol; r = r->down) {
            Select(r);
            solutions += Solutions(depth + 1);
            g_Instrumentation.SetDepth(depth);
            UnSelect(r);
        }
        bestCol->UnCover();

        if (depth == 0) {
            // g_Instrumentation.PrintResults();
        }

        return solutions;
    }

    void AddPossibility(const std::vector<int> &constraints) {
        Node *rightmost = nullptr;
        for (int cix : constraints) {
#if 1
            // Check for duplciates
            std::unordered_set<int> seen;
            for (int num : constraints) {
                if (seen.find(num) != seen.end()) {
                    // Found a duplicate
                    assert(false);
                }
                seen.insert(num);
            }
#endif
            assert(m_NodeCount < t_MaxNodes);
            assert(cix < m_NumTotalConstraints);

            Node *node = &m_Nodes[m_NodeCount++];
            m_Headers[cix].Append(node);

            // if (cix == 62) {
            //     static int count = 0;
            //     ++count;
            //     std::cout << "62 has " << count << " constraints.\n";
            // }

            // Connect node to neighbours left & right
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
            rightmost = node;
        }
    }

    void RemoveConstraint(int cix) { m_Headers[cix].Remove(); }

    bool SanityCheck() const {
        bool noEmptyCols = true;
        for (Node *hn = m_RootNode.right; hn != &m_RootNode; hn = hn->right) {
            Header *h = reinterpret_cast<Header *>(hn);
            if (h->count == 0) {
                noEmptyCols = false;
                std::cout << h->ix << ' ';
            }
        }
        if (!noEmptyCols) {
            std::cout << '\n';
        }
        return noEmptyCols;
    }
    void PrintColCounts() const {
        int colCount = 0, total = 0;
        for (Node *hn = m_RootNode.right; hn != &m_RootNode; hn = hn->right) {
            Header *h = reinterpret_cast<Header *>(hn);
            std::cout << h->ix << '\t' << h->count << '\n';
            ++colCount;
            total += h->count;
        }
        std::cout << "Cols  : " << colCount << '\n';
        std::cout << "Total : " << total << '\n';
    }

private:
    void Select(Node *n) {
        m_Solution.push_back(n);
        for (Node *j = n->right; j != n; j = j->right) {
            j->col->Cover();
        }
        g_Instrumentation.NodeVisited();
    }

    void UnSelect(Node *n) {
        for (Node *j = n->left; j != n; j = j->left) {
            j->col->UnCover();
        }
        m_Solution.pop_back();
    }

    void ConnectColHeaders() {
        // Connect root node.
        m_RootNode.right = &m_Headers[0];
        m_RootNode.right->left = &m_RootNode;
        m_RootNode.left = &m_Headers[m_NumReqConstraints - 1];
        m_RootNode.left->right = &m_RootNode;

        for (std::size_t i = 0; i < m_NumReqConstraints; ++i) {
            // Add rightwards connection
            if (i < m_NumReqConstraints - 1) {
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

        for (std::size_t i = m_NumReqConstraints; i < m_NumTotalConstraints; ++i) {
            // Connect each direction to self
            m_Headers[i].up = &m_Headers[i];
            m_Headers[i].down = &m_Headers[i];
            m_Headers[i].left = &m_Headers[i];
            m_Headers[i].right = &m_Headers[i];
            m_Headers[i].ix = i;
        }
    }

    void PrintSolution() const {
        if (m_PrintFunction) {
            std::vector<std::vector<std::size_t>> selections;
            for (const auto &node : m_Solution) {
                selections.push_back({node->col->ix});
                for (Node *r = node->right; r != node; r = r->right) {
                    selections.back().push_back(r->col->ix);
                }
            }
            m_PrintFunction(selections);
        }
        return;
    }

private:
    Node m_RootNode;
    std::array<Header, t_MaxConstraints> m_Headers;
    std::array<Node, t_MaxNodes> m_Nodes;
    int m_NodeCount = 0;
    // const std::size_t m_NumConstraints;
    // const std::size_t m_OptionalConstraints;
    
    const std::size_t m_NumReqConstraints;
    const std::size_t m_NumOptConstraints;
    const std::size_t m_NumTotalConstraints;

    std::vector<Node *> m_Solution;

    PrintFunctionType m_PrintFunction;
};
