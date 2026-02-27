#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>
#include <conio.h>
#include <windows.h>

using namespace std;

// ================= CURSOR =================
void gotoXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ================= PLAYER =================
class Player {
private:
    int score;
    int lives;

public:
    Player() {
        score = 0;
        lives = 3;
    }

    void addScore(int s) { score += s; }
    void loseLife() { lives--; }

    int getScore() { return score; }
    int getLives() { return lives; }
};

// ================= QUESTION =================
class Question {
private:
    int a, b, ans;

public:
    void generate() {
        a = rand() % 10;
        b = rand() % 10;
        ans = a + b;
    }

    int getAnswer() { return ans; }

    string getText() {
        return to_string(a) + " + " + to_string(b);
    }
};

// ================= GAME =================
class Game {
private:
    Player player;

public:
    void play() {

        srand(time(0));

        const int DEADLINE_Y = 18;
        const int START_Y = 2;

        const int TOTAL_TIME_MS = 5000;   // 5 seconds
        const int FRAME_DELAY = 50;       // smoothness
        const int TOTAL_FRAMES = TOTAL_TIME_MS / FRAME_DELAY;

        while (player.getLives() > 0) {

            Question q;
            q.generate();

            string input = "";
            bool answered = false;

            float position = START_Y;
            float step = (float)(DEADLINE_Y - START_Y) / TOTAL_FRAMES;

            for (int frame = 0; frame <= TOTAL_FRAMES; frame++) {

                // Clear falling area
                for (int i = START_Y; i <= DEADLINE_Y + 2; i++) {
                    gotoXY(0, i);
                    cout << "                                                ";
                }

                // Remaining time
                int timeLeft = (TOTAL_FRAMES - frame) * FRAME_DELAY / 1000;

                // Score + Timer
                gotoXY(0, 0);
                cout << "Lives: " << player.getLives()
                     << "   Score: " << player.getScore()
                     << "   Time Left: " << timeLeft << "s   ";

                // Deadline line
                gotoXY(0, DEADLINE_Y);
                cout << "--------------------------------------------";
                gotoXY(5, DEADLINE_Y);
                cout << " ANSWER BEFORE THIS LINE ";

                // Draw question
                gotoXY(15, (int)position);
                cout << q.getText() << " = ?";

                // Input area
                gotoXY(0, DEADLINE_Y + 3);
                cout << "Your Answer: " << input << "   ";

                // Keyboard input
                if (_kbhit()) {
                    char ch = _getch();

                    if (ch == 13) { // ENTER
                        answered = true;
                        break;
                    }
                    else if (ch == 8) { // BACKSPACE
                        if (!input.empty())
                            input.pop_back();
                    }
                    else if (isdigit(ch)) {
                        input += ch;
                    }
                }

                position += step;

                this_thread::sleep_for(chrono::milliseconds(FRAME_DELAY));
            }

            // ===== RESULT =====
            if (answered && !input.empty()) {
                int userAns = stoi(input);

                if (userAns == q.getAnswer()) {
                    gotoXY(0, DEADLINE_Y + 5);
                    cout << "Correct!                 ";
                    player.addScore(10);
                } else {
                    gotoXY(0, DEADLINE_Y + 5);
                    cout << "Wrong!                   ";
                    player.loseLife();
                }
            } else {
                gotoXY(0, DEADLINE_Y + 5);
                cout << "Time Up!                 ";
                player.loseLife();
            }

            this_thread::sleep_for(chrono::seconds(1));
        }

        // ===== GAME OVER =====
        gotoXY(0, DEADLINE_Y + 7);
        cout << "===== GAME OVER =====\n";
        cout << "Final Score: " << player.getScore() << endl;
    }
};

// ================= MAIN =================
int main() {
    Game g;
    g.play();
    return 0;
}