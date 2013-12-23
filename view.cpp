#include "view.h"
#include "ui_view.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

View::View(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::View)
{
    ui->setupUi(this);
    this->setWindowTitle("Rabbit&Wolfs");

    QTimer* updater = new QTimer(this);
    updater->start(30);

    connect(ui->pbPlay, SIGNAL(clicked()), SLOT(pbPlayClicked()));
    connect(updater, SIGNAL(timeout()), SLOT(update()));
}

void View::setGame(Game* game)
{
    this->game = game;    
}

View::~View()
{
    delete ui;
}

void View::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (!this->game->isActive() || !this->game->isPlayersTurn())
        return;

    this->realMousePosition = e->pos();
    int monsterIndex = this->game->getMonsterIndexOnPosition((e->pos() - QPoint(25, 25)) / 50);
    if (monsterIndex < 0)
        return;

    if (this->game->getMonsterType(monsterIndex) != this->game->getPlayMode())
        return;

    this->game->setSelectedMonsterIndex(monsterIndex);
}

void View::mouseMoveEvent(QMouseEvent *e)
{
    this->realMousePosition = e->pos();
    this->mousePosition = (e->pos() - QPoint(25, 25)) / 50;
}

void View::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (!this->game->isActive())
        return;

    this->realMousePosition = e->pos();
    this->game->moveSelectedMonsterToPosition((e->pos() - QPoint(25, 25)) / 50);
    this->game->setSelectedMonsterIndex(-1);

    Game::MonsterType winner;
    if (this->game->isGameOver(winner))
        QMessageBox::information(NULL, "Attention!", (winner == this->game->getPlayMode() ? "You win" : "Game over"));

}

void View::pbPlayClicked()
{
    this->ui->pbPlay->setText("Restart");

    this->game->setPlayMode(ui->rbRabbit->isChecked() ? Game::MT_RABBIT : Game::MT_WOLF);
    this->game->setAILevel(ui->sbAILevel->value());
    this->game->reset();
}

void View::paintEvent(QPaintEvent *)
{    
    QPainter p(this);
    p.setPen(Qt::NoPen);

    //draw grid
    p.setBrush(QBrush(QColor(40,40,40)));
    for (int i = 0; i < 8; i++)
        for (int k = 0; k < 8; k++)
            if ((i + k) % 2 == 0) p.drawRect(i * 50, k * 50, 50, 50);

    p.setBrush(QBrush(QColor(210,210,210)));
    for (int i = 0; i < 8; i++)
        for (int k = 0; k < 8; k++)
            if ((i + k) % 2 != 0) p.drawRect(i * 50, k * 50, 50, 50);

    p.setPen(Qt::SolidLine);
    p.drawLine(400, 0, 400, 400);

    //draw monsters
    for (int i = 0; i < this->game->getMonsterCount(); i++)
        if (i != this->game->getSelectedMonsterIndex())
        {
            p.setBrush(QBrush(QColor(this->game->getMonsterType(i) == Game::MT_WOLF ? 200 : 40, 40, this->game->getMonsterType(i) == Game::MT_RABBIT ? 200 : 40)));
            p.drawEllipse(this->game->getMonsterPosition(i) * 50 + QPoint(25,25) , 20, 20);
        }


    if (this->game->getSelectedMonsterIndex() >= 0)
        if (!(this->mousePosition.x() < 0 || this->mousePosition.y() < 0 || this->mousePosition.x() > 7 || this->mousePosition.y() > 7))
            if (this->game->canMoveToPosition(this->game->getSelectedMonsterIndex(), this->mousePosition))
            {
                p.setPen(QPen(QBrush(Qt::white), 5));
                p.setBrush(QBrush(QColor(210, 210, 210), Qt::NoBrush));
                p.drawRect(QRect(this->mousePosition * 50, QSize(50, 50)));
            }

    if (game->getSelectedMonsterIndex() >= 0)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(this->game->getMonsterType(this->game->getSelectedMonsterIndex()) == Game::MT_WOLF ? 255 : 40, 40,
                                 this->game->getMonsterType(this->game->getSelectedMonsterIndex()) == Game::MT_RABBIT ? 255 : 40)));
        p.drawEllipse(this->realMousePosition , 20, 20);
    }
}

