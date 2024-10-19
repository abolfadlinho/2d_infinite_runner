#define _USE_MATH_DEFINES
#include <cstdlib> 
#include <glut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <string>
#include <Windows.h>
#include <mmsystem.h>
#include <thread>
#include <cstdlib>

// Game variables
float playerX = -0.6f, playerY = -0.8f;
bool isJumping = false, isDucking = false;
int playerLives = 5, score = 0;
float gameSpeed = 0.008f, timeRemaining = 60.0f;
bool gameRunning = true;
int healthsRemaining = 2; //kangaroo powerup
int scoresRemaining = 2; //slowmotion powerup
std::string gameMessage = ""; //message that will be displayed at end of game
float healthTimer = 0.0f;
float scoreTimer = 0.0f;
float jumpDelta = 0.7f; //amount of Y movement for jump
float obstacleCollisionTimer = 0.0f; //for moving backwards on collision
float x1 = 1.8f, x2 = 2.6f, x3 = 1.1f, x4 = 2.3f; //x coordinates to spawn regenerated objects
int ctr = 0; //counter for object regeneration modulud
float minuteHandAngle = (3 * M_PI / 6);  // Angle of the minute hand (background annimation)
const float minuteHandSpeed = (M_PI / 1500);
float soundTimer = 0.0f;


//Threads
void playBackground() {
    //while (true) {
    PlaySound(TEXT("Sounds/background.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
    //std::this_thread::sleep_for(std::chrono::seconds(100));
//}
}

void playCrash() {
    //while (true) {
    PlaySound(TEXT("Sounds/crash.wav"), NULL, SND_ASYNC | SND_FILENAME);
    //   std::this_thread::sleep_for(std::chrono::seconds(1));
   //}
}

void playPoint() {
    //while (true) {
    PlaySound(TEXT("Sounds/point.wav"), NULL, SND_ASYNC | SND_FILENAME);
    //std::this_thread::sleep_for(std::chrono::seconds(1));
//}
}
//std::thread t1(playBackground);



// Obstacle and collectable classes
class GameObject {
public:
    float x, y, width, height;
    GameObject(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}
    void move(float speed) { x -= speed; }
    virtual void draw() = 0;
    bool isColliding(float px, float py, float pw, float ph) {
        return !(px + pw < x || px > x + width || py + ph < y || py > y + height);
    }
};

//children of game object since both need isColliding and coordinates
class Obstacle : public GameObject {
public:
    Obstacle(float x, float y, float w, float h) : GameObject(x, y, w, h) {}
    void draw() override {
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();

        // Draw the triangle
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y); // vertex 1 (bottom-left)
        glVertex2f(x + width, y);  // vertex 2 (bottom-right)
        glVertex2f(x + width / 2, y + height);     // vertex 3 (top-center)
        glEnd();
        //Marlboro
        glColor3f(0.0f, 0.0f, 0.0f); // Color of the text (white)
        std::string text = "Marlboro";
        glRasterPos2f(x + width / 2 - 0.05f, y + height / 2 - 0.025f); // Set position for the text
        for (char c : text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, c); // Draw each character
        }
    }
};

class Collectable : public GameObject {
private:
    float leafAngle;    // Angle for leaf rotation
    float angleSpeed;   // Speed of rotation
    float maxLeafAngle; // Maximum angle for the leaf
    bool tiltingRight;  // Direction of tilt

public:
    Collectable(float x, float y, float w, float h)
        : GameObject(x, y, w, h),
        leafAngle(0.0f),
        angleSpeed(2.0f), // Speed of oscillation (degrees per frame)
        maxLeafAngle(15.0f), // Maximum tilt angle
        tiltingRight(true) {} // Start tilting to the right

    void update() {
        // Update leaf angle based on direction
        if (tiltingRight) {
            leafAngle += angleSpeed;
            if (leafAngle >= maxLeafAngle) {
                tiltingRight = false; // Switch to tilting left
            }
        }
        else {
            leafAngle -= angleSpeed;
            if (leafAngle <= -maxLeafAngle) {
                tiltingRight = true; // Switch to tilting right
            }
        }
    }

    void draw() override {
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x+0.05f, y);  // Bottom-left
        glVertex2f(x + width-0.05f, y);   // Bottom-right
        glVertex2f(x + width, y + height - 0.1f); // Top-right
        glVertex2f(x, y + height - 0.1f); // Top-left
        glEnd();

        glColor3f(0.5f, 0.25f, 0.0f); // Brown color for the stem
        glBegin(GL_LINES);
        glVertex2f(x + width / 2, y + height - 0.1f); // Start from the top of the apple
        glVertex2f(x + width / 2, y + height); // Draw stem up
        glEnd();

        glColor3f(0.0f, 0.5f, 0.0f); // Green color for the leaf
        glPushMatrix(); // Save the current transformation state
        glTranslatef(x + width / 2, y + height - 0.05f, 0.0f); // Move to stem tip
        glRotatef(leafAngle, 0.0f, 0.0f, 1.0f); // Rotate around the Z-axis (leaf tilt)
        glBegin(GL_TRIANGLES);
        glVertex2f(0.0f, 0.0f); // Stem tip (origin)
        glVertex2f(0.1f, 0.0f); // Leaf left
        glVertex2f(0.1f, -0.03f); // Leaf right
        glEnd();
        glPopMatrix(); // Restore the previous transformation state
    }
};

//slowmotion
class Health : public GameObject {
public:
    Health(float x, float y) : GameObject(x, y, 0.1f, 0.1f) {
        // Set initial width and height
        originalWidth = 0.1f;
        originalHeight = 0.1f;
        currentWidth = originalWidth;
        currentHeight = originalHeight;
        scalingSpeed = 0.001f; // Speed of expansion/retraction
        expanding = true; // Start by expanding
    }

    void draw() override {
        // Draw the box with the current width and height
        glColor3f(1.0f, 1.0f, 1.0f); // White for the box
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + currentWidth, y);
        glVertex2f(x + currentWidth, y + currentHeight);
        glVertex2f(x, y + currentHeight);
        glEnd();

        // Set color for the cross
        glColor3f(1.0f, 0.0f, 0.0f); // Red for the cross

        // Draw the horizontal part of the cross
        drawCrossHorizontal(y);

        // Draw the vertical part of the cross
        drawCrossVertical();
    }

    void update() {
        // Update the current width and height for the animation
        if (expanding) {
            currentWidth += scalingSpeed;
            currentHeight += scalingSpeed;

            // Check if we've reached the maximum size
            if (currentWidth >= 0.2f || currentHeight >= 0.2f) {
                expanding = false; // Start retracting
            }
        }
        else {
            currentWidth -= scalingSpeed;
            currentHeight -= scalingSpeed;

            // Check if we've reached the minimum size
            if (currentWidth <= 0.1f || currentHeight <= 0.1f) {
                expanding = true; // Start expanding again
            }
        }
    }

private:
    float originalWidth, originalHeight; // Store original dimensions
    float currentWidth, currentHeight; // Current dimensions used for drawing
    float scalingSpeed; // Speed of scaling
    bool expanding; // Direction of animation

    void drawCrossHorizontal(float y) {
        // Draw the horizontal part of the cross
        glBegin(GL_QUADS);
        glVertex2f(x + 0.01, y + 0.01f);
        glVertex2f(x + 0.01, y + 0.02f);
        glVertex2f(x + currentWidth - 0.01, y + 0.05f);
        glVertex2f(x + currentWidth - 0.01, y + 0.01f);
        glEnd();

        glBegin(GL_QUADS);
        glVertex2f(x + 0.01, y + currentHeight - 0.01f);
        glVertex2f(x + 0.01, y + currentHeight - 0.02f);
        glVertex2f(x + currentWidth - 0.01, y + currentHeight - 0.05f);
        glVertex2f(x + currentWidth - 0.01, y + currentHeight - 0.01f);
        glEnd();
    }

    void drawCrossVertical() {
        // Draw the vertical part of the cross
        glBegin(GL_QUADS);
        glVertex2f(x + 0.01f, y + 0.02);
        glVertex2f(x + 0.01f, y + currentHeight - 0.02);
        glVertex2f(x + 0.04f, y + currentHeight - 0.02);
        glVertex2f(x + 0.04f, y + 0.02);
        glEnd();
    }
};

//jump
class Score : public GameObject {
private:
    float originalWidth;
    float originalHeight;
    float currentWidth;
    float currentHeight;
    float scalingSpeed; // Speed of the expansion/retraction
    bool expanding; // Flag to indicate if it's expanding or retracting

public:
    Score(float x, float y) : GameObject(x, y, 0.1f, 0.1f) {
        originalWidth = 0.1f;
        originalHeight = 0.1f;
        currentWidth = originalWidth;
        currentHeight = originalHeight;
        scalingSpeed = 0.001f; // Speed of expansion/retraction
        expanding = true; // Start by expanding
    }

    void update() {
        // Update the width and height based on the expanding flag
        if (expanding) {
            currentWidth += scalingSpeed;
            currentHeight += scalingSpeed;
            if (currentWidth >= 0.2f || currentHeight >= 0.2f) {
                expanding = false; // Change to retracting
            }
        }
        else {
            currentWidth -= scalingSpeed;
            currentHeight -= scalingSpeed;
            if (currentWidth <= 0.1f || currentHeight <= 0.1f) {
                expanding = true; // Change to expanding
            }
        }

        // Update the GameObject's width and height for drawing
        width = currentWidth;
        height = currentHeight;
    }

    void draw() override {
        glColor3f(0.5f, 0.0f, 0.5f); // Purple for the box
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
        glEnd();
        glColor3f(1.0f, 1.0f, 1.0f); // White for letter J
        glBegin(GL_POLYGON);
        glVertex2f(x + 0.6f * width, y + 0.0f * height);  // Bottom-left
        glVertex2f(x + 0.6f * width, y + 0.8f * height);  // Top-left
        glVertex2f(x + 0.8f * width, y + 0.8f * height);  // Top-right
        glVertex2f(x + 0.8f * width, y + 0.0f * height);  // Bottom-right
        glEnd();

        // Draw the curved bottom part of the letter J
        glBegin(GL_POLYGON);
        glVertex2f(x + 0.1f * width,0.01f+ y + 0.0f * height);  // Bottom-left of the curve
        glVertex2f(x + 0.2f * width, 0.01f + y + 0.1f * height);  // Curve control point
        glVertex2f(x + 0.4f * width, 0.01f + y + 0.0f * height);  // Bottom-right of the curve
        glVertex2f(x + 0.2f * width, 0.01f + y + 0.0f * height);  // Bottom-middle
        glEnd();

        // Draw the horizontal top bar of the letter J
        glBegin(GL_POLYGON);
        glVertex2f(x + 0.3f * width, y + 0.7f * height);  // Top-left
        glVertex2f(x + 0.5f * width, y + 0.8f * height);  // Top-middle
        glVertex2f(x + 0.7f * width, y + 0.8f * height);  // Top-right
        glVertex2f(x + 0.7f * width, y + 0.7f * height);  // Bottom-right
        glEnd();
    }
};


std::vector<Obstacle> obstacles;
std::vector<Collectable> collectables;
std::vector<Health> healths;
std::vector<Score> scores;

// Function prototypes
void display();
void timer(int);
void updateObjects();
void drawPlayer();
void drawHumanCharacter(float x, float y, float height);
void drawBoundaries();
void displayScoreAndHealth();
void handleKeyboard(unsigned char key, int x, int y);
void handleKeyUp(unsigned char key, int x, int y);
void checkCollisions();
void gameWon();
void gameOver();

static void init() {
    //PlaySound(TEXT("Sounds/background.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);

    glClearColor(0.5f, 0.8f, 0.9f, 1.0f);  // Set the background color
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    // initialize some objects
    obstacles.push_back(Obstacle(x1, -0.8f, 0.1f, 0.1f));
    obstacles.push_back(Obstacle(x2, -0.6f, 0.1f, 0.1f));
    collectables.push_back(Collectable(x3, -0.4f, 0.2f, 0.2f));
    collectables.push_back(Collectable(x4, -0.8f, 0.2f, 0.2f));
    healths.push_back(Health(5.0f, -0.8f));
    healths.push_back(Health(15.0f, -0.4f));
    scores.push_back(Score(10.0f, -0.8f));
    scores.push_back(Score(20.0f, -0.4f));

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBoundaries();
    drawPlayer();

    // draw obstacles and collectables
    for (auto& obstacle : obstacles) {
        obstacle.draw();
    }
    for (auto& collectable : collectables) {
        collectable.draw();
    }

    if (healthsRemaining > 0) {
        for (auto& health : healths) {
            health.draw();
        }
    }
    if (scoresRemaining) {
        for (auto& score : scores) {
            score.draw();
        }
    }

    displayScoreAndHealth();

    // game end
    if (!gameMessage.empty()) {
        if (gameMessage.find("Game End!") != std::string::npos) {
            PlaySound(TEXT("Sounds/win.wav"), NULL, SND_ASYNC | SND_FILENAME);
            glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glColor3f(0.0f, 1.0f, 0.0f); // Set text color to green for "Game Won!"
        }
        else {
            PlaySound(TEXT("Sounds/lose.wav"), NULL, SND_ASYNC | SND_FILENAME);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glColor3f(1.0f, 0.0f, 0.0f); // Set text color to red for other messages
        }

        glRasterPos2f(-0.2f, 0.0f); // Position the text in the center

        for (const char& c : gameMessage) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Draw each character
        }
    }

    glutSwapBuffers();
}

static void drawCircle(float x, float y, float radius) {
    const int numSegments = 100; // Number of segments to approximate the circle
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y); // Center of the circle
    for (int i = 0; i <= numSegments; ++i) {
        float angle = 2.0f * M_PI * i / numSegments; // Calculate angle for each segment
        float dx = radius * cos(angle); // X component
        float dy = radius * sin(angle); // Y component
        glVertex2f(x + dx, y + dy); // Vertex for this segment
    }
    glEnd();
}

void drawHand(float length, float angle, float width) {
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.0f);  // Center of the clock
    glVertex2f(length * cos(angle), length * sin(angle));  // End of the hand
    glEnd();
}

void drawBoundaries() {

    glColor3f(1.0f, 1.0f, 1.0f);  // Black color for clock outline
    drawCircle(0, 0, 0.3f);  // Clock face

    // Draw hour marks
    glColor3f(0.0f, 0.0f, 0.0f);  // Black color for hour marks
    for (int i = 0; i < 12; i++) {
        float angle = i * (M_PI / 6);  // 30 degrees in radians
        float x1 = 0.24f * cos(angle);
        float y1 = 0.24f * sin(angle);
        float x2 = 0.3f * cos(angle);
        float y2 = 0.3f * sin(angle);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }

    // Draw hour hand (e.g., at 3 o'clock)
    glColor3f(1.0f, 0.0f, 0.0f);  // Red color for hour hand
    drawHand(0.18f, (3 * M_PI / 6), 5.0f);  // Hour hand at 3 o'clock

    // Draw minute hand (e.g., at 12 o'clock)
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue color for minute hand
    drawHand(0.3f, minuteHandAngle, 3.0f);

    //bottom boundary
    glColor3f(0.3f, 0.3f, 0.3f);  // Set the color of the boundary quad (gray)
    glBegin(GL_QUADS);
    // Define the bottom boundary quad coordinates
    glVertex2f(-1.0f, -0.9f);   // Bottom-left corner
    glVertex2f(1.0f, -0.9f);    // Bottom-right corner
    glVertex2f(1.0f, -0.8f);    // Top-right corner (slightly above the bottom)
    glVertex2f(-1.0f, -0.8f);   // Top-left corner
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f); // Set triangle color to blue
    int numberOfTriangles = 10;
    float triangleWidth = 2.0f / numberOfTriangles; // Calculate width of each triangle
    for (int i = 0; i < numberOfTriangles; ++i) {
        float xOffset = -1.0f + i * triangleWidth; // Calculate x position for each triangle

        glBegin(GL_TRIANGLES);
        glVertex2f(xOffset, -1.0f);                // Bottom vertex
        glVertex2f(xOffset + triangleWidth, -1.0f); // Bottom-right vertex
        glVertex2f(xOffset + triangleWidth / 2.0f, -0.9f); // Top vertex
        glEnd();
    }

    //Top boundary
    glColor3f(1.0f, 1.0f, 1.0f); // Set circle color to green
    int numberOfCircles = 4;
    float circleWidth = 2.0f / numberOfCircles; // Calculate width of each circle
    float radius = circleWidth / 2.0f; // Set radius to half of the circle width

    for (int i = 0; i < numberOfCircles; ++i) {
        float xOffset = -1.0f + (i + 0.5f) * circleWidth; // Calculate x position for each circle

        drawCircle(xOffset, 0.75f, radius); // Draw the circle at the calculated position
    }
}

void drawPlayer() {
    drawHumanCharacter(playerX, playerY, 0.2f);
}

void drawHumanCharacter(float x, float y, float height) {
    float headRadius = height * 0.1f; // Radius for the head
    float bodyHeight = height * 0.5f;   // Height for the body
    float armLength = height * 0.3f;   // Length for the arms
    float legHeight = height * 0.4f;    // Height for the legs
    if (isDucking) {
        bodyHeight = height * 0.1f;
        legHeight = height * 0.3f;
    }

    glColor3f(1.0f, 0.8f, 0.6f);  // Light skin tone
    glBegin(GL_POLYGON);
    float radius = 0.025f;
    for (int i = 0; i < 5; ++i) {
        float angle = 2.0f * 3.14159f * float(i) / 5; // Calculate angle for each vertex
        float vertexX = x + radius * cos(angle);  // X coordinate of the vertex
        float vertexY = y + radius * sin(angle);  // Y coordinate of the vertex
        glVertex2f(vertexX, vertexY + 0.2f);              // Specify the vertex
    }
    glEnd();

    glColor3f(1.0f, 0.0f, 0.0f);
    // Draw body (rectangle as quad)
    glBegin(GL_QUADS);
    glVertex2f(x - 0.03f, y + height - headRadius - bodyHeight);
    glVertex2f(x + 0.03f, y + height - headRadius - bodyHeight);
    glVertex2f(x + 0.03f, y + height - headRadius);
    glVertex2f(x - 0.03f, y + height - headRadius);
    glEnd();

    glColor3f(1.0f, 0.8f, 0.6f);  // Light skin tone
    // Draw arms (lines)
    glBegin(GL_LINES);
    // Left arm
    glVertex2f(x - 0.03f, y + height - headRadius - bodyHeight * 0.5f);
    glVertex2f(x - 0.03f - armLength, y + height - headRadius - bodyHeight * 0.5f);

    // Right arm
    glVertex2f(x + 0.03f, y + height - headRadius - bodyHeight * 0.5f);
    glVertex2f(x + 0.03f + armLength, y + height - headRadius - bodyHeight * 0.5f);
    glEnd();

    glColor3f(0.0f, 0.0f, 0.5f);
    // Draw legs (triangles)
    glBegin(GL_TRIANGLES);
    // Left leg
    glVertex2f(x - 0.02f, y + height - headRadius - bodyHeight);
    glVertex2f(x - 0.04f, y + height - headRadius - bodyHeight - legHeight);
    glVertex2f(x, y + height - headRadius - bodyHeight - legHeight);

    // Right leg
    glVertex2f(x + 0.02f, y + height - headRadius - bodyHeight);
    glVertex2f(x + 0.04f, y + height - headRadius - bodyHeight - legHeight);
    glVertex2f(x, y + height - headRadius - bodyHeight - legHeight);
    glEnd();
}

void displayScoreAndHealth() {
    // Health bar
    for (int i = 0; i < playerLives; i++) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(-0.95f + i * 0.075f, 0.75f);
        glVertex2f(-0.85f + i * 0.075f, 0.75f);
        glVertex2f(-0.85f + i * 0.075f, 0.7f);
        glVertex2f(-0.95f + i * 0.075f, 0.7f);
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f); // Color of the border (black)
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.95f + i * 0.075f, 0.75f);          // Bottom-left
        glVertex2f(-0.85f + i * 0.075f, 0.75f);          // Bottom-right
        glVertex2f(-0.85f + i * 0.075f, 0.7f);         // Top-right
        glVertex2f(-0.95f + i * 0.075f, 0.7f);         // Top-left
        glEnd();
    }


    // Score and time display
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-0.85f, 0.65f);
    std::string scoreStr = "Score: " + std::to_string(score);
    for (char c : scoreStr) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    glRasterPos2f(-0.45f, 0.75f);
    std::string pu1Strb = "Slowmotion timer: " + std::to_string(int(healthTimer * 10));
    for (char c : pu1Strb) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
    glRasterPos2f(-0.39f, 0.65f);
    std::string pu1Str = "Slowmotion: " + std::to_string(healthsRemaining);
    for (char c : pu1Str) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }


    glRasterPos2f(0.06f, 0.75f);
    std::string pu2Strb = "Kangaroo timer: " + std::to_string(int(scoreTimer * 10));
    for (char c : pu2Strb) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
    glRasterPos2f(0.12f, 0.65f);
    std::string pu2Str = "Kangaroos: " + std::to_string(scoresRemaining);
    for (char c : pu2Str) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }

    glRasterPos2f(0.66f, 0.7f);
    std::string timeStr = "Time: " + std::to_string(int(timeRemaining));
    for (char c : timeStr) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
}

static bool isOverlapping(float x1, float y1, float width1, float height1,
    float x2, float y2, float width2, float height2) {
    return (x1 < x2 + width2 && x1 + width1 > x2 && y1 < y2 + height2 && y1 + height1 > y2);
}

static bool canPlaceObject(float x, float y, float width, float height, const void* currentObject = nullptr) {
    for (const auto& obstacle : obstacles) {
        if (&obstacle != currentObject && isOverlapping(x, y, width, height, obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
            return false; // Overlap with obstacle
        }
    }
    for (const auto& collectable : collectables) {
        if (&collectable != currentObject && isOverlapping(x, y, width, height, collectable.x, collectable.y, collectable.width, collectable.height)) {
            return false; // Overlap with collectable
        }
    }
    for (const auto& health : healths) {
        if (&health != currentObject && isOverlapping(x, y, width, height, health.x, health.y, health.width, health.height)) {
            return false; // Overlap with health
        }
    }
    for (const auto& score : scores) {
        if (&score != currentObject && isOverlapping(x, y, width, height, score.x, score.y, score.width, score.height)) {
            return false; // Overlap with score
        }
    }

    return true; // No overlap, safe to place
}

void timer(int) {
    if (gameRunning) {

        if (isJumping) {
            if (playerY < -0.8f + jumpDelta) {
                playerY += 0.025f;
            }
            else {
                isJumping = false;
            }
        }
        else {
            if (playerY > -0.8f) {
                playerY -= 0.025f;
            }
        }

        //animations
        minuteHandAngle -= minuteHandSpeed;  // Update the angle
        if (minuteHandAngle >= 2 * M_PI) {
            minuteHandAngle -= 2 * M_PI;  // Wrap around the angle
        }

        // Update game time
        timeRemaining -= 0.02f;

        if (soundTimer <= 0.02f && soundTimer > 0.0f) {
            //PlaySound(TEXT("Sounds/background.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
        }

        if (healthTimer <= 0.02f && healthTimer > 0.0f) {
            //cancel power
            //playerX = -0.6f;
            gameSpeed = 0.008f;
        }
        if (scoreTimer <= 0.02f && scoreTimer > 0.0f) {
            jumpDelta = 0.7f;
            if (playerY > -0.8f) {
                playerY = -0.8f + jumpDelta;
            }
        }
        if (obstacleCollisionTimer <= 0.02f && obstacleCollisionTimer > 0.0f) {
            if (canPlaceObject(-0.6f, playerY, 0.2f, 0.2f, NULL)) {
                playerX = -0.6f;
            }
            else {
                obstacleCollisionTimer = 0.5f;
            }
        }

        if (soundTimer > 0.0f) {
            soundTimer -= 0.02f;
        }

        if (healthTimer > 0.0f) {
            healthTimer -= 0.02f;
        }

        if (scoreTimer > 0.0f) {
            scoreTimer -= 0.02f;
        }

        if (obstacleCollisionTimer > 0.0f) {
            obstacleCollisionTimer -= 0.02f;
        }

        updateObjects();
        checkCollisions();

        if (playerLives <= 0) {
            gameRunning = false;
            gameOver();
        }
        else {
            if (timeRemaining <= 0) {
                gameRunning = false;
                gameWon();
            }
        }

        glutPostRedisplay();
        glutTimerFunc(20, timer, 0);
    }
}

void moveEverythingBack(float deltax) {
    for (auto& obstacle : obstacles) {
        obstacle.x += deltax;
    }
    for (auto& collectable : collectables) {
        collectable.x += deltax;
    }
    for (auto& health : healths) {
        health.x += deltax;
    }
    for (auto& score : scores) {
        score.x += deltax;
    }
}


void checkCollisions() {
    for (auto& obstacle : obstacles) {
        if (obstacle.isColliding(playerX - 0.2f, playerY, 0.2f, 0.2f) && (playerX < obstacle.x + 0.2f)) {
            std::thread t2(playCrash);
            t2.detach();
            //PlaySound(TEXT("Sounds/crash.wav"), NULL, SND_ASYNC | SND_FILENAME);
            soundTimer = 0.5f;
            playerLives--;
            /*playerX = -0.95f;
            playerY = -0.8f;
            isJumping = false;
            obstacleCollisionTimer = 1.0f;
            gameSpeed = 0.005f;*/
            moveEverythingBack(0.5f);

            /*ctr++;
            obstacle.x = -5.0f;
            switch (ctr % 4) {
            case 0:
                if (canPlaceObject(x1, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x1;
                }
                break;
            case 1:
                if (canPlaceObject(x2, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x2;
                }
                break;
            case 2:
                if (canPlaceObject(x3, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x3;
                }
                break;
            case 3:
                if (canPlaceObject(x4, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x4;
                }
                break;
            default:
                ctr--;
                break;
            }*/
        }
    }

    for (auto& collectable : collectables) {
        if (collectable.isColliding(playerX - 0.2f, playerY, 0.2f, 0.2f)) {
            std::thread t3(playPoint);
            t3.detach();
            //PlaySound(TEXT("Sounds/point.wav"), NULL, SND_ASYNC | SND_FILENAME);
            soundTimer = 0.4f;
            score++;
            ctr++;
            collectable.x = -5.0f;
            switch (ctr % 4) {
            case 0:
                if (canPlaceObject(x1, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x1;
                }
                break;
            case 1:
                if (canPlaceObject(x2, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x2;
                }
                break;
            case 2:
                if (canPlaceObject(x3, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x3;
                }
                break;
            case 3:
                if (canPlaceObject(x4, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x4;
                }
                break;
            default:
                ctr--;
                break;
            }
        }
    }

    for (auto& health : healths) {
        if (health.isColliding(playerX - 0.2f, playerY, 0.2f, 0.2f)) {
            if (healthsRemaining <= 0) {
                for (auto& innerHealth : healths) {
                    innerHealth.x = -5.0f;
                    healths.pop_back();
                }
                healths.clear();
            }
            else {
                healthsRemaining--;
                //playerX = -0.9f; //powerup
                gameSpeed = 0.004f;
                healthTimer = 5.0f;
            }
            health.x = -5.0f;
        }
    }

    for (auto& scoreObj : scores) {
        if (scoreObj.isColliding(playerX - 0.2f, playerY, 0.2f, 0.2f)) {
            if (scoresRemaining <= 0) {
                for (auto& innerScore : scores) {
                    innerScore.x = -5.0f;
                    scores.pop_back();
                }
                scores.clear();
            }
            else {
                scoresRemaining--;
                jumpDelta = 1.0f; //power up
                scoreTimer = 5.0f;
            }
            scoreObj.x = -5.0f;
        }
    }
}

void updateObjects() {
    for (auto& obstacle : obstacles) {
        obstacle.move(gameSpeed);
        //if (obstacle.x < -1.0f) {
        if (obstacle.x < -1.0f) {
            ctr++;
            switch (ctr % 4) {
            case 0:
                if (canPlaceObject(x1, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x1;
                }
                break;
            case 1:
                if (canPlaceObject(x2, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x2;
                }
                break;
            case 2:
                if (canPlaceObject(x3, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x3;
                }
                break;
            case 3:
                if (canPlaceObject(x4, obstacle.y, obstacle.width, obstacle.height, &obstacle)) {
                    obstacle.x = x4;
                }
                break;
            default:
                ctr--;
                break;
            }
        }
    }

    for (auto& collectable : collectables) {
        collectable.update();
        collectable.move(gameSpeed);
        if (collectable.x < -1.0f) {
            ctr++;
            switch (ctr % 4) {
            case 0:
                if (canPlaceObject(x1, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x1;
                }
                break;
            case 1:
                if (canPlaceObject(x2, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x2;
                }
                break;
            case 2:
                if (canPlaceObject(x3, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x3;
                }
                break;
            case 3:
                if (canPlaceObject(x4, collectable.y, collectable.width, collectable.height, &collectable)) {
                    collectable.x = x4;
                }
                break;
            default:
                ctr--;
                break;
            }
        }
    }

    for (auto& health : healths) {
        health.update();
        health.move(gameSpeed);
        if (health.x < -1.0f) {
            if (canPlaceObject(1.0f, health.y, health.width, health.height, &health)) {
                health.x = 20.0f;
            }
        }
    }

    for (auto& score : scores) {
        score.update();
        score.move(gameSpeed);
        if (score.x < -1.0f) {
            if (canPlaceObject(1.0f, score.y, score.width, score.height, &score)) {
                score.x = 20.0f;
            }
        }
    }
    // Increase game speed over time
    //gameSpeed += 0.00001f; //TODO
}

void handleKeyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':  // Jump
        if (!isJumping && (playerY == -0.8f || playerY <= -0.75f) && !isDucking) {
            isJumping = true;
            //playerY += jumpDelta;
        }
        break;
    case 's':  // Duck
        if (!isDucking && (playerY == -0.8f || playerY <= -0.75f) && !isJumping) {
            isDucking = true;
            playerY = -0.9f;
        }
        break;
    }
}

void handleKeyUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':  // End jump
        //isJumping = false;
        //playerY = -0.8f;
        break;
    case 's':  // End duck
        if (!isJumping && isDucking) {
            isDucking = false;
            playerY = -0.8f;
        }
        break;
    }
}

void gameOver() {
    gameMessage = "Game Lose! Score: " + std::to_string(score); // Convert score to string
}

void gameWon() {
    gameMessage = "Game End! Score: " + std::to_string(score); // Convert score to string
}

void runPythonScript() {
    system("\"C:/Program Files/Spyder/Python/python.exe\" script.py");
}

int main(int argc, char** argv) {
    //t1.detach();
    srand(time(0));

    std::thread pythonThread(runPythonScript);
    pythonThread.detach();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("2D Infinite Runner");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeyboard);
    glutKeyboardUpFunc(handleKeyUp);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}