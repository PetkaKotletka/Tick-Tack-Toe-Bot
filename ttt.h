#include <algorithm>
#include <vector>
#include <random>
#include <queue>

const int ALPH_SIZE = 3;

struct node {
    int counter = 0;
    int parentIdx = 0, patternIdx = 0;
    bool terminal = false;
    node* go[ALPH_SIZE];
    node* link;
    node* parent;
};

struct pattern {
    std::vector<int> data;
    int once, more;
};

struct moveWithEval {
    std::pair<int, int> move;
    int eval;
};

int dist(std::pair<int, int>& a, std::pair<int, int>& b) {
    return std::min(std::abs(a.first - b.first), std::abs(a.second - b.second));
}

class Automatum {
public:
    Automatum() = default;
    Automatum(const std::vector<pattern>& patterns) {
        root = new node();
        terminalNodes = 0;
        buildTrie(patterns);
        buildAuto();
    }

    int getTerminalNodes() {
        return terminalNodes;
    }

    void processText(std::vector<std::vector<int>>& board, 
                     int startX, int startY, 
                     int finishX, int finishY,
                     int incX, int incY,
                     bool leqX, bool leqY) { // true: coord <= finish
        node* v = root;
        for (int x = startX, y = startY;
            !(leqX ^ (x <= finishX)) && !(leqY ^ (y <= finishY)); 
             x += incX, y += incY) {
            v = v->go[board[x][y]];
            v->counter++;
        }
    }

    void getResults(std::vector<int>& results) {
        if (results.size() != terminalNodes) {
            results.resize(terminalNodes);
        }
        propagate();
        for (auto v : nodes) {
            if (v->terminal) {
                results[v->patternIdx] = v->counter;
            }
        }
        clear();
    }

    ~Automatum() {
        deleteTrie();
    }
private:
    node* root;
    std::vector<node*> nodes;
    int terminalNodes;

    void buildTrie(const std::vector<pattern>& patterns) {
        for (int i = 0; i < patterns.size(); ++i) {
            node* v = root;
            for (auto &branch : patterns[i].data) {
                if (v->go[branch] == nullptr) {
                    v->go[branch] = new node();
                    v->go[branch]->parent = v;
                    v->go[branch]->parentIdx = branch;
                }
                v = v->go[branch];
            }
            v->terminal = true;
            v->patternIdx = i;
            terminalNodes++;
        }
    }

    void buildAuto() {
        node* rootLink = new node();
        root->link = rootLink;
        root->parent = rootLink;
        for (int i = 0; i < ALPH_SIZE; ++i) {
            rootLink->go[i] = root;
        }
        std::queue<node*> q;
        q.push(root);
        nodes.push_back(root);
        while (!q.empty()) {
            node* v = q.front();
            q.pop();
            for (int i = 0; i < ALPH_SIZE; ++i) {
                if (v->go[i] != nullptr) {
                    q.push(v->go[i]);
                    nodes.push_back(v->go[i]);
                }
            }
            if (v != root) {
                v->link = v->parent->link->go[v->parentIdx];
            }
            for (int i = 0; i < ALPH_SIZE; ++i) {
                if (v->go[i] == nullptr) {
                    v->go[i] = v->link->go[i];
                }
            }
        }
        std::reverse(nodes.begin(), nodes.end());
    }

    void propagate() {
        for (auto v : nodes) {
            v->link->counter += v->counter;
        }
    }

    void clear() {
        for (auto v : nodes) {
            v->counter = 0;
        }
    }

    void deleteTrie() {
        delete root->link;
        for (auto v : nodes) {
            delete v;
        }
    }
};

class Game {
public:
    Game() = default;
    Game(int boardSize, int toWin): boardSize(boardSize), toWin(toWin) {
        board = std::vector<std::vector<int>>(boardSize, std::vector<int>(boardSize, 0));
        automatum = new Automatum(patterns);
        results.resize(automatum->getTerminalNodes());
        moveX = true;
        lastX = -1; lastY = -1;
        lastMinX = -1; lastMinY = -1; lastMaxX = -1; lastMaxY = -1;
        minX = boardSize; minY = boardSize;
        maxX = -1; maxY = -1;
        positionEvaluation = 0; prevPositionEvaluation = 0;
    }
    
    bool isMoveX() { 
        return moveX; 
    }

    void setMoveX(bool mv) { 
        moveX = mv; 
    }
    
    std::vector<std::vector<int>> getBoard() { 
        return board; 
    }

    void move(int x, int y) {
        if (board[x][y] == 0) {
            prevPositionEvaluation = positionEvaluation;
            positionEvaluation -= evaluateMove(x, y);
            board[x][y] = moveX ? 1 : 2;
            positionEvaluation += evaluateMove(x, y);
            moveX = !moveX;
            lastX = x;
            lastY = y;
            updateBounds();
        }
    }

    void getLastMove(int& x, int& y) {
        x = lastX;
        y = lastY;
    }

    bool checkWin() {
        if (lastX == -1) {
            return false;
        }
        automatum->processText(board, lastX - toWin, lastY, lastX + toWin, lastY, 1, 0, true, true);
        automatum->processText(board, lastX, lastY - toWin, lastX, lastY + toWin, 0, 1, true, true);
        automatum->processText(board, lastX - toWin, lastY - toWin, lastX + toWin, lastY + toWin, 1, 1, true, true);
        automatum->processText(board, lastX - toWin, lastY + toWin, lastX + toWin, lastY - toWin, 1, -1, true, false);
        automatum->getResults(results);
        return results[0] > 0 || results[1] > 0;
    }

    void machineMove() {
        Game machine(*this);
        if (!moveIfCanWin(machine)) {
            machine.setMoveX(!machine.isMoveX());
            if (!moveIfCanWin(machine)) {
                machine.setMoveX(!machine.isMoveX());
                std::pair<int, int> nextMove;
                int score, alpha = -inf, beta = inf;
                score = getBestScore(machine, 1, nextMove, alpha, beta);
                std::cout << "Best score: " << score << std::endl;
                move(nextMove.first, nextMove.second);
            }
        }
    }

private:
    const int maxDistToMove = 2, maxDistToCheck = 3;
    const int maxDepth = 4;
    const int inf = 100000;
    const int randomNoise = 5;
    const std::vector<std::pair<int, int>> group = {{inf, inf}, {1000, 1300}, {80, 180}, {60, 220}, {10, 25}}; // TODO
    const std::vector<pattern> patterns = {
        {{1, 1, 1, 1, 1}, group[0].first, group[0].second}, {{2, 2, 2, 2, 2}, -group[0].first, -group[0].second}, // 0-1

        {{0, 1, 1, 1, 1, 0}, group[1].first, group[1].second}, {{0, 2, 2, 2, 2, 0}, -group[1].first, -group[1].second}, // 2-3

        {{1, 1, 1, 1, 0}, group[2].first, group[2].second}, {{2, 2, 2, 2, 0}, -group[2].first, -group[2].second}, // 4-13
        {{1, 1, 1, 0, 1}, group[2].first, group[2].second}, {{2, 2, 2, 0, 2}, -group[2].first, -group[2].second},
        {{1, 1, 0, 1, 1}, group[2].first, group[2].second}, {{2, 2, 0, 2, 2}, -group[2].first, -group[2].second},
        {{1, 0, 1, 1, 1}, group[2].first, group[2].second}, {{2, 0, 2, 2, 2}, -group[2].first, -group[2].second},
        {{0, 1, 1, 1, 1}, group[2].first, group[2].second}, {{0, 2, 2, 2, 2}, -group[2].first, -group[2].second},

        {{0, 1, 1, 1, 0, 0}, group[3].first, group[3].second}, {{0, 2, 2, 2, 0, 0}, -group[3].first, -group[3].second}, // 14-21
        {{0, 0, 1, 1, 1, 0}, group[3].first, group[3].second}, {{0, 0, 2, 2, 2, 0}, -group[3].first, -group[3].second},
        {{0, 1, 0, 1, 1, 0}, group[3].first, group[3].second}, {{0, 2, 0, 2, 2, 0}, -group[3].first, -group[3].second},
        {{0, 1, 1, 0, 1, 0}, group[3].first, group[3].second}, {{0, 2, 2, 0, 2, 0}, -group[3].first, -group[3].second},

        {{0, 1, 1, 0, 0, 0}, group[4].first, group[4].second}, {{0, 2, 2, 0, 0, 0}, -group[4].first, -group[4].second}, // 22-33
        {{0, 0, 1, 1, 0, 0}, group[4].first, group[4].second}, {{0, 0, 2, 2, 0, 0}, -group[4].first, -group[4].second},
        {{0, 1, 0, 1, 0, 0}, group[4].first, group[4].second}, {{0, 2, 0, 2, 0, 0}, -group[4].first, -group[4].second},
        {{0, 1, 0, 0, 1, 0}, group[4].first, group[4].second}, {{0, 2, 0, 0, 2, 0}, -group[4].first, -group[4].second},
        {{0, 0, 1, 0, 1, 0}, group[4].first, group[4].second}, {{0, 0, 2, 0, 2, 0}, -group[4].first, -group[4].second},
        {{0, 0, 0, 1, 1, 0}, group[4].first, group[4].second}, {{0, 0, 0, 2, 2, 0}, -group[4].first, -group[4].second}
    };

    int positionEvaluation, prevPositionEvaluation;

    int boardSize, toWin;
    int lastX, lastY;
    int lastMinX, lastMinY, lastMaxX, lastMaxY;
    int minX, minY, maxX, maxY;
    bool moveX;

    std::vector<std::vector<int>> board;
    std::vector<int> results;
    Automatum* automatum = nullptr;

    void revert(int x, int y, int miX, int miY, int maX, int maY, int prevPos) {
        if (lastX != -1 && lastY != -1) {
            board[lastX][lastY] = 0;
        }
        revertBounds(miX, miY, maX, maY);
        lastX = x; lastY = y;
        positionEvaluation = prevPos;
        moveX = !moveX;
    }

    void updateBounds() {
        lastMinX = minX; lastMinY = minY;
        lastMaxX = maxX; lastMaxY = maxY;
        minX = std::min(minX, lastX); minY = std::min(minY, lastY);
        maxX = std::max(maxX, lastX); maxY = std::max(maxY, lastY);
    }

    void revertBounds(int miX, int miY, int maX, int maY) {
        minX = lastMinX; maxX = lastMaxX;
        minY = lastMinY; maxY = lastMaxY;
        lastMinX = miX; lastMinY = miY; lastMaxX = maX; lastMaxY = maY;
    }

    int evaluateMove(int x, int y) {
        automatum->processText(board, x - toWin, y, x + toWin, y, 1, 0, true, true);
        automatum->processText(board, x, y - toWin, x, y + toWin, 0, 1, true, true);
        automatum->processText(board, x - toWin, y - toWin, x + toWin, y + toWin, 1, 1, true, true);
        automatum->processText(board, x - toWin, y + toWin, x + toWin, y - toWin, 1, -1, true, false);
        automatum->getResults(results);

        int answer = 0;
        for (int i = 0; i < patterns.size(); ++i) {
            if (results[i] != 0) {
                answer += results[i] > 1 ? patterns[i].more : patterns[i].once;
            }
        }

        return answer;
    }

    int evaluatePosition() {
        for (int y = minY - maxDistToCheck; y <= maxY + maxDistToCheck; ++y) {
            // columns ↓
            automatum->processText(board, 
                                  minX - maxDistToCheck, y, 
                                  maxX + maxDistToCheck, y, 
                                  1, 0, true, true);
            // upper diagonals to the right ↘
            automatum->processText(board, 
                                  minX - maxDistToCheck, y, 
                                  maxX + maxDistToCheck, maxY + maxDistToCheck, 
                                  1, 1, true, true);
            // upper diagonals to the left ↙
            automatum->processText(board, 
                                  minX - maxDistToCheck, y, 
                                  maxX + maxDistToCheck, minY - maxDistToCheck, 
                                  1, -1, true, false);
        }
        for (int x = minX - maxDistToCheck; x <= maxX + maxDistToCheck; ++x) {
            // rows →
            automatum->processText(board, 
                                  x, minY - maxDistToCheck, 
                                  x, maxY + maxDistToCheck, 
                                  0, 1, true, true);
            // lower diagonals to the right ↘
            automatum->processText(board, 
                                  x, minY - maxDistToCheck, 
                                  maxX + maxDistToCheck, maxY + maxDistToCheck, 
                                  1, 1, true, true);
            // lower diagonals to the left ↙
            automatum->processText(board, 
                                  x, maxY + maxDistToCheck, 
                                  maxX + maxDistToCheck, minY - maxDistToCheck, 
                                  1, -1, true, false);
        }
        automatum->getResults(results);
        int answer = 0;
        int score;
        for (int i = 0; i < patterns.size(); ++i) {
            if (results[i] == 0) {
                continue;
            }
            score = results[i] > 1 ? patterns[i].more : patterns[i].once;
            if (score == inf || score == -inf) {
                return score;
            } else {
                answer += score;
            }
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distr(-randomNoise, randomNoise);
        return answer + distr(gen);
    }

    bool isClose(int centerX, int centerY) {
        for (int x = centerX - maxDistToMove; x <= centerX + maxDistToMove; ++x) {
            for (int y = centerY - maxDistToMove; y <= centerY + maxDistToMove; ++y) {
                if (board[x][y] != 0) {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<std::pair<int, int>> getAvailableMoves() {
        std::vector<std::pair<int, int>> moves;
        for (int x = minX - maxDistToMove; x <= maxX + maxDistToMove; ++x) {
            for (int y = minY - maxDistToMove; y <= maxY + maxDistToMove; ++y) {
                if (board[x][y] == 0 && isClose(x, y)) {
                    moves.push_back({x, y});
                }
            }
        }
        return moves;
    }

    bool moveIfCanWin(Game &machine) {
        int x, y, miX, maX, miY, maY;
        int lastEval = machine.positionEvaluation;
        machine.getLastMove(x, y);
        miX = lastMinX; miY = lastMinY; maX = lastMaxX; maY = lastMaxY;
        for (const auto& curMove : machine.getAvailableMoves()) {
            machine.move(curMove.first, curMove.second);
            if (machine.checkWin()) {
                move(curMove.first, curMove.second);
                return true;
            }
            machine.revert(x, y, miX, miY, maX, maY, lastEval);
        }
        return false;
    }

    int getBestScore(Game &machine, int depth, std::pair<int, int>& nextMove, int alpha, int beta) {
        if (machine.positionEvaluation >= inf) {
            return inf;
        } else if (machine.positionEvaluation <= -inf) {
            return -inf;
        } else if (depth >= maxDepth) {
            return std::min(inf, std::max(-inf, machine.positionEvaluation));
        }
        int x, y, score;
        int miX, maX, miY, maY;
        int lastEval = machine.positionEvaluation;
        machine.getLastMove(x, y);
        miX = lastMinX; miY = lastMinY; maX = lastMaxX; maY = lastMaxY;
        int bestScore = machine.isMoveX() ? -inf : inf;
        std::vector<std::pair<int, int>> moves = machine.getAvailableMoves();
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(moves.begin(), moves.end(), g);
        std::pair<int, int> bestMove = moves[0];

        for (const auto& move : moves) {
            machine.move(move.first, move.second);
            score = getBestScore(machine, depth + 1, nextMove, alpha, beta);
            machine.revert(x, y, miX, miY, maX, maY, lastEval);
            if (machine.isMoveX()) {
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = move;
                }
                if (score == inf) {
                    break;
                }
                alpha = std::max(alpha, bestScore);
            } else {
                if (score < bestScore) {
                    bestScore = score;
                    bestMove = move;
                }
                if (score == -inf) {
                    break;
                }
                beta = std::min(beta, bestScore);
            }
            if (alpha >= beta) {
                break;
            }
        }
        nextMove = bestMove;
        return bestScore;
    }
};