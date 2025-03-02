#include <iostream>
#include <vector>
#include <utility>
#include <array>
#include <thread>
#include "sys/ioctl.h"
#include "termios.h"
#include "stdio.h"
#include "time.h"
#include "stdlib.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <memory>

class Board {
public:
    int x_;
    int z_;
    bool out;
    std::vector<std::vector<char>> bg;
    Board(int x, int z) : x_(x), z_(z), bg(z, std::vector<char>(x, '.')), out(false) {}
    void setup() {
        int i;
        for (i = 0; i < x_; i++) {
            bg[0][i] = 'x';
            bg[3][i] = 'x';
            bg[z_ - 1][i] = 'x';
        }
        for (i = 0; i < z_; i++) {
            bg[i][0] = 'x';
            bg[i][x_ - 1] = 'x';
        }
        bg[1][8] = 'x';
        bg[2][8] = 'x';
    }
    void print() {
        for (int j = 0; j < z_; j++) {
            for (int i = 0; i < x_; i++) {
                if (bg[j][i] == 'x') {
                    std::cout << "# ";
                } else if ((bg[j][i] == 'L') || (bg[j][i] == 'M')) {
                    std::cout << "\033[38;5;208mo \033[0m";
                } else if ((bg[j][i] == 'Z') || (bg[j][i] == 'A')) {
                    std::cout << "\033[92mo \033[0m";
                } else if ((bg[j][i] == 'T') || (bg[j][i] == 'U')) {
                    std::cout << "\033[35mo \033[0m";
                } else if ((bg[j][i] == 'O') || (bg[j][i] == 'P')) { //sssssssssssssssssssssss
                    std::cout << "\033[33mo \033[0m";
                } else if (bg[j][i] == '.') {
                    std::cout << "  ";
                } else {
                    std::cout << bg[j][i] << ' ';
                }
            }
            std::cout << std::endl;
        }
    }
    void clear() {
        int out = system("clear");
        if (out != 0) {
            std::cerr << "clear command failed" << std::endl;
            exit(1);
        }
    }
    void test_defeat() {
        bool defeat=false;
        for (int i = 1; i < x_ - 1; i++) {
            if ((bg[5][i] == 'M') || (bg[5][i] == 'P') || (bg[5][i] == 'A') || (bg[5][i] == 'U')) { //ssssssssssssssssssss
                defeat = true;
            }
        }
        if (defeat) { std::cout << "Game Over!" << std::endl;
            std::exit(0);}}
    void test_ligne() { int ligne=-1;
        for (int i=6; i < z_-1; i++){bool full=true;
            for (int j=1; j < x_-1; j++){if (bg[i][j]!='M' && bg[i][j]!='P' && bg[i][j]!='A' && bg[i][j]!='U'){full=false;}} //sssssssssssssssss
            if (full){ligne=i;}}
            if (ligne !=-1){for (int i=ligne; i > 6; i--){for (int j=1;j < x_-1; j++){bg[i][j]=bg[i-1][j];}}}

    }
};

class Header {
public:
    int score_;
    Header() : score_(0) {}
    void update_score() { score_ += 10; }
    void add(Board &board) {
        board.bg[1][2] = 's';
        board.bg[1][3] = 'c';
        board.bg[1][4] = 'o';
        board.bg[1][5] = 'r';
        board.bg[1][6] = 'e';
        board.bg[2][2] = '0' + score_ / 10000;
        board.bg[2][3] = '0' + (score_ % 10000) / 1000;
        board.bg[2][4] = '0' + (score_ % 1000) / 100;
        board.bg[2][5] = '0' + (score_ % 100) / 10;
        board.bg[2][6] = '0' + score_ % 10;
        board.bg[1][10] = 'p';
        board.bg[1][11] = 'r';
        board.bg[1][12] = 'o';
        board.bg[2][10] = 't';
        board.bg[2][11] = 'e';
        board.bg[2][12] = 't';
    }
    void remove(Board board) {
        board.bg[1][2] = '.';
        board.bg[1][3] = '.';
        board.bg[1][4] = '.';
        board.bg[1][5] = '.';
        board.bg[1][6] = '.';
        board.bg[2][2] = '.';
        board.bg[2][3] = '.';
        board.bg[2][4] = '.';
        board.bg[2][5] = '.';
        board.bg[2][6] = '.';
        board.bg[1][10] = '.';
        board.bg[1][11] = '.';
        board.bg[1][12] = '.';
        board.bg[2][10] = '.';
        board.bg[2][11] = '.';
        board.bg[2][12] = '.';
    }
};

class Tetromino {
public:
    int role_; // 0: next tet, 1: active tet, 2: fixed tet
    char dir_;
    int dx_;
    bool touch_left;
    bool touch_right;
    Tetromino() : dir_('N'), touch_right(false), touch_left(false), role_(0), dx_(0) {}
    virtual void add(Board &board) const = 0;
    virtual void remove(Board &board) const = 0;
    virtual void movement(char key) = 0;
    virtual void update_coordinates() = 0;
    virtual void initiate_fall() = 0;
    virtual void test_ground(Board &board, Header &head) = 0;
    virtual void update_collision(Board board) = 0;
    virtual ~Tetromino() = default;
};

class L : public Tetromino {
public:
    std::vector<std::pair<int, int>> pos_;
    L() : Tetromino() {}
    void add(Board &board) const override {
        if (role_ == 2) {
            for (auto p : pos_) {
                board.bg[p.first][p.second] = 'M';
            }
        } else {
            for (auto p : pos_) {
                board.bg[p.first][p.second] = 'L';
            }
        }
    }
    void remove(Board &board) const override {
        for (auto p : pos_) {
            board.bg[p.first][p.second] = '.';
        }
    }
    void movement(char key) override {
        if (key == 'q' && !touch_left) {
            dx_ = -1;
        } else if (key == 'd' && !touch_right) {
            dx_ = 1;
        } else {
            dx_ = 0;
        }
    }
    void update_coordinates() override {
        if (role_ == 1) {
            for (auto &p : pos_) {
                p.first += 1;
                p.second += dx_;
            }
        } else if (role_ == 0) {
            pos_ = {{1, 15}, {1, 16}, {2, 15}, {1, 17}};
        }
    }
    void initiate_fall() override {
        pos_ = {{4, 9}, {4, 8}, {5, 8}, {4, 10}};
        role_ = 1;
    }
    void test_ground(Board &board, Header &head) override {
        if (((board.bg[pos_[1].first + 1][pos_[1].second] != '.') && (board.bg[pos_[1].first + 1][pos_[1].second] != 'O'))||((board.bg[pos_[0].first + 1][pos_[0].second] != '.') && (board.bg[pos_[0].first + 1][pos_[0].second] != 'O'))||((board.bg[pos_[2].first + 1][pos_[2].second] != '.') && (board.bg[pos_[2].first + 1][pos_[2].second] != 'O'))||((board.bg[pos_[3].first + 1][pos_[3].second] != '.') && (board.bg[pos_[3].first + 1][pos_[3].second] != 'O'))) {
            role_ = 2;
        }
    }
    void update_collision(Board board) override {
        touch_left=false;
        touch_right=false;
        for (auto p : pos_) {
            if (board.bg[p.first][p.second + 1] != 'L' && board.bg[p.first][p.second + 1] != '.') {
                touch_right = true;
            }
            if (board.bg[p.first][p.second - 1] != 'L' && board.bg[p.first][p.second - 1] != '.') {
                touch_left = true;
            }
        }
    }
};

class O : public Tetromino {
    public:
        std::vector<std::pair<int, int>> pos_;
        O() : Tetromino() {}
        void add(Board &board) const override {
            if (role_ == 2) {
                for (auto p : pos_) {
                    board.bg[p.first][p.second] = 'P';
                }
            } else {
                for (auto p : pos_) {
                    board.bg[p.first][p.second] = 'O';
                }
            }
        }
        void remove(Board &board) const override {
            for (auto p : pos_) {
                board.bg[p.first][p.second] = '.';
            }
        }
        void movement(char key) override {
            if (key == 'q' && !touch_left) {
                dx_ = -1;
            } else if (key == 'd' && !touch_right) {
                dx_ = 1;
            } else {
                dx_ = 0;
            }
        }
        void update_coordinates() override {
            if (role_ == 1) {
                for (auto &p : pos_) {
                    p.first += 1;
                    p.second += dx_;
                }
            } else if (role_ == 0) {
                pos_ = {{2, 15}, {2, 16}, {1, 16}, {1, 15}};
            }
        }
        void initiate_fall() override {
            pos_ = {{4, 9}, {5, 9}, {5, 10}, {4, 10}};
            role_ = 1;
        }
        void test_ground(Board &board, Header &head) override {
            if (((board.bg[pos_[1].first + 1][pos_[1].second] != '.') && (board.bg[pos_[1].first + 1][pos_[1].second] != 'O'))||((board.bg[pos_[0].first + 1][pos_[0].second] != '.') && (board.bg[pos_[0].first + 1][pos_[0].second] != 'O'))||((board.bg[pos_[2].first + 1][pos_[2].second] != '.') && (board.bg[pos_[2].first + 1][pos_[2].second] != 'O'))||((board.bg[pos_[3].first + 1][pos_[3].second] != '.') && (board.bg[pos_[3].first + 1][pos_[3].second] != 'O'))) {
                role_ = 2;
            }
        }
        void update_collision(Board board) override {
            touch_left=false;
            touch_right=false;
            for (auto p : pos_) {
                if (board.bg[p.first][p.second + 1] != 'O' && board.bg[p.first][p.second + 1] != '.') {
                    touch_right = true;
                }
                if (board.bg[p.first][p.second - 1] != 'O' && board.bg[p.first][p.second - 1] != '.') {
                    touch_left = true;
                }
            }
        }
    };

class Z : public Tetromino {
        public:
            std::vector<std::pair<int, int>> pos_;
            Z() : Tetromino() {}
            void add(Board &board) const override {
                if (role_ == 2) {
                    for (auto p : pos_) {
                        board.bg[p.first][p.second] = 'A';
                    }
                } else {
                    for (auto p : pos_) {
                        board.bg[p.first][p.second] = 'Z';
                    }
                }
            }
            void remove(Board &board) const override {
                for (auto p : pos_) {
                    board.bg[p.first][p.second] = '.';
                }
            }
            void movement(char key) override {
                if (key == 'q' && !touch_left) {
                    dx_ = -1;
                } else if (key == 'd' && !touch_right) {
                    dx_ = 1;
                } else {
                    dx_ = 0;
                }
            }
            void update_coordinates() override {
                if (role_ == 1) {
                    for (auto &p : pos_) {
                        p.first += 1;
                        p.second += dx_;
                    }
                } else if (role_ == 0) {
                    pos_ = {{2, 15}, {2, 16}, {1, 14}, {1, 15}};
                }
            }
            void initiate_fall() override {
                pos_ = {{4, 9}, {5, 9}, {5, 10}, {4, 8}};
                role_ = 1;
            }
            void test_ground(Board &board, Header &head) override {
                if (((board.bg[pos_[1].first + 1][pos_[1].second] != '.') && (board.bg[pos_[1].first + 1][pos_[1].second] != 'Z'))||((board.bg[pos_[0].first + 1][pos_[0].second] != '.') && (board.bg[pos_[0].first + 1][pos_[0].second] != 'Z'))||((board.bg[pos_[2].first + 1][pos_[2].second] != '.') && (board.bg[pos_[2].first + 1][pos_[2].second] != 'Z'))||((board.bg[pos_[3].first + 1][pos_[3].second] != '.') && (board.bg[pos_[3].first + 1][pos_[3].second] != 'Z'))) {
                    role_ = 2;
                }
            }
            void update_collision(Board board) override {
                touch_left=false;
                touch_right=false;
                for (auto p : pos_) {
                    if (board.bg[p.first][p.second + 1] != 'Z' && board.bg[p.first][p.second + 1] != '.') {
                        touch_right = true;
                    }
                    if (board.bg[p.first][p.second - 1] != 'Z' && board.bg[p.first][p.second - 1] != '.') {
                        touch_left = true;
                    }
                }
            }
        };

class T : public Tetromino {
    public:
        std::vector<std::pair<int, int>> pos_;
        T() : Tetromino() {}
        void add(Board &board) const override {
            if (role_ == 2) {
                for (auto p : pos_) {
                    board.bg[p.first][p.second] = 'U';
                }
            } else {
                for (auto p : pos_) {
                    board.bg[p.first][p.second] = 'T';
                }
            }
        }
        void remove(Board &board) const override {
            for (auto p : pos_) {
                board.bg[p.first][p.second] = '.';
            }
        }
        void movement(char key) override {
            if (key == 'q' && !touch_left) {
                dx_ = -1;
            } else if (key == 'd' && !touch_right) {
                dx_ = 1;
            } else {
                dx_ = 0;
            }
        }
        void update_coordinates() override {
            if (role_ == 1) {
                for (auto &p : pos_) {
                    p.first += 1;
                    p.second += dx_;
                }
            } else if (role_ == 0) {
                pos_ = {{2, 15}, {1, 16}, {1, 14}, {1, 15}};
            }
        }
        void initiate_fall() override {
            pos_ = {{4, 9}, {5, 9}, {4, 10}, {4, 8}};
            role_ = 1;
        }
        void test_ground(Board &board, Header &head) override {
            if (((board.bg[pos_[1].first + 1][pos_[1].second] != '.') && (board.bg[pos_[1].first + 1][pos_[1].second] != 'Z'))||((board.bg[pos_[0].first + 1][pos_[0].second] != '.') && (board.bg[pos_[0].first + 1][pos_[0].second] != 'Z'))||((board.bg[pos_[2].first + 1][pos_[2].second] != '.') && (board.bg[pos_[2].first + 1][pos_[2].second] != 'Z'))||((board.bg[pos_[3].first + 1][pos_[3].second] != '.') && (board.bg[pos_[3].first + 1][pos_[3].second] != 'Z'))) {
                role_ = 2;
            }
        }
        void update_collision(Board board) override {
            touch_left=false;
            touch_right=false;
            for (auto p : pos_) {
                if (board.bg[p.first][p.second + 1] != 'T' && board.bg[p.first][p.second + 1] != '.') {
                    touch_right = true;
                }
                if (board.bg[p.first][p.second - 1] != 'T' && board.bg[p.first][p.second - 1] != '.') {
                    touch_left = true;
                }
            }
        }
    };

        //ssssssssssssssssssssssssssssssssssssss

std::unique_ptr<Tetromino> rand_tet() {
    if (std::rand() % 4 == 0) {
        return std::make_unique<L>();
    } else if (std::rand() % 3 == 0){
        return std::make_unique<O>();
    } else if (std::rand() % 2 == 0){
        return std::make_unique<Z>(); //ssssssssssssssssssssssssssssss
    } else {
        return std::make_unique<T>();
    }
}

class Game {
public:
    int lap_;
    char key;
    int STDIN;
    bool initialized;
    int keyev;
    Game(int lap) : lap_(lap), key('a'), STDIN(0), initialized(false), keyev(0) {}
    void frameSleep() {
        std::this_thread::sleep_for(std::chrono::milliseconds(lap_));
    }
        void start(Board &board, Header &head) {
        board.clear();
        std::unique_ptr<Tetromino> tet_1;
        std::unique_ptr<Tetromino> tet_2;
        tet_1 = rand_tet();
        tet_2 = rand_tet();
        tet_2->initiate_fall();
        std::unique_ptr<Tetromino> next_tet = rand_tet();
    
        while (true) {
            board.test_defeat();
            frameSleep();
            keyEvent();
            tet_2->dx_ = 0;
            tet_1->dx_ = 0;
            if (tet_2->role_==1) {
                tet_2->update_collision(board);
                if (keyev) {
                    std::cin >> key;
                    if ((key == 'q') || (key == 'd')) {
                        tet_2->movement(key);
                    }
                }
            }
            else {tet_1->update_collision(board);
                if (keyev) {
                    std::cin >> key;
                    if ((key == 'q') || (key == 'd')) {
                        tet_1->movement(key);
                    }
                }
            }
            board.clear();
            if (tet_2->role_==1){tet_2->test_ground(board, head);}
            else {tet_1->test_ground(board, head);}
            tet_2->add(board);
            tet_1->add(board);
            head.add(board);
            board.test_ligne();
            board.print();

            if (tet_2->role_ == 2) {
                tet_1->remove(board);
                tet_1->initiate_fall();
                tet_2 = std::move(next_tet);
                tet_2->role_ = 0;
                next_tet = rand_tet();
                head.score_+=10;
            }
            if (tet_1->role_ == 2) {
                tet_2->remove(board);
                tet_2->initiate_fall();
                tet_1 = std::move(next_tet); 
                tet_1->role_ = 0;
                next_tet = rand_tet();
                head.score_+=10;
            }

            tet_1->remove(board);
            tet_2->remove(board);
            tet_1->update_coordinates();
            tet_2->update_coordinates();
            head.remove(board);
        }
    }
    void keyEvent() {
        if (!initialized) {
            termios term;
            tcgetattr(STDIN, &term);
            term.c_lflag &= ~ICANON;
            tcsetattr(STDIN, TCSANOW, &term);
            setbuf(stdin, NULL);
            initialized = true;
        }
        int bytesWaiting;
        ioctl(STDIN, FIONREAD, &bytesWaiting);
        keyev = bytesWaiting;
    }
};

int main() {
    std::srand(std::time(nullptr)); 
    Board board(20, 30);
    Game game(300);
    Header head;

    board.setup();
    game.start(board, head);

    return 0;
}