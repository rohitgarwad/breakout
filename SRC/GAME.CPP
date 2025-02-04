#include "GAME.H"
#include "KEYBRD.H"
#include "PADDLE.H"
#include "RENDERER.H"
#include "BALL.H"
#include "CONF.H"
#include "AABB.H"
#include "LEVELS.H"
#include "UI.H"

static const int brickScoreTable[16] = {0,0,10,0,30,0,40,0,0,0,0,0,20,0,10,0};

Game::Game() {
    paddle = new Paddle(SCREEN_WIDTH - 50, SCREEN_HEIGHT - 10, MAGENTA);
    
    ball = new Ball(paddle->pos.x + (paddle->width >> 1),  paddle->pos.y - 5, WHITE);

    renderer = new Renderer();

    ui = new UI();

    initkb();

    GameLevel one;
    one.init(levelOne);
    levels[0] = one;

    GameLevel two;
    two.init(levelTwo);
    levels[1] = two;

    GameLevel three;
    three.init(levelThree);
    levels[2] = three;

    GameLevel four;
    four.init(levelFour);
    levels[3] = four;
    
    currentLevel = 0;

    score = 0;

    life = PLAYER_LIFE;

    isRunning = 1;

    done = 0;
}

Game::~Game() {
    delete paddle;
    
    delete ball;

    delete ui;

    delete renderer;

    exitkb();
}

void Game::loop(void) {
    while (isRunning) {
        processInput();

        if (!done) {
            update();
            render();
        }
    }
}

void Game::processInput(void) {
    char key = kbhit();

    if (key == ESC) {
        isRunning = 0;
    } else if (key == L_ARROW) {
        paddle->move(LEFT);
    } else if (key == R_ARROW) {
        paddle->move(RIGHT);
    }
}

void Game::update(void) {
    ui->update(score, life);

    ball->move();

    doCollision();
}

void Game::render(void) {
    renderer->clear(BLACK);

    ui->draw(renderer);
    
    ball->draw(renderer);
    
    paddle->draw(renderer);

    levels[currentLevel].draw(renderer);

    renderer->updateBuffer();
}

void Game::doCollision(void) {
     if (ball->pos.y + ball->height >= SCREEN_HEIGHT) {
        ball->reset(paddle->pos.x + (paddle->width >> 1), paddle->pos.y - 5);
        
        life--;
        
        if (life == 0) {
            done = 1;
        }
    } else if (checkCollision(ball, paddle)) {
        ball->velocity.y = -ball->velocity.y;
        ball->pos.y = paddle->pos.y - 5;
    } else {
        GameLevel* level = &levels[currentLevel];
    
        for (int i = 0; i < BRICK_COUNT; ++i) {
            GameObject* brick = &level->bricks[i];
            
            if (!brick->isActive) continue;

            if (checkAABBCollision(ball, brick)) {
                brick->hitCount--;
                    
                if (brick->hitCount == 0) {
                    brick->isActive = 0;
                    
                    score += brickScoreTable[brick->_color];
                    
                    level->update();
                    
                    if (level->isFinished()) {
                        currentLevel++;
                        ball->reset(paddle->pos.x + (paddle->width >> 1), paddle->pos.y - 5);
                        break;
                    }
                }
                    
                ball->velocity.y = -ball->velocity.y;
            }
        }
    }
}