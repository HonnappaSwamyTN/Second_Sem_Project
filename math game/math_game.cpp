#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <conio.h>
using namespace std;

/* ================= CONSOLE HELPER FUNCTIONS ================= */

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void hideCursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 1;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void showCursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 1;
    info.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

void clearLine(int y) {
    gotoxy(0, y);
    cout << "                                        ";
}

/* ================= PLAYER CLASS ================= */
class Player {
private:
    string name;
    int lives;
    int score;
public:
    Player(string n) { name = n; lives = 3; score = 0; }
    void addScore(int s) { score += s; }
    void loseLife()      { lives--; }
    int getLives()       { return lives; }
    int getScore()       { return score; }
    string getName()     { return name; }
};

/* ================= ABSTRACT QUESTION CLASS ================= */
class Question {
protected:
    int a, b;
    char op;
    int answer;
    int points;
public:
    virtual void generate() = 0;
    bool checkAnswer(int userAns) { return userAns == answer; }
    int getPoints() { return points; }
    string getQuestionText() {
        return to_string(a) + " " + op + " " + to_string(b) + " = ?";
    }
    virtual ~Question() {}
};

class EasyQuestion : public Question {
public:
    void generate() override {
        a = rand() % 10 + 1; b = rand() % 10 + 1;
        op = '+'; answer = a + b; points = 10;
    }
};

class MediumQuestion : public Question {
public:
    void generate() override {
        a = rand() % 15 + 5; b = rand() % 10 + 1;
        op = '-'; answer = a - b; points = 20;
    }
};

class HardQuestion : public Question {
public:
    void generate() override {
        a = rand() % 10 + 2; b = rand() % 10 + 2;
        op = '*'; answer = a * b; points = 30;
    }
};

class ExpertQuestion : public Question {
public:
    void generate() override {
        b = rand() % 9 + 2;
        int multiplier = rand() % 10 + 1;
        a = b * multiplier;
        op = '/'; answer = a / b; points = 40;
    }
};

/* ================= LAYOUT CONSTANTS ================= */
const int HEADER_ROWS = 7;    // rows 0-6 used by header
const int DROP_ROWS   = 12;   // question travels 12 rows down
const int INPUT_ROW   = HEADER_ROWS + DROP_ROWS + 1;
const int TIME_LIMIT  = 5;    // seconds before time's up

// Drop speed: question moves 1 row every DROP_INTERVAL milliseconds
// 12 rows / 5 seconds = need ~400ms per row to fill exactly 5 seconds
// We use 350ms so it reaches bottom slightly before time runs out
const int DROP_INTERVAL = 350;

/* ================= GAME CLASS ================= */
class Game {
private:
    Player* player;
    int highScore;

public:
    Game() { highScore = loadHighScore(); }

    int loadHighScore() {
        ifstream file("highscore.txt");
        int hs = 0;
        if (file >> hs) return hs;
        return 0;
    }

    void saveHighScore(int score) {
        if (score > highScore) {
            ofstream file("highscore.txt");
            file << score;
        }
    }

    void drawHeader() {
        gotoxy(0, 0); cout << "=============================";
        gotoxy(0, 1); cout << "       MATH DROP GAME        ";
        gotoxy(0, 2); cout << "=============================";
        gotoxy(0, 6); cout << "-----------------------------";
    }

    void updateHeader(int timeLeft) {
        // Lives
        gotoxy(0, 3);
        cout << "Lives: ";
        for (int i = 0; i < player->getLives(); i++)  cout << "<3 ";
        for (int i = player->getLives(); i < 3; i++)  cout << "    ";

        // Score
        gotoxy(0, 4);
        cout << "Score: " << player->getScore() << "      ";

        // Timer bar — shrinks from right to left as time runs out
        gotoxy(0, 5);
        cout << "Time:  [";
        for (int i = 0; i < timeLeft; i++)          cout << "#";
        for (int i = timeLeft; i < TIME_LIMIT; i++) cout << ".";
        cout << "] " << timeLeft << "s ";
    }

    Question* createQuestion() {
        int r = rand() % 4;
        Question* q;
        if      (r == 0) q = new EasyQuestion();
        else if (r == 1) q = new MediumQuestion();
        else if (r == 2) q = new HardQuestion();
        else             q = new ExpertQuestion();
        q->generate();
        return q;
    }

    bool dropQuestion(Question* q, int& userAns) {
        string text     = "  >> " + q->getQuestionText() + "  ";
        string input    = "";
        int currentRow  = HEADER_ROWS;   // start just below the header line
        int prevRow     = -1;

        DWORD startMs   = GetTickCount();   // millisecond timer for smooth drop
        DWORD lastDrop  = startMs;          // when did we last move the question down

        while (true) {
            DWORD now     = GetTickCount();
            int elapsedMs = (int)(now - startMs);
            int timeLeft  = TIME_LIMIT - (elapsedMs / 1000);

            // Time is up
            if (timeLeft <= 0) return false;

            // --- UPDATE TIMER BAR (every loop, very cheap) ---
            updateHeader(timeLeft);

            // --- DROP THE QUESTION (only when DROP_INTERVAL ms has passed) ---
            if ((int)(now - lastDrop) >= DROP_INTERVAL) {
                lastDrop = now;

                // Erase old row
                if (prevRow != -1) clearLine(prevRow);

                // Draw at new row
                gotoxy(0, currentRow);
                cout << text;
                prevRow = currentRow;

                // Move down for next step, stop at the bottom
                if (currentRow < HEADER_ROWS + DROP_ROWS) {
                    currentRow++;
                }
            }

            // --- DRAW INPUT LINE (always at fixed row) ---
            gotoxy(0, INPUT_ROW);
            showCursor();
            cout << "Answer: " << input << "  ";
            gotoxy(8 + (int)input.size(), INPUT_ROW);

            // --- READ KEYBOARD (non-blocking) ---
            if (_kbhit()) {
                char ch = _getch();

                if (ch == '\r' && !input.empty()) {
                    userAns = stoi(input);
                    hideCursor();
                    return true;
                }
                else if (ch == 8 && !input.empty()) {
                    input.pop_back();
                }
                else if ((ch == '-' && input.empty()) || isdigit(ch)) {
                    input += ch;
                }
            }

            Sleep(30); // ~30fps loop, smooth with no flicker
        }
    }

    void start() {
        srand(time(0));
        system("cls");
        hideCursor();

        // Name entry screen
        showCursor();
        gotoxy(0, 0); cout << "=============================";
        gotoxy(0, 1); cout << "       MATH DROP GAME        ";
        gotoxy(0, 2); cout << "=============================";
        gotoxy(0, 4); cout << "Enter Player Name: ";
        string name;
        cin >> name;
        player = new Player(name);

        system("cls");
        hideCursor();
        drawHeader();

        while (player->getLives() > 0) {
            // Clear drop zone + input area
            for (int i = HEADER_ROWS; i <= INPUT_ROW + 1; i++)
                clearLine(i);

            Question* q = createQuestion();
            int userAns = 0;

            bool answeredInTime = dropQuestion(q, userAns);

            // Clear drop zone
            for (int i = HEADER_ROWS; i <= INPUT_ROW + 1; i++)
                clearLine(i);

            // Show result
            gotoxy(0, HEADER_ROWS + DROP_ROWS / 2);
            if (!answeredInTime) {
                cout << "   *** TIME'S UP! Lost a life! ***  ";
                player->loseLife();
            } else if (q->checkAnswer(userAns)) {
                cout << "   *** CORRECT! +" << q->getPoints() << " points! ***    ";
                player->addScore(q->getPoints());
            } else {
                cout << "   *** WRONG! Lost a life! ***      ";
                player->loseLife();
            }

            updateHeader(0);
            Sleep(1200);
            delete q;
        }

        // Game Over
        system("cls");
        showCursor();
        cout << "=============================\n";
        cout << "          GAME OVER          \n";
        cout << "=============================\n";
        cout << "Player:      " << player->getName() << "\n";
        cout << "Final Score: " << player->getScore() << "\n";
        saveHighScore(player->getScore());
        if (player->getScore() > highScore)
            cout << "*** New High Score! ***\n";
        cout << "=============================\n";
        delete player;
    }
};

/* ================= MAIN ================= */
int main() {
    Game game;
    game.start();
    return 0;
}g