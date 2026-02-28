#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>
#include <queue>

#include "core/engine.h"
#include "utils/gl_utils.h"
#include "transform2D.h"

using namespace std;
using namespace m1;

constexpr auto GRID_WIDTH = 10;
constexpr auto GRID_HEIGHT = 5;

constexpr auto GRID_CORNER_X = 0;
constexpr auto GRID_CORNER_Y = 55;

constexpr auto ANIMATION_BREAK = 1;
constexpr auto ANIMATION_PARTICLES = 2;
constexpr auto ANIMATION_SNOW = 3;

constexpr auto PARTICLE_WHITE = 0;
constexpr auto PARTICLE_BLUE = 1;
constexpr auto PARTICLE_RED = 2;
constexpr auto PARTICLE_GRAY = 3;
constexpr auto PARTICLE_GREEN = 4;
constexpr auto PARTICLE_PURPLE = 5;

Tema1_player::Tema1_player()
{
}


Tema1_player::~Tema1_player()
{
}

void Tema1_player::Init() {
    // Lock camera in orthogonal 2D projection, and set the logical coordinate space

    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 10));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    // I have a 16:10 monitor, so it feels most natural to use these
    // logical coordinates when setting up object positions
    logicSpace.x = 0;
    logicSpace.y = 0;
    logicSpace.height = 100;
    logicSpace.width = 160;

    shipOffsetX = 80;

    grid = vector<vector<unsigned int>>(GRID_HEIGHT, vector<unsigned int>(GRID_WIDTH, 0));

    ballThrown = false;
    ballY = 18;
    ballX = shipOffsetX;
    ballSpeedX = 0;
    ballSpeedY = 0;

    lives = 3;
    score = 0;
    gameFinished = false;
    gameLost = false;
    gameWon = false;

    ssDuration = 0.0f;

    auto rect_blue = create_endpoints_square(glm::vec3(0, 0.3f, 0.7f), glm::vec3(0, 0.3f, 0.7f), 1.98, 0.99);
    CreateMesh("rect_blue", rect_blue.first, rect_blue.second);

    auto rect_red = create_endpoints_square(glm::vec3(0.7f, 0, 0), glm::vec3(0.7f, 0, 0), 1.98, 0.99);
    CreateMesh("rect_red", rect_red.first, rect_red.second);

    auto rect_gray = create_endpoints_square(glm::vec3(0.604, 0.671, 0.69), glm::vec3(0.604, 0.671, 0.69), 1.98, 0.99);
    CreateMesh("rect_gray", rect_gray.first, rect_gray.second);

    auto rect_green = create_endpoints_square(glm::vec3(0, 0.8, 0), glm::vec3(0, 0.8, 0), 1.98, 0.99);
    CreateMesh("rect_green", rect_green.first, rect_green.second);

    auto rect_purple = create_endpoints_square(glm::vec3(0.29, 0.039, 0.631), glm::vec3(0.29, 0.039, 0.631), 1.98, 0.99);
    CreateMesh("rect_purple", rect_purple.first, rect_purple.second);

    auto particle_white = create_endpoints_square(glm::vec3(0.675, 0.71, 0.78), glm::vec3(0.675, 0.71, 0.78), 1, 1);
    CreateMesh("particle_white", particle_white.first, particle_white.second);

    auto particle_blue = create_endpoints_square(glm::vec3(0, 0.3f, 0.7f), glm::vec3(0, 0.3f, 0.7f), 1, 1);
    CreateMesh("particle_blue", particle_blue.first, particle_blue.second);

    auto particle_red = create_endpoints_square(glm::vec3(0.7f, 0, 0), glm::vec3(0.7f, 0, 0), 1, 1);
    CreateMesh("particle_red", particle_red.first, particle_red.second);

    auto particle_gray = create_endpoints_square(glm::vec3(0.604, 0.671, 0.69), glm::vec3(0.604, 0.671, 0.69), 1, 1);
    CreateMesh("particle_gray", particle_gray.first, particle_gray.second);

    auto particle_green = create_endpoints_square(glm::vec3(0, 0.8, 0), glm::vec3(0, 0.8, 0), 1, 1);
    CreateMesh("particle_green", particle_green.first, particle_green.second);

    auto particle_purple = create_endpoints_square(glm::vec3(0.29, 0.039, 0.631), glm::vec3(0.29, 0.039, 0.631), 1, 1);
    CreateMesh("particle_purple", particle_purple.first, particle_purple.second);

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            grid[i][j] = i + 1;
        }
    }
    for (auto pair : meshes) {
        cout << pair.first << endl;
    }

    textRenderer = new gfxc::TextRenderer(window->props.selfDir, logicSpace.width, logicSpace.height);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "BitcountGridSingle_Cursive-Bold.ttf"), 18);
}

void Tema1_player::FrameStart() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
    SetViewportArea(viewSpace, glm::vec3(0), true);
}

void Tema1_player::Update(float deltaTimeSeconds) {
    if (ssDuration > 0.001) {
        logicSpace.x = glm::exp(-3 * (0.5 - ssDuration) * glm::sin(60 * (0.5 - ssDuration) + 0.12));
        logicSpace.y = -0.3 * glm::exp(-3 * (0.5 - ssDuration) * glm::sin(60 * (0.5 - ssDuration) + 0.12));
    }

    visMatrix = glm::mat3(1);
    visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);

    if (!(gameWon || gameLost)) {
        gameFinished = true;
        gameWon = true;
        for (int i = 0; i < GRID_HEIGHT; i++) {
            for (int j = 0; j < GRID_WIDTH; j++) {
                if (grid[i][j]) {
                    gameFinished = false;
                    gameWon = false;
                }
            }
        }
    }
    

    if (gameFinished && gameLost) {
        textRenderer->RenderText("You Lost", 45, 45, 0.8, glm::vec3(1, 1, 1));
        return;
    }
    else if (gameFinished && gameWon) {
        textRenderer->RenderText("You Won", 45, 45, 0.8, glm::vec3(1, 1, 1));
        return;
    }

    char buf[5];
    string scoreText = "Score: " + string(itoa(score, buf, 10));
    string livesText = "Lives: " + string(itoa(lives, buf, 10));

    textRenderer->RenderText(scoreText, 0, 2, 0.2);
    textRenderer->RenderText(livesText, 140, 2, 0.2);

    // Render all animations
    for (int i = 0; i < animations.size(); i++) {
        if (animations[i].animationType == ANIMATION_BREAK) {
            glm::mat3 breakAnim = glm::mat3(1);
            breakAnim = visMatrix * transform2D::Translate(animations[i].x + 8.0f * (1 - animations[i].remainingDuration),
                animations[i].y + 4.0f * (1 - animations[i].remainingDuration));
            breakAnim *= transform2D::Scale(8 * (animations[i].remainingDuration), 8 * (animations[i].remainingDuration));

            RenderMesh2D(meshes["rect_blue"], shaders["VertexColor"], breakAnim);
        }
        else if (animations[i].animationType == ANIMATION_PARTICLES) {
            int numParticles = 7;
            constexpr float spreadAngle = glm::radians(30.0f);
            float baseSpeed = 20.0f * animations[i].speed;

            glm::vec2 dir = glm::normalize(glm::vec2(animations[i].directionX, animations[i].directionY));
            float baseAngle = atan2f(dir.y, dir.x);

            for (int j = 0; j < numParticles; j++) {
                float t = (float)j / (numParticles - 1);
                float angle = baseAngle - spreadAngle / 2 + t * spreadAngle;

                // Direction vector for this particle
                float particleSpeedX = cos(angle) * baseSpeed;
                float particleSpeedY = sin(angle) * baseSpeed;

                // Slightly vary speed by index with a sin
                particleSpeedX *= (1 + glm::sin(AI_MATH_PI * t));
                particleSpeedY *= (1 + glm::sin(AI_MATH_PI * t));

                glm::mat3 particlePos = glm::mat3(1);
                particlePos = visMatrix * transform2D::Translate(animations[i].x + particleSpeedX * (1.0f - animations[i].remainingDuration),
                    animations[i].y + particleSpeedY * (1.0f - animations[i].remainingDuration)) *
                    transform2D::Scale(1, 1);

                switch (animations[i].particleColor) {
                    case PARTICLE_WHITE: {
                        RenderMesh2D(meshes["particle_white"], shaders["VertexColor"], particlePos);
                        break;
                    }
                    case PARTICLE_BLUE: {
                        RenderMesh2D(meshes["particle_blue"], shaders["VertexColor"], particlePos);
                        break;
                    }
                    case PARTICLE_RED: {
                        RenderMesh2D(meshes["particle_blue"], shaders["VertexColor"], particlePos);
                        break;
                    }
                    case PARTICLE_GRAY: {
                        RenderMesh2D(meshes["particle_red"], shaders["VertexColor"], particlePos);
                        break;
                    }
                    case PARTICLE_GREEN: {
                        RenderMesh2D(meshes["particle_gray"], shaders["VertexColor"], particlePos);
                        break;
                    }
                    case PARTICLE_PURPLE: {
                        RenderMesh2D(meshes["particle_green"], shaders["VertexColor"], particlePos);
                        break;
                    }

                }
            }
        }
        else if (animations[i].animationType == ANIMATION_SNOW) {
            glm::mat3 particlePos = glm::mat3(1);
            particlePos = visMatrix * transform2D::Translate(animations[i].x,
                animations[i].y - animations[i].speed * (2.0f - animations[i].remainingDuration)) *
                transform2D::Scale(0.4, 0.4);

            RenderMesh2D(meshes["particle_white"], shaders["VertexColor"], particlePos);
        }
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    // Render the ship
    glm::mat3 shipPos = glm::mat3(1);
    shipPos = visMatrix * transform2D::Translate(shipOffsetX, 10) * transform2D::Scale(3, 3);

    RenderMesh2D(meshes["ship"], shaders["VertexColor"], shipPos);

    // Render the grid
    glm::mat3 baseGrid = glm::mat3(1);
    baseGrid = transform2D::Translate(GRID_CORNER_X, GRID_CORNER_Y);

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            glm::mat3 gridRect = visMatrix * transform2D::Translate(j * 16 + 0.1, i * 8 + 0.05) * baseGrid * transform2D::Scale(8, 8);
            switch (grid[i][j])
            {
                case 1: {
                    RenderMesh2D(meshes["rect_blue"], shaders["VertexColor"], gridRect);
                    break;
                }
                case 2: {
                    RenderMesh2D(meshes["rect_red"], shaders["VertexColor"], gridRect);
                    break;
                }
                case 3: {
                    RenderMesh2D(meshes["rect_gray"], shaders["VertexColor"], gridRect);
                    break;
                }
                case 4: {
                    RenderMesh2D(meshes["rect_green"], shaders["VertexColor"], gridRect);
                    break;
                }
                case 5: {
                    RenderMesh2D(meshes["rect_purple"], shaders["VertexColor"], gridRect);
                    break;
                }
            }
        }
    }
    
    // Update ball position
    if (ballThrown) {
        if (ballX + 1 + deltaTimeSeconds * ballSpeedX >= 160 && ballSpeedX > 0) {
            ballSpeedX *= -1;
        }
        else if (ballX - 1 + deltaTimeSeconds * ballSpeedX <= 0 && ballSpeedX < 0) {
            ballSpeedX *= -1;
        }
        else {
            ballX += deltaTimeSeconds * ballSpeedX;
        }
        
        if (ballY + 1 + deltaTimeSeconds * ballSpeedY >= 97 && ballSpeedY > 0) {
            ballSpeedY *= -1;
        }
        else if (ballY - 1 + deltaTimeSeconds * ballSpeedY <= 3 && ballSpeedY < 0) {
            lives--;

            if (lives <= 0) {
                gameFinished = true;
                gameLost = true;
            }

            ballThrown = false;
            shipOffsetX = 80;
            ballX = 80;
            ballY = 18;
            ballSpeedX = 0;
            ballSpeedY = 0;
        }
        else {
            ballY += deltaTimeSeconds * ballSpeedY;
        }

        // Check collision with blocks in the grid
        for (int i = 0; i < GRID_HEIGHT; i++) {
            for (int j = 0; j < GRID_WIDTH; j++) {
                if (!grid[i][j]) continue; // Skip destroyed blocks

                float minBlockX = GRID_CORNER_X + j * 16;
                float maxBlockX = GRID_CORNER_X + (j + 1) * 16;
                float minBlockY = GRID_CORNER_Y + i * 8;
                float maxBlockY = GRID_CORNER_Y + (i + 1) * 8;

                // Next ball position
                float nextX = ballX + deltaTimeSeconds * ballSpeedX;
                float nextY = ballY + deltaTimeSeconds * ballSpeedY;

                float leftXClip = (nextX + 1) - minBlockX;
                float rightXClip = maxBlockX - (nextX - 1);
                float upYClip = maxBlockY - (nextY - 1);
                float downYClip = (nextY + 1) - minBlockY;

                bool collided = false;

                // Ball hits from the left
                if (nextX + 1 > minBlockX &&
                    nextX + 1 < maxBlockX &&
                    (nextY + 1 > minBlockY && nextY + 1 < maxBlockY || nextY - 1 > minBlockY && nextY - 1 < maxBlockY) &&
                    ballSpeedX > 0) {
                    if (upYClip > 0 && upYClip < leftXClip || downYClip > 0 && downYClip < leftXClip) {

                    }
                    else {
                        ballSpeedX = -ballSpeedX;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, grid[i][j], partSpeed));
                    }
                    
                }

                // Ball hits from the right
                if (maxBlockX > (nextX - 1) &&
                    minBlockX < (nextX - 1) &&
                    (nextY + 1 > minBlockY && nextY + 1 < maxBlockY || nextY - 1 > minBlockY && nextY - 1 < maxBlockY) &&
                    ballSpeedX < 0) {
                    if (upYClip > 0 && upYClip < rightXClip || downYClip > 0 && downYClip < rightXClip) {

                    }
                    else {
                        ballSpeedX = -ballSpeedX;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, grid[i][j], partSpeed));
                    }
                }

                // Ball hits from below
                if (nextY + 1 > minBlockY &&
                    nextY + 1 < maxBlockY &&
                    (nextX + 1 > minBlockX && nextX + 1 < maxBlockX || nextX - 1 > minBlockX && nextX - 1 < maxBlockX) &&
                    ballSpeedY > 0) {
                    if (leftXClip > 0 && leftXClip < downYClip || rightXClip > 0 && rightXClip < downYClip) {

                    }
                    else {
                        ballSpeedY = -ballSpeedY;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, grid[i][j], partSpeed));
                    }
                }

                // Ball hits from above
                if (maxBlockY > (nextY - 1) &&
                    minBlockY < (nextY - 1) &&
                    (nextX + 1 > minBlockX && nextX + 1 < maxBlockX || nextX - 1 > minBlockX && nextX - 1 < maxBlockX) &&
                    ballSpeedY < 0) {
                    if (leftXClip > 0 && leftXClip < upYClip || rightXClip > 0 && rightXClip < upYClip) {

                    }
                    else {
                        ballSpeedY = -ballSpeedY;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, grid[i][j], partSpeed));
                    }
                }

                // If we hit the block, lower it's hp
                if (collided) {
                    grid[i][j]--;
                    if (grid[i][j] == 0) {
                        // If block was destroyed, increase score and start the destroyed block animation
                        animations.push_back(Animation(minBlockX, minBlockY, 16.0f, 8.0f, 1.0f, ANIMATION_BREAK, PARTICLE_WHITE, 0));
                        score++;

                        if (ssDuration < 0.001) {
                            ssDuration = 0.25;
                        }
                    }
                }
            }
        }

        // Check collision with the ship paddle
        for (int i = 0; i < shipColliders.size(); i++) {
            float minBlockX = shipColliders[i].cornerX * 3 + shipOffsetX;
            float maxBlockX = (shipColliders[i].cornerX + shipColliders[i].width) * 3 + shipOffsetX;
            float minBlockY = shipColliders[i].cornerY * 3 + 10;
            float maxBlockY = (shipColliders[i].cornerY + shipColliders[i].height) * 3 + 10;

            if (!shipColliders[i].isBumperHead) {
                // Next ball position
                float nextX = ballX + deltaTimeSeconds * ballSpeedX;
                float nextY = ballY + deltaTimeSeconds * ballSpeedY;

                float leftXClip = (nextX + 1) - minBlockX;
                float rightXClip = maxBlockX - (nextX - 1);
                float upYClip = maxBlockY - (nextY - 1);
                float downYClip = (nextY + 1) - minBlockY;

                bool collided = false;

                // Ball hits from the left
                if (nextX + 1 > minBlockX &&
                    nextX + 1 < maxBlockX &&
                    (nextY + 1 > minBlockY && nextY + 1 < maxBlockY || nextY - 1 > minBlockY && nextY - 1 < maxBlockY) &&
                    ballSpeedX > 0) {
                    if (upYClip > 0 && upYClip < leftXClip || downYClip > 0 && downYClip < leftXClip) {

                    }
                    else {
                        ballSpeedX = -ballSpeedX;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }

                }

                // Ball hits from the right
                if (maxBlockX > (nextX - 1) &&
                    minBlockX < (nextX - 1) &&
                    (nextY + 1 > minBlockY && nextY + 1 < maxBlockY || nextY - 1 > minBlockY && nextY - 1 < maxBlockY) &&
                    ballSpeedX < 0) {
                    if (upYClip > 0 && upYClip < rightXClip || downYClip > 0 && downYClip < rightXClip) {

                    }
                    else {
                        ballSpeedX = -ballSpeedX;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }
                }

                // Ball hits from below
                if (nextY + 1 > minBlockY &&
                    nextY + 1 < maxBlockY &&
                    (nextX + 1 > minBlockX && nextX + 1 < maxBlockX || nextX - 1 > minBlockX && nextX - 1 < maxBlockX) &&
                    ballSpeedY > 0) {
                    if (leftXClip > 0 && leftXClip < downYClip || rightXClip > 0 && rightXClip < downYClip) {

                    }
                    else {
                        ballSpeedY = -ballSpeedY;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }
                }

                // Ball hits from above
                if (maxBlockY > (nextY - 1) &&
                    minBlockY < (nextY - 1) &&
                    (nextX + 1 > minBlockX && nextX + 1 < maxBlockX || nextX - 1 > minBlockX && nextX - 1 < maxBlockX) &&
                    ballSpeedY < 0) {
                    if (leftXClip > 0 && leftXClip < upYClip || rightXClip > 0 && rightXClip < upYClip) {

                    }
                    else {
                        ballSpeedY = -ballSpeedY;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }
                }
            }
            else {
                // Next ball position
                float nextX = ballX + deltaTimeSeconds * ballSpeedX;
                float nextY = ballY + deltaTimeSeconds * ballSpeedY;

                float leftXClip = (nextX + 1) - minBlockX;
                float rightXClip = maxBlockX - (nextX - 1);
                float upYClip = maxBlockY - (nextY - 1);
                float downYClip = (nextY + 1) - minBlockY;

                bool collided = false;

                if (nextX + 1 > minBlockX &&
                    nextX + 1 < maxBlockX &&
                    (nextY + 1 > minBlockY && nextY + 1 < maxBlockY || nextY - 1 > minBlockY && nextY - 1 < maxBlockY) &&
                    ballSpeedX > 0) {
                    if (upYClip > 0 && upYClip < leftXClip || downYClip > 0 && downYClip < leftXClip) {

                    }
                    else {
                        ballSpeedX = -ballSpeedX;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }

                }

                // Ball hits from the right
                if (maxBlockX > (nextX - 1) &&
                    minBlockX < (nextX - 1) &&
                    (nextY + 1 > minBlockY && nextY + 1 < maxBlockY || nextY - 1 > minBlockY && nextY - 1 < maxBlockY) &&
                    ballSpeedX < 0) {
                    if (upYClip > 0 && upYClip < rightXClip || downYClip > 0 && downYClip < rightXClip) {

                    }
                    else {
                        ballSpeedX = -ballSpeedX;
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }
                }

                // Ball hits from above
                if (maxBlockY > (nextY - 1) &&
                    minBlockY < (nextY - 1) &&
                    (nextX + 1 > minBlockX && nextX + 1 < maxBlockX || nextX - 1 > minBlockX && nextX - 1 < maxBlockX) &&
                    ballSpeedY < 0) {
                    if (leftXClip > 0 && leftXClip < upYClip || rightXClip > 0 && rightXClip < upYClip) {

                    }
                    else {
                        float ballSpeed = glm::sqrt(ballSpeedX * ballSpeedX + ballSpeedY * ballSpeedY);
                        float angle = (ballX - (maxBlockX + minBlockX) / 2) * 2 / (maxBlockX - minBlockX) * glm::radians(60.0f);

                        ballSpeedX = ballSpeed * glm::sin(angle);
                        ballSpeedY = ballSpeed * glm::cos(angle);
                        collided = true;
                        float partSpeed = 1.0 * (rand() % 100) / 50 + 0.2;
                        animations.push_back(Animation(nextX, nextY, ballSpeedX, ballSpeedY, 1, ANIMATION_PARTICLES, PARTICLE_WHITE, partSpeed));
                    }
                }
            }
        }
    }
    else {
        ballX = shipOffsetX;
    }

    // Add random background snow
    int tryNewSnow = rand() % 4;
    if (!tryNewSnow) {
        float snowX = 1.0 * (rand() % 16000) / 100;
        float snowY = 1.0 * (rand() % 10000) / 100;
        float snowSpeed = 1.0 * (rand() % 100) / 10;
        animations.push_back(Animation(snowX, snowY, 0, -1, 2, ANIMATION_SNOW, PARTICLE_WHITE, snowSpeed));
    }
    

    // Render the ball
    glm::mat3 ballPos = visMatrix * transform2D::Translate(ballX, ballY) * transform2D::Scale(2, 2);
    RenderMesh2D(meshes["circle"], shaders["VertexColor"], ballPos);

    // Decrease the remaining time of all animations, and stop the finished ones;
    for (int i = 0; i < animations.size(); i++) {
        animations[i].remainingDuration -= deltaTimeSeconds;
        if (animations[i].remainingDuration <= 0) {
            animations.erase(animations.begin() + i);
            i--;
        }
    }
    if (ssDuration > 0.001) {
        ssDuration -= deltaTimeSeconds;
    }
    else {
        logicSpace.x = 0;
        logicSpace.y = 0;
    }
}

void Tema1_player::FrameEnd() {
}

// Controlling
void Tema1_player::OnInputUpdate(float deltaTime, int mods)
{
    if (window->KeyHold(GLFW_KEY_LEFT)) {
        shipOffsetX = std::max(0.0f, shipOffsetX - deltaTime * 60);
    }
    if (window->KeyHold(GLFW_KEY_RIGHT)) {
        shipOffsetX = std::min(160.0f, shipOffsetX + deltaTime * 60);
    }
}


void Tema1_player::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_SPACE && !ballThrown) {
        ballThrown = true;
        ballSpeedX = 40;
        ballSpeedY = 40;
    }
    if (key == GLFW_KEY_LEFT_SHIFT) {
        ballSpeedX *= 2;
        ballSpeedY *= 2;
    }
}

void Tema1_player::OnKeyRelease(int key, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT) {
        ballSpeedX /= 2;
        ballSpeedY /= 2;
    }
}

void Tema1_player::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}

void Tema1_player::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{

}

void Tema1_player::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{

}

void Tema1_player::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1_player::OnWindowResize(int width, int height)
{
}

// Other functions for different purposes
void Tema1_player::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = 0;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    unsigned int IBO = 0;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    glBindVertexArray(0);

    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
        cout << "\t[NOTE] : For developers : This happens because OpenGL core spec >=3.1 forbids null VAOs." << std::endl;
    }

    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

// Viewport - Window
glm::mat3 Tema1_player::VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    tx = viewSpace.x - sx * logicSpace.x;
    ty = viewSpace.y - sy * logicSpace.y;

    return glm::transpose(glm::mat3(
        sx, 0.0f, tx,
        0.0f, sy, ty,
        0.0f, 0.0f, 1.0f));
}

glm::mat3 Tema1_player::VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty, smin;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    if (sx < sy)
        smin = sx;
    else
        smin = sy;
    tx = viewSpace.x - smin * logicSpace.x + (viewSpace.width - smin * logicSpace.width) / 2;
    ty = viewSpace.y - smin * logicSpace.y + (viewSpace.height - smin * logicSpace.height) / 2;

    return glm::transpose(glm::mat3(
        smin, 0.0f, tx,
        0.0f, smin, ty,
        0.0f, 0.0f, 1.0f));
}


void Tema1_player::SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear)
{
    glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(colorColor.r, colorColor.g, colorColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    GetSceneCamera()->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
    GetSceneCamera()->Update();
}