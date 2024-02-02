#include <iostream>
#include <cmath>
#include <ctime>
#include <thread>
#include <chrono>
#include <GL/glut.h>
#include <GL/freeglut.h>

#include "ttt.h"

const int WINDOW_SIZE = 600;
const int BOARD_SIZE = 100;
const int POINTS_TO_WIN = 5;
const float CELL_SIZE = 0.4f;

const float SCALE_INC = 0.1f;
const float MAX_SCALE = 3.5f, MIN_SCALE = 0.8f;

float scale = 2.0f;
float originX = 0.0f;
float originY = 0.0f;
bool isDragging = false;
int startX, startY, lastX, lastY;

bool isMachineMove = false;

enum CursorState { NORMAL, DRAGGING };
CursorState cursorState = NORMAL;

Game* game = nullptr;

void drawO(float x, float y, float r, bool isRed) {
    if (isRed) {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glBegin(GL_POINTS);
    for (float ang = 0.0f; ang < 2.0f * M_PI; ang += 0.001f) {
        float c = cos(ang), s = sin(ang);
        glVertex2f(x + c * r, y + s * r);
    }
    glEnd();
    if (isRed) {
        glColor3f(0.0f, 0.0f, 0.0f);
    }
}

void drawX(float x, float y, float r, bool isRed) {
    if (isRed) {
        glColor3f(1.0f, 0.0f, 0.0f);
    }
    glBegin(GL_LINES);
    glVertex2f(x - r, y - r);
    glVertex2f(x + r, y + r);
    glVertex2f(x - r, y + r);
    glVertex2f(x + r, y - r);
    glEnd();
    if (isRed) {
        glColor3f(0.0f, 0.0f, 0.0f);
    }
}

void drawBoard() {
     // Draw grid lines
    glBegin(GL_LINES);
    float halfCell = CELL_SIZE / 2.0;
    for (int i = 0; i < BOARD_SIZE + 1; ++i) {
        // Horizontal lines
        glVertex2f(-CELL_SIZE * BOARD_SIZE / 2.0 + originX, (i - BOARD_SIZE / 2.0) * CELL_SIZE + originY);
        glVertex2f(CELL_SIZE * BOARD_SIZE / 2.0 + originX, (i - BOARD_SIZE / 2.0) * CELL_SIZE + originY);
        // Vertical lines
        glVertex2f((i - BOARD_SIZE / 2.0) * CELL_SIZE + originX, -CELL_SIZE * BOARD_SIZE / 2.0 + originY);
        glVertex2f((i - BOARD_SIZE / 2.0) * CELL_SIZE + originX, CELL_SIZE * BOARD_SIZE / 2.0 + originY);
    }
    glEnd();
    float centerX, centerY, radius;
    bool isLast = false;
    std::vector<std::vector<int>> board = game->getBoard();
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            centerX = ((float)j - (float)BOARD_SIZE / 2.0 + 0.5) * CELL_SIZE + originX;
            centerY = ((float)BOARD_SIZE / 2.0 - 0.5 - (float)i) * CELL_SIZE + originY;
            radius = CELL_SIZE / 2.5;
            isLast = (lastX == i && lastY == j);
            if (board[i][j] == 1) {
                drawX(centerX, centerY, radius, isLast);
            } else if (board[i][j] == 2) {
                drawO(centerX, centerY, radius, isLast);
            }
        }
    }
    
    glutPostRedisplay();
}

void checkWin() {
    game->getLastMove(lastX, lastY);
    if (lastX != -1 && lastY != -1 && game->checkWin()) {
        if (game->isMoveX()) {
            std::cout << "O won!" << std::endl;
        } else {
            std::cout << "X won!" << std::endl;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        drawBoard();
        glFlush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::vector<std::vector<int>> board = game->getBoard();
        for (int i = 40; i < 60; ++i) {
            for (int j = 40; j < 60; ++j) {
                std::cout << board[i][j] << " ";
            }
            std::cout << std::endl;
        }
        exit(0);
    }
}

void machineMove() {
    game->machineMove();
    checkWin();
    isMachineMove = false;
}

void myInit(void) {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glColor3f(0.0f, 0.0f, 0.0f);
    glPointSize(3.0);
    glLineWidth(3.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-scale, scale, -scale, scale);
    game = new Game(BOARD_SIZE, POINTS_TO_WIN);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) // ESC key
        exit(0);
}

void mouseWheel(int button, int dir, int x, int y) {
    if (dir > 0 && scale < MAX_SCALE) {
        scale -= SCALE_INC;
    } else if (dir < 0 && scale > MIN_SCALE) {
        scale += SCALE_INC;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-scale, scale, -scale, scale);

    glutPostRedisplay();
}

void setCursor(CursorState state) {
    switch (state) {
        case NORMAL:
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
            break;
        case DRAGGING:
            glutSetCursor(GLUT_CURSOR_INFO);
            break;
        default:
            break;
    }
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !isMachineMove) {
        float relativeX = (float)x / WINDOW_SIZE * (2.0 * scale) - scale - originX;
        float relativeY = (float)(WINDOW_SIZE - y) / WINDOW_SIZE * (2.0 * scale) - scale - originY;
        float boardNotScaledX = ((CELL_SIZE * (float)BOARD_SIZE / 2.0) - relativeY);
        float boardNotScaledY = ((CELL_SIZE * (float)BOARD_SIZE / 2.0) + relativeX);
        if (boardNotScaledX < 0 || 
            boardNotScaledX > CELL_SIZE * BOARD_SIZE || 
            boardNotScaledY < 0 || 
            boardNotScaledY > CELL_SIZE * BOARD_SIZE) {
            std::cout << "out of bounds" << std::endl;
        } else {
            int gridX = boardNotScaledX / CELL_SIZE;
            int gridY = boardNotScaledY / CELL_SIZE;
            game->move(gridX, gridY);
            checkWin();
            isMachineMove = true;
        }
    } else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            startX = x;
            startY = y;
            cursorState = DRAGGING;
            setCursor(cursorState);
        } else if (state == GLUT_UP) {
            isDragging = false;
            cursorState = NORMAL;
            setCursor(cursorState);
        }
    }
}

void mouseMotion(int x, int y) {
    if (isDragging) {
        originX += (x - startX) * (2.0 * scale) / glutGet(GLUT_WINDOW_WIDTH);
        originY -= (y - startY) * (2.0 * scale) / glutGet(GLUT_WINDOW_HEIGHT);
        startX = x;
        startY = y;

        glutPostRedisplay();
    }
}

void myDisplay(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    drawBoard();
    glFlush();
    if (isMachineMove) {
        machineMove();
    }
}

int main(int argc, char** argv) {
    std::srand(std::time(0));  
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);
    glutInitWindowPosition(400, 200);
    glutCreateWindow("Tic-Tac-Toe Field");
    myInit();
    glutKeyboardFunc(keyboard);
    glutMouseWheelFunc(mouseWheel);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    setCursor(cursorState);
    glutDisplayFunc(myDisplay);
    glutMainLoop();
    return 0;
}
