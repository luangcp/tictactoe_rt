#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <random>
#include <chrono>

class TicTacToe {
public:
    char board[3][3];
    char current_player;
    bool game_over;
    char winner;
    std::mutex mtx;
    std::condition_variable cv;

    TicTacToe() {
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) board[i][j] = ' ';
        current_player = 'X';
        game_over = false;
        winner = 'D';
    }

    void display_board() {
        for(int i=0; i<3; i++) {
            std::cout << " " << board[i][0] << " | " << board[i][1] << " | " << board[i][2] << "\n";
            if(i<2) std::cout << "-----------\n";
        }
        std::cout << "\n";
    }

    bool make_move(char player, int row, int col) {
        if(row >= 0 && row < 3 && col >= 0 && col < 3 && board[row][col] == ' ') {
            board[row][col] = player;
            return true;
        }
        return false;
    }

    bool check_win(char player) {
        for(int i=0; i<3; i++) {
            if(board[i][0]==player && board[i][1]==player && board[i][2]==player) return true;
            if(board[0][i]==player && board[1][i]==player && board[2][i]==player) return true;
        }
        if(board[0][0]==player && board[1][1]==player && board[2][2]==player) return true;
        if(board[0][2]==player && board[1][1]==player && board[2][0]==player) return true;
        return false;
    }

    bool check_draw() {
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) if(board[i][j] == ' ') return false;
        return true;
    }

    bool is_game_over() { return game_over; }
    char get_winner() { return winner; }
};

class Player {
public:
    TicTacToe& game;
    char symbol;
    std::string strategy;

    Player(TicTacToe& g, char s, std::string strat) : game(g), symbol(s), strategy(strat) {}

    void play() {
        while(true) {
            std::unique_lock<std::mutex> lock(game.mtx);
            game.cv.wait(lock, [&] { return game.current_player == symbol || game.game_over; });

            if(game.game_over) break;

            bool move_made = false;
            if(strategy == "sequencial") {
                for(int i=0; i<3 && !move_made; i++) {
                    for(int j=0; j<3 && !move_made; j++) {
                        if(game.make_move(symbol, i, j)) move_made = true;
                    }
                }
            } else { // aleatorio
                while(!move_made && !game.check_draw()) {
                    int r = rand() % 3;
                    int c = rand() % 3;
                    if(game.make_move(symbol, r, c)) move_made = true;
                }
            }

            if(move_made) {
                std::cout << "Matricula: 2022426104 - Jogador " << symbol << " jogou:\n";
                game.display_board();
                
                if(game.check_win(symbol)) {
                    game.game_over = true;
                    game.winner = symbol;
                } else if(game.check_draw()) {
                    game.game_over = true;
                    game.winner = 'D';
                } else {
                    game.current_player = (symbol == 'X') ? 'O' : 'X';
                }
            }
            
            game.cv.notify_all();
        }
    }
};

int main() {
    srand(time(NULL));
    TicTacToe game;
    Player p1(game, 'X', "sequencial");
    Player p2(game, 'O', "aleatorio");

    std::thread t1(&Player::play, &p1);
    std::thread t2(&Player::play, &p2);

    t1.join();
    t2.join();

    std::cout << ">>> Fim de jogo! Vencedor: " << game.get_winner() << " <<<\n\n";
    return 0;
}
