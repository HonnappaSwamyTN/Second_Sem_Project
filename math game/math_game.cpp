#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using namespace std;

/* ================= PLAYER CLASS ================= */

class Player {
private:
    string name;
    int lives;
    int score;

public:
    Player(string n) {
        name = n;
        lives = 3;
        score = 0;
    }

    void addScore(int s) { score += s; }
    void loseLife() { lives--; }

    int getLives() { return lives; }
    int getScore() { return score; }
    string getName() { return name; }
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

    bool checkAnswer(int userAns) {
        return userAns == answer;
    }

    int getPoints() { return points; }

    string getQuestionText() {
        return to_string(a) + " " + op + " " + to_string(b) + " = ?";
    }

    virtual ~Question() {}
};

/* ================= EASY QUESTION ================= */

class EasyQuestion : public Question {
public:
    void generate() override {
        a = rand() % 10 + 1;
        b = rand() % 10 + 1;
        op = '+';
        answer = a + b;
        points = 10;
    }
};

/* ================= MEDIUM QUESTION ================= */

class MediumQuestion : public Question {
public:
    void generate() override {
        a = rand() % 15 + 5;
        b = rand() % 10 + 1;
        op = '-';
        answer = a - b;
        points = 20;
    }
};

/* ================= HARD QUESTION ================= */

class HardQuestion : public Question {
public:
    void generate() override {
        a = rand() % 10 + 2;
        b = rand() % 10 + 2;
        op = '*';
        answer = a * b;
        points = 30;
    }
};

/* ================= GAME CLASS ================= */

class Game {
private:
    Player* player;
    int highScore;

public:
    Game() {
        highScore = loadHighScore();
    }

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
            cout << "\n🎉 New High Score Saved!\n";
        }
    }

    void showLives(int lives) {
        cout << "Lives: ";
        for (int i = 0; i < lives; i++) cout << "❤️ ";
        cout << endl;
    }

    void showUI(string question, int pos) {
        system("cls");

        cout << "=============================\n";
        cout << "        MATH DROP GAME        \n";
        cout << "=============================\n";

        showLives(player->getLives());

        cout << "Score: " << player->getScore() << endl;
        cout << "High Score: " << highScore << endl;

        cout << "\n\n";

        for (int i = 0; i < pos; i++)
            cout << endl;

        cout << "        " << question << endl;

        cout << "\n-----------------------------\n";
    }

    Question* createQuestion() {
        int r = rand() % 3;

        Question* q;

        if (r == 0) q = new EasyQuestion();
        else if (r == 1) q = new MediumQuestion();
        else q = new HardQuestion();

        q->generate();
        return q;
    }

    void start() {
        srand(time(0));

        string name;
        cout << "Enter Player Name: ";
        cin >> name;

        player = new Player(name);

        while (player->getLives() > 0) {

            Question* q = createQuestion();

            string text = q->getQuestionText();

            bool answered = false;
            int userAns;

            for (int pos = 0; pos < 10; pos++) {

                showUI(text, pos);

                cout << "\nAnswer: ";

                if (cin >> userAns) {
                    if (q->checkAnswer(userAns)) {
                        cout << "✅ Correct!\n";
                        player->addScore(q->getPoints());
                        answered = true;
                        Sleep(800);
                        break;
                    } else {
                        cout << "❌ Wrong!\n";
                        player->loseLife();
                        Sleep(800);
                        answered = true;
                        break;
                    }
                }

                Sleep(500);
            }

            if (!answered) {
                cout << "\nMissed! Question reached bottom!\n";
                player->loseLife();
                Sleep(800);
            }

            delete q;
        }

        system("cls");

        cout << "=============================\n";
        cout << "         GAME OVER            \n";
        cout << "=============================\n";

        cout << "Final Score: " << player->getScore() << endl;

        saveHighScore(player->getScore());

        delete player;
    }
};

/* ================= MAIN ================= */

int main() {
    Game game;
    game.start();
    return 0;
}