#include <glut.h>
#include <math.h>
#include <stdlib.h>

// untuk circle 
static const int SEGMENTS = 72;

// movement dan physics state
static float g_posX = 0.0f;      // horizontal offset
static float g_posY = 0.0f;      // vertical offset (jump)
static float g_velY = 0.0f;      // vertical velocity
static bool  g_inAir = false;    // jumping state
static bool  g_moveLeft = false;
static bool  g_moveRight = false;
static const float g_speed = 120.0f;   // units/sec
static const float g_jumpVel = 260.0f; // initial jump velocity
static const float g_gravity = -900.0f; // units/sec^2
static int g_lastTimeMs = 0;

// shape srimitives yang dipakai ulang
static void filledCircle(float cx, float cy, float r, unsigned char R, unsigned char G, unsigned char B) {
    glColor3ub(R, G, B);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= SEGMENTS; ++i) {
        float a = (float)i / SEGMENTS * 6.28318530718f;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void circleOutline(float cx, float cy, float r, float width, unsigned char R, unsigned char G, unsigned char B) {
    glLineWidth(width);
    glColor3ub(R, G, B);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < SEGMENTS; ++i) {
        float a = (float)i / SEGMENTS * 6.28318530718f;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void filledRect(float x1, float y1, float x2, float y2, unsigned char R, unsigned char G, unsigned char B) {
    glColor3ub(R, G, B);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

static void line(float x1, float y1, float x2, float y2, float width, unsigned char R, unsigned char G, unsigned char B) {
    glLineWidth(width);
    glColor3ub(R, G, B);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

static void triangle(float x1, float y1, float x2, float y2, float x3, float y3, unsigned char R, unsigned char G, unsigned char B) {
    glColor3ub(R, G, B);
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

// function part tubuh spongebob
static void drawEye(float cx, float cy) {
    filledCircle(cx, cy, 26, 255, 255, 255);
    circleOutline(cx, cy, 26, 2.0f, 0, 0, 0);
    filledCircle(cx, cy, 12, 102, 178, 255); // iris
    filledCircle(cx, cy, 6, 0, 0, 0);        // pupil
}

// dir: -1 untuk left eye, +1 untuk right eye
static void drawEyelashes(float cx, float cy, int dir) {
    line(cx, cy + 28, cx, cy + 45, 2.0f, 0, 0, 0); // center lash
    // outer lash (goes outward)
    line(cx + 15 * dir, cy + 25, cx + 25 * dir, cy + 40, 2.0f, 0, 0, 0);
    // inner lash (goes inward)
    line(cx - 15 * dir, cy + 25, cx - 25 * dir, cy + 40, 2.0f, 0, 0, 0);
}

static void drawPore(float cx, float cy, float r) {
    filledCircle(cx, cy, r, 204, 178, 0);
}

static void drawBeltLoop(float xLeft) {
    filledRect(xLeft, -20, xLeft + 10, -5, 0, 0, 0);
}

static void drawTieAtWhiteBand() {
    const float s = 1.5f;          // skala dasi
    const float ax = 0.0f, ay = 20.0f; // anchor di tepi atas (sejajar kotak putih)

    // knot (rect) diskalakan mengelilingi anchor
    float x1 = ax + (-5.0f - ax) * s;
    float y1 = ay + (5.0f - ay) * s;
    float x2 = ax + (5.0f - ax) * s;
    float y2 = ay + (20.0f - ay) * s;

    // segitiga untuk knotnya
    glColor3ub(200, 0, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y2);      // kiri alas (sejajar y=20)
    glVertex2f(x2, y2);      // kanan alas (sejajar y=20)
    glVertex2f(ax, y1);      // puncak
    glEnd();

    // outline knot
    glColor3ub(0, 0, 0);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y2);
    glVertex2f(x2, y2);
    glVertex2f(ax, y1);
    glEnd();

    // diamond (ujung dasi) diskalakan mengelilingi anchor
    float v0x = ax + (0.0f - ax) * s, v0y = ay + (20.0f - ay) * s; // atas
    float v1x = ax + (12.0f - ax) * s, v1y = ay + (5.0f - ay) * s; // kanan
    float v2x = ax + (0.0f - ax) * s, v2y = ay + (-15.0f - ay) * s; // bawah
    float v3x = ax + (-12.0f - ax) * s, v3y = ay + (5.0f - ay) * s; // kiri

    glColor3ub(200, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(v0x, v0y);
    glVertex2f(v1x, v1y);
    glVertex2f(v2x, v2y);
    glVertex2f(v3x, v3y);
    glEnd();

    // outline diamond
    glColor3ub(0, 0, 0);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(v0x, v0y);
    glVertex2f(v1x, v1y);
    glVertex2f(v2x, v2y);
    glVertex2f(v3x, v3y);
    glEnd();
}

static void drawMouthLowerArc(float cx, float cy, float r, float width) {
    glColor3ub(0, 0, 0);
    glLineWidth(width);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= SEGMENTS; ++i) {
        float a = 3.14159265f + (3.14159265f * (float)i / SEGMENTS); //2 pi supaya arc bawah
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

static void drawTeeth(float y1, float y2) {
    filledRect(-10, y1, -2, y2, 255, 255, 255);
    filledRect(2, y1, 10, y2, 255, 255, 255);
    glColor3ub(0, 0, 0);
    glBegin(GL_LINE_LOOP); glVertex2f(-10, y1); glVertex2f(-2, y1); glVertex2f(-2, y2); glVertex2f(-10, y2); glEnd();
    glBegin(GL_LINE_LOOP); glVertex2f(2, y1);  glVertex2f(10, y1); glVertex2f(10, y2); glVertex2f(2, y2);  glEnd();
}

static void drawLeg(float x1, float x2) {
    filledRect(x1, -40, x2, -110, 252, 226, 5);
}

static void drawSock(float xLeft, unsigned char r, unsigned char g, unsigned char b) {
    // Sock body
    filledRect(xLeft, -110, xLeft + 22, -125, 255, 255, 255);
    // Stripe
    filledRect(xLeft, -114, xLeft + 22, -118, r, g, b);
}

static void drawShoe(float rectLeft, float rectRight, float yTop, float yBottom, float capCx, float capCy) {
    filledRect(rectLeft, yTop, rectRight, yBottom, 0, 0, 0);
    filledCircle(capCx, capCy, 12, 0, 0, 0);
}

static void drawArm(float x1, float x2, float yTop, float yBottom) {
    filledRect(x1, yTop, x2, yBottom, 252, 226, 5);
}

static void drawSpongeBob() {
    // karena awalnya badan spongebob dibuat agak kecil, terus rupanya mulutnya kelebaran jd badannya di perpanjang dan yang diatas mulut di geser keatas + 25
    const float SHIFT_UP = 25.0f;

    // body
    float bodyLeft = -110.0f, bodyRight = 110.0f;
    float bodyBottom = -20.0f, bodyTop = 220.0f;
    filledRect(bodyLeft, bodyBottom, bodyRight, bodyTop, 252, 226, 5);

    // border
    glLineWidth(3.0f);
    glColor3ub(189, 160, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bodyLeft, bodyBottom);
    glVertex2f(bodyRight, bodyBottom);
    glVertex2f(bodyRight, bodyTop);
    glVertex2f(bodyLeft, bodyTop);
    glEnd();

    // pores / pori - pori
    drawPore(-80, 150 + SHIFT_UP, 8);
    drawPore(-60, 60, 12);
    drawPore(70, 130 + SHIFT_UP, 9);
    drawPore(50, 20, 7);
    drawPore(-10, 110 + SHIFT_UP, 6);
    drawPore(90, 70, 10);

    // baju
    filledRect(bodyLeft, 0, bodyRight, 20, 255, 255, 255);

    // collar
    triangle(-20, 20, -5, 20, -20, 0, 255, 255, 255);
    triangle(20, 20, 5, 20, 20, 0, 255, 255, 255);

    // pants
    filledRect(bodyLeft, -40, bodyRight, 0, 120, 72, 0);

    // belt
    line(bodyLeft, -10, bodyRight, -10, 4.0f, 0, 0, 0);
    drawBeltLoop(-70);
    drawBeltLoop(-20);
    drawBeltLoop(10);
    drawBeltLoop(60);

    // tie
    drawTieAtWhiteBand();

	// eyes dan eyelashes
    float eyeY = 145.0f;
    drawEye(-45, eyeY);
    drawEye(45, eyeY);
    drawEyelashes(-45, eyeY, -1);
    drawEyelashes(45, eyeY, +1);

    // nose outline
    circleOutline(0, 85 + SHIFT_UP, 12, 2.0f, 189, 160, 0);

    // cheeks
    filledCircle(-75, 90,12, 255, 200, 200);
    filledCircle(75, 90,12, 255, 200, 200);

    // mouth and teeth 
    drawMouthLowerArc(0.0f, 100.0f, 55.0f, 3.0f);
    drawTeeth(32.0f, 45.0f);

    // legs
    drawLeg(-55, -35);
    drawLeg(35, 55);

    // socks
    drawSock(-56, 255, 0, 0);  // kiri - merah
    drawSock(34, 0, 0, 255);   // kanat - biru

    // shoes
    drawShoe(-68, -22, -125, -138, -22, -138);
    drawShoe(22, 68, -125, -138, 22, -138);

    // arms
    drawArm(bodyRight, bodyRight + 45, 60, 45); // kanan
    drawArm(bodyLeft - 45, bodyLeft, 60, 45);   // kiri
}

static void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();
    glTranslatef(g_posX, g_posY, 0.0f);
    drawSpongeBob();
    glPopMatrix();

    glutSwapBuffers();
}

static void init2D() {
    glClearColor(1.f, 1.f, 1.f, 1.f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(-200, 200, -160, 240);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// input handlers untuk movement
static void onSpecialDown(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  g_moveLeft = true;
    if (key == GLUT_KEY_RIGHT) g_moveRight = true;
}

static void onSpecialUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  g_moveLeft = false;
    if (key == GLUT_KEY_RIGHT) g_moveRight = false;
}

static void onKeyboardDown(unsigned char key, int x, int y) {
    if (key == ' ') {
        if (!g_inAir) {
            g_inAir = true;
            g_velY = g_jumpVel;
        }
    }
}

static void onKeyboardUp(unsigned char key, int x, int y) {
}

static void update() {
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (g_lastTimeMs == 0) g_lastTimeMs = now;
    float dt = (now - g_lastTimeMs) / 1000.0f;
    g_lastTimeMs = now;

    float vx = 0.0f;
    if (g_moveLeft)  vx -= g_speed;
    if (g_moveRight) vx += g_speed;
    g_posX += vx * dt;

    if (g_inAir) {
        g_velY += g_gravity * dt;
        g_posY += g_velY * dt;
        if (g_posY <= 0.0f) {
            g_posY = 0.0f;
            g_velY = 0.0f;
            g_inAir = false;
        }
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(800, 700);
    glutCreateWindow("Spongebob - UTS Grafkom");

    init2D();

    glutDisplayFunc(display);
    glutIdleFunc(update);
    glutSpecialFunc(onSpecialDown);
    glutSpecialUpFunc(onSpecialUp);
    glutKeyboardFunc(onKeyboardDown);
    glutKeyboardUpFunc(onKeyboardUp);

    glutMainLoop();
    return 0;
}
