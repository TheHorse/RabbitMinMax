#include "game.h"
#include "qmath.h"

Game::Game()
    :active(false), selectedMonster(-1)
{
    possibleMoves[0] = QPoint( -1, +1);  //влево вверх
    possibleMoves[1] = QPoint( +1, +1);  //вправо вверх
    possibleMoves[2] = QPoint( -1, -1);  //влево вниз
    possibleMoves[3] = QPoint( +1, -1);  //вправо вниз

    initialize();
}

void Game::reset()
{
    this->active = true;
    initialize();

    if (this->gameMode != MT_RABBIT)
        runMinMax((gameMode == MT_RABBIT) ? MT_WOLF : MT_RABBIT, 0, -EPIC_BIG_VALUE, +EPIC_BIG_VALUE);

    this->playersTurn = true;
}

int Game::getMonsterIndexOnPosition(const QPoint &pos)
{
    if (this->rabbit == pos)
       return 0;

    for (int i = 0; i < 4; i++)
        if (this->wolfs[i] == pos)
           return i + 1;

    return -1;
}

bool Game::canMoveToPosition(int monsterIndex, const QPoint &pos)
{
    Q_ASSERT(monsterIndex >= 0 || monsterIndex <= 4);

    if (!checkRange(pos))
        return false;

    if ((pos.x() + pos.y()) % 2 != 0)
        return false;

    QPoint oldPosition = getMonsterPosition(monsterIndex);
    QPoint diff = oldPosition - pos;

    if (abs(diff.x()) != 1 || abs(diff.y()) != 1)
        return false;

    if (oldPosition == pos)
        return false;

    for (int i = 0; i < getMonsterCount(); i++)
        if (i != monsterIndex && getMonsterPosition(i) == pos)
            return false;

    //волк не может ходить вверх
    if (monsterIndex > 0 && oldPosition.y() > pos.y())
        return false;

    return true;
}

bool Game::isGameOver(MonsterType& winner)
{
    winner = MT_NO_ONE;

    bool canMove = false;
    for (int k = 0; k < 4; k++)
        for (int i = 0; i < 2; i++)
            if (canMoveToPosition(k + 1, this->wolfs[k] + this->possibleMoves[i]))
               canMove = true;

    if (!canMove || this->rabbit.y() == 0)
        winner = MT_RABBIT;

    canMove = false;
    for (int i = 0; i < 4; i++)
        if (canMoveToPosition(0, this->rabbit + this->possibleMoves[i]))
            canMove = true;

    if (!canMove)
        winner = MT_WOLF;

    if (winner != MT_NO_ONE)
    {       
        this->active = false;
        return true;
    }

    return false;
}

bool Game::moveSelectedMonsterToPosition(const QPoint &pos)
{
    if (this->selectedMonster < 0)
        return false;

    if (canMoveToPosition(this->selectedMonster, pos))
        if (this->selectedMonster == 0)
            this->rabbit = pos;
        else
            this->wolfs[this->selectedMonster - 1] = pos;
    else
        return false;


    if (isGameOver())
        return true;

    this->playersTurn = !this->playersTurn;
    runMinMax(this->gameMode == MT_RABBIT ? MT_WOLF : MT_RABBIT, 0, -EPIC_BIG_VALUE, +EPIC_BIG_VALUE);
    this->playersTurn = !this->playersTurn;

    isGameOver();
    return true;
}


void Game::prepareMap()
{
    for (int i = 0; i < 8; i++)
        memset(this->map[i], 0, 8 * sizeof(int));

    this->map[this->rabbit.y()][this->rabbit.x()] = RABBIT;
    for (int i = 0; i < 4; i++)
        this->map[this->wolfs[i].y()][this->wolfs[i].x()] = WOLF;
}

//поиск в ширину, волновой алгоритм.
int Game::getHeuristicEvaluation()
{
    if (this->rabbit.y() == 0)
        return 0;

    this->searchWay.clear();
    this->searchWay.enqueue(this->rabbit);
    while (!this->searchWay.empty())
    {
        QPoint currentPosition = this->searchWay.dequeue(); //current position
        for (int i = 0; i < 4; i++)
            if (canMove(currentPosition + possibleMoves[i]))
            {
                QPoint newPosition = currentPosition + possibleMoves[i]; //new position
                this->map[newPosition.y()][newPosition.x()] = this->map[currentPosition.y()][currentPosition.x()] + 1;
                this->searchWay.enqueue(newPosition);
            }
    }

    int min = MAX_VALUE;
    for (int i = 0; i < 4; i++)
        if ((this->map[0][i * 2] > MIN_VALUE) && (this->map[0][i * 2] < min))
            min = this->map[0][i * 2];

    return min - 1;
}

void Game::temporaryMonsterMovement(int monsterIndex, int x, int y)
{
    if (monsterIndex == 0)
    {
        this->map[this->rabbit.y()][this->rabbit.x()] = EMPTY;
        this->map[this->rabbit.y() + y][this->rabbit.x() + x] = RABBIT;
        this->rabbit += QPoint(x, y);
    } else
    {
        this->map[this->wolfs[monsterIndex - 1].y()][this->wolfs[monsterIndex - 1].x()] = EMPTY;
        this->map[this->wolfs[monsterIndex - 1].y() + y][this->wolfs[monsterIndex - 1].x() + x] = WOLF;
        this->wolfs[monsterIndex - 1] += QPoint(x, y);
    }
}

bool Game::canMove(int x, int y)
{
    if (!checkRange(x,y))
        return false;

    if (map[y][x] != EMPTY)
        return false;

    return true;
}

int Game::runMinMax(MonsterType monster, int recursiveLevel, int alpha, int beta)
{
    if (recursiveLevel == 0)
        this->prepareMap();

    int test = NOT_INITIALIZED;

    //на последнем уровне дерева (на листах) возвращаем значение функции эвристики
    if (recursiveLevel >= this->AILevel * 2)
    {
        int heuristic =  getHeuristicEvaluation();
        prepareMap();
        return heuristic;
    }

    //индекс выбранного пути. 0-7 - возможные ходы волков, 8-11 - возможные хода зайца
    int bestMove = NOT_INITIALIZED;

    bool isWolf = (monster == MT_WOLF);
    int MinMax = isWolf ? MIN_VALUE : MAX_VALUE;

    //перебираем все возможные хода данного монстра
    for (int i = (isWolf ? 0 : 8); i < (isWolf ? 8 : 12); i++)
    {
        int curMonster = isWolf ? i / 2 + 1 : 0;
        QPoint curMonsterPos = curMonster == 0 ? this->rabbit : this->wolfs[curMonster - 1];
        QPoint curMove = this->possibleMoves[isWolf ? i % 2 : i % 4];

        if (canMove(curMonsterPos + curMove))
        {
            //...ходим, после чего обрабатываем ситуацию
            temporaryMonsterMovement(curMonster, curMove);

            //оцениваем, насколько хорош ход, который мы выбрали
            test = runMinMax(isWolf ? MT_RABBIT : MT_WOLF, recursiveLevel + 1, alpha, beta);

            //... и восстанавливаем исходное состояние
            temporaryMonsterMovement(curMonster, -curMove);

            //если он лучше всех, что были до этого - запомним, что он лучший
            if ((test > MinMax && monster == MT_WOLF) || (test <= MinMax && monster == MT_RABBIT) || (bestMove == NOT_INITIALIZED))
            {
                MinMax = test;
                bestMove = i;
            }

            if (isWolf)
                alpha = qMax(alpha, test);
            else
                beta = qMin(beta, test);

            if (beta < alpha)
               break;
        }
    }

    //Для терминальных состояний, когда некуда ходить - возвращаем показания эвристической функции.
    if (bestMove == NOT_INITIALIZED)
    {
        int heuristic =  getHeuristicEvaluation();
        prepareMap();
        return heuristic;
    }

    //ну и собственно, ходим, коль уже выбрали лучший ход
    if (recursiveLevel == 0 && bestMove != NOT_INITIALIZED)
    {
        if (monster == MT_WOLF)
            this->wolfs[bestMove / 2] += this->possibleMoves[bestMove % 2];
        else
            this->rabbit += this->possibleMoves[bestMove % 4];
    }

    return MinMax;
}

void Game::initialize()
{
    this->wolfs[0] = QPoint(0, 0);
    this->wolfs[1] = QPoint(2, 0);
    this->wolfs[2] = QPoint(4, 0);
    this->wolfs[3] = QPoint(6, 0);

    this->rabbit = QPoint(3, 7);
}
