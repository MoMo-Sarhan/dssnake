#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#ifdef __linux__
#include <X11/Xlib.h>

#include <thread>
#elif _WIN32
#include <windows.h>
#endif
#include <math.h>

#include <cmath>
#include <iostream>
#include <vector>
using namespace std;

#define PI 3.17
#ifdef __linux__
Display* d = XOpenDisplay(NULL);
Screen* s = DefaultScreenOfDisplay(d);
const int Hight = s->height / 2;
const int Width = s->width / 2;
#elif _WIN32
const int Hight = GetSystemMetrics(SM_CYSCREEN) / 2;
const int Width = GetSystemMetrics(SM_CXSCREEN) / 2;
#endif

struct point {
    float x;
    float y;
    bool down = false;
    bool bomb = false;
};

//+++++++++++++++Snake++++++++++++++++++++++++++++++++++++++++
enum Direction {
    Top = 1,
    Down,
    Left,
    Right,
    Left_Top,
    Left_Down,
    Right_Top,
    Right_Down
};

// properties of snake
struct Snake {
    int head_X, head_Y;
    int tailLength = 1;
    point* tail = new point[100];
    Direction direction = Top;
};

Snake snake;
void shiftTail(point arr[], int size_of_shape);
void move(Snake* snake);
void drawSnake(Snake snake);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++Triangle++++++++++++++++++++++++++

bool triange_top = false;
point* triangle_shoot = new point[100];
float tx = -Width + 100, ty = -200, delta_ty = 10;
int shoot_number = -1;
int life_count = 0;
point* life = new point[3];
void DrawTriangle(float tx, float ty);
void Move_shoot();
void reset_shoot();

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++
// size_of_shape of shape
int size_of_shape = (Width / 30) * (Hight / 2) / 30;

// count of touched points
int count = size_of_shape;
int level_count = 0;

// count of snake foods
int count_2 = 0;
// initialize the size_of_shape of shape
point* Points = new point[size_of_shape];
float sxL = 0, syL = 0, xp = 0, yp = 0, xq = 0, yq = 0, xL = 0, yL = -Hight,
      deltaxq = 10, deltayq = 10, deltaxL = 10, deltayL = 10, deltasy = 10,
      deltasx = 10;
float cx = 0, cy = 0, deltaPi, radius = 50;
// to hide the points
float special_end = max(Hight, Width) * 2;
// delta of circle
float deltaxp = 10, deltayp = 10;

bool KeyBoard[256];
bool top_End, buttom_End, left_End, right_End;
bool fullScreen, go_Left, go_Up;
bool no_die = false;
bool enable_music = true;
bool hold_ball = false;
bool enable_pause;

void initMatrix();
void draw();
void Reshape(int, int);
void backGround();
void timer(int);
void keyBoardDown(unsigned char, int, int);
void keyBoardUp(unsigned char, int, int);
void keySpecialDown(int, int, int);
void keySpecialUp(int, int, int);
void Mouse(int, int, int, int);
void playSound(int i);
void playMusic(int i);
void endGame();
void pause(bool);

int main(int argc, char** argv) {
    snake.head_X = 0;
    snake.head_Y = 0;
    snake.tailLength = 10;
    snake.direction = Right;
    initMatrix();

    cout << Hight << endl;
    cout << Width << endl;
    // glut init
    glutInit(&argc, argv);
    glutInitWindowSize(Width, Hight);
    // create window
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(20, 20);
    glutInitWindowPosition(500, 500);
    glutCreateWindow("DsSnake Game");
    // draw
    glutDisplayFunc(draw);
    glutReshapeFunc(Reshape);
    backGround();

    glutKeyboardFunc(keyBoardDown);
    glutKeyboardUpFunc(keyBoardUp);
    glutSpecialFunc(keySpecialDown);
    glutSpecialUpFunc(keySpecialUp);
    glutMouseFunc(Mouse);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();

    return 0;
}
void DrawMyCircle(float cx, float cy, float r, float count, int color) {
    glBegin(GL_TRIANGLE_FAN);
    if (color == 0)
        glColor3f(.8, .7, .3);
    else
        glColor3f(0, 0, 0);

    glVertex2f(cx, cy);
    for (float i = 0; i <= (2 * PI) * ((float)count / size_of_shape);
         i += PI / 500) {
        glVertex2f(cx + cos(i) * r, cy + sin(i) * r);
    }
    glEnd();
}
void draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glEnable(GL_POINT_SMOOTH);
    //    glPointSize(5);

    glBegin(GL_POINTS);
    glColor3f(1, 0, 0);
    glLoadIdentity();
    for (int i = 0; i < size_of_shape; i++) {
        if (Points[i].bomb)
            glColor3f(0, 0, 1);
        else
            glColor3f(1, 0, 0);

        if (Points[i].x != special_end && Points[i].y != special_end)
            glVertex2f(Points[i].x, Points[i].y);
    }
    glEnd();
    DrawMyCircle(0, 0, 100, count, 0);

    if (level_count > 1) DrawMyCircle(0, -200, 100, count_2, 1);
    if (level_count > 1) drawSnake(snake);
    if (level_count == 3) DrawTriangle(tx, ty);

    glutSwapBuffers();
    glEnable(GL_POINT_SMOOTH);
    glLoadIdentity();
    glPointSize(10);

    // for the ball
    glBegin(GL_POINTS);
    glColor3f(0, 0, 1);
    glVertex2f(xp, yp);
    glEnd();

    glutSwapBuffers();

    // for stick
    glColor3f(1, 0.5, 1);
    glLineWidth(50);
    glBegin(GL_LINES);
    glVertex2f(xL, yL);
    glVertex2f(xL + 500, yL);
    glEnd();

    // draw lifes
    glBegin(GL_POINTS);
    glColor3f(1, 0 ,1);
    for (int i = 2; i >= life_count; i--) {
        glVertex2f(life[i].x, life[i].y);
    }
    glEnd();

    glColor3f(0, 0, 1);
    glLineWidth(.3);
    // border for snake
    glBegin(GL_LINES);
    glVertex2f(-Width, Hight / 2.0);
    glVertex2f(Width, Hight / 2.0);
    glEnd();

    glutSwapBuffers();
}
void Reshape(int w, int h) {
    float ratio = (float)w / h;
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-Width, Width, -Hight, Hight);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void backGround() { glClearColor(.1, .4, .3, 0); }
void timer(int v) {
    glutPostRedisplay();
    glutTimerFunc(10 / .5, timer, 0);

    int tmp_tailLength = snake.tailLength;
    //=======================================================
    for (int i = 0; i < size_of_shape; i++) {
        // for the ball
        if (xp == Points[i].x && yp == Points[i].y) {
            Points[i].down = true;
            top_End = !top_End;
            if (count > 2) count--;
            // for bomb
            if (Points[i].bomb == true && (i > 0 || i < size_of_shape - 1)) {
                Points[i - 1].down = true;
                Points[i + 1].down = true;
                if (i + (Width * 2) / 30 < size_of_shape)
                    Points[i + (Width * 2) / 30].down = true;
                if (i - (Width * 2) / 30 >= 0)
                    Points[i - (Width * 2) / 30].down = true;
                playSound(4);
            }
        }
        // to fall the touched point
        if (Points[i].down) Points[i].y -= deltasy;
        if (snake.head_X == Points[i].x && snake.head_Y == Points[i].y) {
            Points[i].y = special_end;
            Points[i].x = special_end;
            if (snake.tailLength <= 90) snake.tailLength += 10;
        }
        // for the snake
        for (int j = 0; j < tmp_tailLength; j++) {
            if (snake.tail[j].x == Points[i].x &&
                snake.tail[j].y == Points[i].y) {
                Points[i].y = special_end;
                Points[i].x = special_end;
                if (snake.tailLength <= 90) snake.tailLength++;
                if (count_2 < size_of_shape - 2) count_2++;
                if (enable_music) playSound(1);
            }
        }

        // for triangle
        for (int j = 0; j < 100; j++) {
            if (triangle_shoot[j].x == Points[i].x &&
                triangle_shoot[j].y == Points[i].y) {
                Points[i].y = special_end;
                Points[i].x = special_end;
                count_2++;
            }
        }
    }
    //=======================================================
    if (xp >= xL && xp <= xL + 500 && yp == yL) {
        top_End = false;
    }
    if ((xp < xL || xp > xL + 500) && yp == -Hight && no_die) {
        cout << no_die << endl;
        cout << life_count << endl;
        if (no_die) life_count++;
        if (life_count >= 3) endGame();
    }

    if (xp == Width) right_End = true;
    if (xp == -Width) right_End = false;
    if (right_End)
        xp -= deltaxp;
    else
        xp += deltaxp;

    if (yp == Hight) top_End = true;

    if (yp == -Hight) top_End = false;

    if (top_End)
        yp -= deltayp;
    else
        yp += deltayp;

    if (go_Left && xL > -Width) {
        xL -= deltaxL;
    } else {
        if (xL < Width - 500) xL += deltaxL;
    }

    // for snake
    move(&snake);

    // triangle
    if (ty + 100 == Hight / 2.0) triange_top = true;
    if (ty - 100 == -Hight) triange_top = false;
    if (triange_top)
        ty -= delta_ty;
    else
        ty += delta_ty;
    for (int j = 0; j < 100; j++) {
        if (triangle_shoot[j].down) {
            triangle_shoot[j].x += delta_ty;
        }
        if (triangle_shoot[j].x == Width) {
            if (shoot_number >= 0) shoot_number--;
            triangle_shoot[j].x = tx;
            triangle_shoot[j].down = false;
        }
    }
}
void move(Snake* snake) {
    // shift the tail
    shiftTail(snake->tail, snake->tailLength);
    snake->tail[0].x = snake->head_X;
    snake->tail[0].y = snake->head_Y;

    switch (snake->direction) {
        case Top:
            if (snake->head_Y == Hight / 2) snake->head_Y = -Hight;
            snake->head_Y += deltasy;
            break;
        case Down:
            if (snake->head_Y == -Hight) snake->head_Y = Hight / 2;
            snake->head_Y -= deltasy;
            break;
        case Right:
            if (snake->head_X == Width) snake->head_X = -Width;
            snake->head_X += deltasx;
            break;
        case Left:
            if (snake->head_X == -Width) snake->head_X = Width;
            snake->head_X -= deltasx;
            break;
        case Left_Top:
            if (snake->head_Y == Hight / 2) snake->head_Y = -Hight;
            snake->head_Y += deltasy;
            if (snake->head_X == -Width) snake->head_X = Width;
            snake->head_X -= deltasx;
            break;
        case Left_Down:
            if (snake->head_X == -Width) snake->head_X = Width;
            snake->head_X -= deltasx;
            if (snake->head_Y == -Hight) snake->head_Y = Hight / 2;
            snake->head_Y -= deltasy;
            break;
        case Right_Top:
            if (snake->head_X == Width) snake->head_X = -Width;
            snake->head_X += deltasx;
            if (snake->head_Y == Hight / 2) snake->head_Y = -Hight;
            snake->head_Y += deltasy;
            break;
        case Right_Down:
            if (snake->head_X == Width) snake->head_X = -Width;
            snake->head_X += deltasx;
            if (snake->head_Y == -Hight) snake->head_Y = Hight / 2;
            snake->head_Y -= deltasy;
            break;
    }
}
void shiftTail(point arr[], int size_of_shape) {
    for (int i = size_of_shape - 2; i >= 0; i--) {
        arr[i + 1].x = arr[i].x;
        arr[i + 1].y = arr[i].y;
    }
}
void drawSnake(Snake snake) {
    glBegin(GL_POINTS);
    glColor3f(0, 0, 0);
    for (int i = 0; i < snake.tailLength; i++) {
        glVertex2f(snake.tail[i].x, snake.tail[i].y);
    }
    glEnd();
}

void Move_shoot() {
    triangle_shoot[shoot_number].down = true;
    triangle_shoot[shoot_number].x = tx;
    triangle_shoot[shoot_number].y = ty;
}
void initMatrix() {
    // for shape
    float tmp_height = Hight;
    float tmp_widht = -Width;
    // init shape
    for (int i = 0; i < size_of_shape; i++) {
        if (tmp_widht == Width) {
            tmp_height -= 30;
            tmp_widht = -Width;
        }
        Points[i].x = tmp_widht += 30;
        Points[i].y = tmp_height;
    }

    // init bomb
    srand(time(0));
    for (int i = 0; i < 50; i++) {
        int index = (rand() % size_of_shape) - 1;
        Points[index].bomb = true;
    }
    // init trinagle shoot
    for (int i = 0; i < 100; i++) {
        triangle_shoot[i].x = tx;
        triangle_shoot[i].x = ty;
    }
    // init life
    life[0].x = -Width / 2.0;
    life[0].y = 0;

    life[1].x = 0;
    life[1].y = Hight / 4.0;

    life[2].x = Width / 2.0;
    life[2].y = 0;
}
void DrawTriangle(float tx, float ty) {
    glBegin(GL_TRIANGLES);
    glVertex2f(tx, ty);
    glVertex2f(tx - 100, ty + 100);
    glVertex2f(tx - 100, ty - 100);
    glEnd();
    glBegin(GL_POINTS);
    glColor3f(1, 1, 1);
    for (int i = 0; i < 100; i++) {
        if (triangle_shoot[i].down)
            glVertex2f(triangle_shoot[i].x, triangle_shoot[i].y);
    }
    glEnd();

    //    jfkaf
}
void reset_shoot() {
    for (int i = 0; i < 100; i++) {
        triangle_shoot[i].x = tx;
        triangle_shoot[i].y = ty;
        triangle_shoot[i].down = false;
    }
    shoot_number = -1;
}
void playSound(int i) {
    thread t(playMusic, i);
    t.join();
    // jkfakj
}

void playMusic(int i) {
    switch (i) {
        case 1:
            // snake eats
            if (level_count > 1)
                system(
                    "test -z `pgrep mpv`  && mpv --no-video --start=00:00:04 "
                    "--end=00:00:09 output.mp3 >/dev/null &");
            break;
            // for fire
        case 2:
            if (level_count == 3)
                system(
                    "mpv --no-video --start=00:00:00.2 "
                    "--end=00:00:01 pew.mp3 >/dev/null &");
            break;
            // for reset
        case 3:
            if (level_count == 3)
                system(
                    "mpv --no-video --start=00:00:09 "
                    "--end=00:00:09 fire.mp3 >/dev/null &");

            break;
            // for bom
        case 4:
            system(
                "mpv --no-video --start=00:00:02 "
                "--end=00:00:03 bomb.mp3 >/dev/null &");
            break;
    }
}
void endGame() {
    system("pkill mpv");
    exit(0);
}
void pause(bool enable) {
    if (enable) {
        deltaxL = 0;
        deltayL = 0;
        deltaxp = 0;
        deltayp = 0;
        deltasy = 0;
        deltasx = 0;
        delta_ty = 0;
        enable_music = false;
    } else {
        deltaxL = 10;
        deltayL = 10;
        deltaxp = 10;
        deltayp = 10;
        deltasy = 10;
        deltasx = 10;
        delta_ty = 10;
        enable_music = true;
    }
    // kljfaf
}

void keyBoardDown(unsigned char key, int x, int y) {
    KeyBoard[key] = true;

    if (KeyBoard['h']) {
        go_Left = true;
    }
    if (KeyBoard['l']) {
        go_Left = false;
    }
    if (KeyBoard['k'] && yL < (Hight / 2.0)) {
        yL += deltayL;
    }
    if (KeyBoard['j'] && yL > -Hight) {
        yL -= deltayL;
    }
    if (KeyBoard[27]) {
        system("pkill mpv");
        exit(0);
    }

    if (KeyBoard['a'] && KeyBoard['w']) {
        snake.direction = Left_Top;
    }
    if (KeyBoard['a'] && KeyBoard['s']) {
        snake.direction = Left_Down;
    }
    if (KeyBoard['d'] && KeyBoard['w']) {
        snake.direction = Right_Top;
    }
    if (KeyBoard['d'] && KeyBoard['s']) {
        snake.direction = Right_Down;
    }

    // Direction of snake
    if (KeyBoard['a']) {
        //        if (snake.direction != Right)
        snake.direction = Left;
    }
    if (KeyBoard['d']) {
        //        if (snake.direction != Left)
        snake.direction = Right;
    }
    if (KeyBoard['w']) {
        //        if (snake.direction != Down)
        snake.direction = Top;
    }
    if (KeyBoard['s']) {
        //        if (snake.direction != Top)
        snake.direction = Down;
    }

    // to shoot the fire
    if (KeyBoard[' ']) {
        if (shoot_number <= 99) shoot_number++;
        Move_shoot();
        if (enable_music) playSound(2);
    }
    if (KeyBoard['r']) {
        if (enable_music) playSound(3);
        reset_shoot();
    }
    if (KeyBoard['n']) {
        no_die = !no_die;
    }
    if (KeyBoard['m']) {
        system("pkill mpv");
        enable_music = !enable_music;
    }
    if (KeyBoard['t']) {
        hold_ball = !hold_ball;
    }
    if (KeyBoard['p']) {
        enable_pause = !enable_pause;
        pause(enable_pause);
    }
    if (KeyBoard['e']) {
        level_count %= 3;
        level_count++;
        if (level_count == 1) system("pkill mpv");
        cout << level_count << endl;
    }
}
void keyBoardUp(unsigned char key, int x, int y) { KeyBoard[key] = false; }
void keySpecialDown(int key, int x, int y) {
    KeyBoard[key] = false;
    if (key == GLUT_KEY_F11) {
        fullScreen = !fullScreen;
        if (fullScreen) {
            glutFullScreen();
        } else {
            glutReshapeWindow(Width, Hight);
            glutPositionWindow(Width / 2, Hight / 2);
        }
    }
    if (key == GLUT_KEY_LEFT) {
        go_Left = true;
    }
    if (key == GLUT_KEY_RIGHT) {
        go_Left = false;
    }
    if (key == GLUT_KEY_UP && yL < (Hight / 2.0)) {
        yL += deltayL;
    }
    if (key == GLUT_KEY_DOWN && yL > -Hight) {
        yL -= deltayL;
    }
}
void keySpecialUp(int key, int x, int y) {}
void Mouse(int key, int stat, int x, int y) {
    int new_x = (((x - (Width / 2)) * 10) / (Width / 2));
    int new_y = (((y - (Hight / 2)) * 10) / (Hight / 2));
}

