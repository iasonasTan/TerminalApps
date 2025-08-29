#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <csignal>

class Screen;
class Player;
class Target;
class Entity;
class TargetSpawner;
void read();
void worker();
int kbhit();

std::atomic<bool> running(true);

class Entity {
    protected:
        int x;
        int y;

    public:
        Entity(int x, int y) {
            this -> x = x;
            this -> y = y;
        }

    public:
        virtual void update() = 0;
        int getX() { return x; }
        int getY() { return y; }
};

class Player : public Entity {
    private:
        int score;
        char direction;

    public:
        Player(int x, int y) : Entity(x, y) {
            score = 0;
        }

    public:
        void increaseScore() {
            score++;
        }

        int getScore() {
            return score;
        }

        void update() override;

        void read() {
            if(!kbhit()) {
                direction = 0;
                return;
            }
            direction = getchar();
        }
};
Player* player_p = nullptr;

class Screen {
    private:
        int width;
        int height;

    public:
        Screen(int w, int h) {
            width = w;
            height = h;
        }

    public:
        int getWidth() {
            return width;
        }

        int getHeight() {
            return height;
        }

        void drawHeader() {
            std::cout << "SNAKE GAME C++" << std::endl;
        }
        
        void draw(Target** entities, int len) {
            system("clear");
            drawHeader();
            for(int i=0; i<height; i++) {
                int limit = i == 0 ? width + 1 : width;
                for(int j=0; j<limit; j++) {
                    bool foundEntity = false;
                    for(int q=0; q<len; q++) {
                        Entity* entity = (Entity*)entities[q];
                        if(j==entity->getX()&&i==entity->getY()) {
                            std::cout << "*";
                            foundEntity = true;
                        }
                    }
                    if(!foundEntity) {
                        if(i==0&&j==limit-1) {
                            std::cout << "S:" << player_p -> getScore();
                        } else if(j==player_p->getX()&&i==player_p->getY()){
                            std::cout << "%";
                        } else {
                            std::cout << ".";
                        }
                    }
                }
                std::cout << std::endl;
            }
        }
};
Screen* screen_p = nullptr;

void Player::update() {
    switch(direction) {
        case 'w':
            if(y > 0) {
                y--;
            }
            break;
        case 's':
            if(y < screen_p -> getHeight() -1) {
                y++;
            }
            break;
        case 'a':
            if(x > 0) {
                x--;
            }
            break;
        case 'd':
            if (x < screen_p -> getWidth() -1) {
                x++;
            }
            break;
        case 'q':
            running = false;
            break;
    }
    direction = 0;
}

class Target : public Entity {
    private:
        bool disposed;
    
    public:
        Target() : Entity(std::rand() % screen_p -> getWidth(), std::rand() % screen_p -> getHeight()) {
            disposed = false;
        }

    public:
        void update() override {
            // detect collision
            if(player_p -> getX() == x && player_p -> getY() == y) {
                dispose();
                player_p -> increaseScore();
            }
        }

        void dispose() { disposed = true; }
        bool isDisposed() { return disposed; }
};

class TargetSpawner {
    private:
        std::vector<Target*> targetsVector;

    public:
        TargetSpawner() {
            
        }

    public:
        void update() {
            for(auto it = targetsVector.begin(); it != targetsVector.end(); ) {
                (*it)->update();
                if((*it)->isDisposed()) {
                    delete *it;
                    it = targetsVector.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void spawnTarget() {
            Target* target = new Target();
            targetsVector.push_back(target);
            if(targetsVector.size() > 5) { // MAX TARGET LEN
                targetsVector.erase(targetsVector.begin());
            }
        }

        Target** getTargets() {
            return targetsVector.data();
        }

        int getTargetCount() {
            return targetsVector.size();
        }

};
TargetSpawner* targetSpawner_p=nullptr;

// made my chatgpt.
int kbhit() {
    termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void worker() {
    while(running) {
        player_p -> read();
        player_p -> update();
        targetSpawner_p -> update();
        screen_p -> draw(targetSpawner_p -> getTargets(), targetSpawner_p->getTargetCount());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void targetSpawnWorker() {
    while(running) {
        targetSpawner_p -> spawnTarget();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void resetChangedSettings(int sig) {
    system("xset r on");
    std::exit(1);
}

int main() {
    system("xset r off");
    signal(SIGINT, resetChangedSettings);

    // set random time ig
    std::srand(std::time(0));
    // initialize objects and pointers
    Screen screen = Screen(50, 16);
    screen_p = &screen;
    Player player = Player(25, 10);
    player_p = &player;
    TargetSpawner targetSpawner = TargetSpawner();
    targetSpawner_p = &targetSpawner;

    // start thread
    std::thread workerThread(worker);
    std::thread targetSpawnThread(targetSpawnWorker);
    workerThread.join();
    targetSpawnThread.join();

    system("xset r on");
    return 0;
}