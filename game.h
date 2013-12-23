#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QPoint>
#include <QQueue>
#include <QList>

#define MIN_VALUE 0
#define MAX_VALUE 255
#define EPIC_BIG_VALUE 500

class Game
{    
public:
    Game();

    enum MonsterType {
        MT_NO_ONE = 0,
        MT_RABBIT = 1,
        MT_WOLF = 2
    };

    void setActive(bool active) { this->active = active; }
    bool isActive() { return this->active; }

    void setAILevel(int AILevel) { this->AILevel = AILevel; }
    void setPlayMode(MonsterType gameMode) { this->gameMode = gameMode;  }
    int getMonsterCount() { return 5; }
    MonsterType getPlayMode() { return this->gameMode; }

    QPoint& getRabbitPosition() { return rabbit; }
    QPoint& getWolfPosition(int wolfIndex)
    {
        Q_ASSERT(wolfIndex < 4 && wolfIndex >= 0);
        return wolfs[wolfIndex];
    }

    bool checkRange(int x, int y) { return (x >= 0 && y >= 0 && x <= 7 && y <= 7); }
    bool checkRange(const QPoint& point) { return checkRange(point.x(), point.y()); } 

    QPoint& getMonsterPosition(int monsterIndex)
    {
        if (monsterIndex == 0)
            return getRabbitPosition();
        else
            return getWolfPosition(monsterIndex - 1);
    }

    MonsterType getMonsterType(int monsterIndex)
    {
        if (monsterIndex == 0)
            return MT_RABBIT;
        else
            return MT_WOLF;
    }    

    int getSelectedMonsterIndex() { return selectedMonster;  }
    void setSelectedMonsterIndex(int monsterIndex) { this->selectedMonster = monsterIndex; }
    int isPlayersTurn() { return playersTurn; }

    //Старт / рестарт игры
    void reset();

    //Возвращает индекс монстра, который находится в указанной клеточке, либо -1
    int getMonsterIndexOnPosition(const QPoint& pos);

    //true если указанный монстр может пойти на указанную клеточку
    bool canMoveToPosition(int monsterIndex, const QPoint& pos);

    //перемещает помеченного пользователем монстра на указанную позицию
    bool moveSelectedMonsterToPosition(const QPoint& pos);

    bool isGameOver(MonsterType& winner);
    bool isGameOver() { MonsterType winner; return isGameOver(winner); }

private:
    static const int NOT_INITIALIZED = 255;

    static const int EMPTY = 0;
    static const int RABBIT = 1;
    static const int WOLF = 255;

    QPoint wolfs[4];
    QPoint rabbit;
    MonsterType gameMode;

    int map[8][8];
    QQueue<QPoint> searchWay;
    QPoint possibleMoves[4];

    bool active;
    bool playersTurn;
    int AILevel;
    int selectedMonster;

    void initialize();

    //простая проверка на свободность и существование ячейки
    bool canMove(int x, int y);
    bool canMove(const QPoint& point) { return canMove(point.x(), point.y()); }

    //возращает эвристическую оценку вероятности победы. чем больше, тем вероятней что победят волки
    int getHeuristicEvaluation();

    //собственно, сам алгоритм, выполняет ход исскуственного интеллекта
    int runMinMax(MonsterType monster, int recursiveLevel, int alpha, int beta);

    //используется минимаксом для локальной перестановки монстров на карте
    void temporaryMonsterMovement(int monsterIndex, int x, int y);
    void temporaryMonsterMovement(int monsterIndex, const QPoint& point) { temporaryMonsterMovement(monsterIndex, point.x(), point.y()); }

    //подготовить карту для работы с эвристической функцией и проверки на допустимость передвижений
    void prepareMap();
};

#endif // GAME_H
