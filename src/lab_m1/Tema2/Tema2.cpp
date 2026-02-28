#include "lab_m1/Tema2/Tema2.h"

#include <vector>
#include <iostream>

#include "core/engine.h"
#include "utils/gl_utils.h"
#include "transform.h"

// Map constants
constexpr int MAP_SIZE = 101;
constexpr int MAP_VECTOR_INTERVAL = 50;
constexpr int MAP_CENTER = 50;

// Indexing constants (used to reprezent what exactly is on the map at said position)
constexpr int INDEX_TERRAIN = 1;
constexpr int INDEX_RIVER = 2;
constexpr int INDEX_MOUNTAIN = 4;
constexpr int INDEX_STATION = 8;
constexpr int INDEX_RAIL = 16;

constexpr int RESOURCE_BRICK = 2;
constexpr int RESOURCE_COAL = 1;
constexpr int RESOURCE_WOOD = 0;

constexpr bool DIRECTION_X = 0;
constexpr bool DIRECTION_Z = 1;

constexpr bool SENSE_PLUS = 0;
constexpr bool SENSE_MINUS = 1;

constexpr int PRESSED_A = 1;
constexpr int PRESSED_W = 2;
constexpr int PRESSED_D = 4;
constexpr int PRESSED_S = 8;

constexpr int COAL_REQUIRED = 1;
constexpr int WOOD_REQUIRED = 2;
constexpr int BRICK_REQUIRED = 3;

constexpr int HAVE_COAL = 1;
constexpr int HAVE_WOOD = 2;
constexpr int HAVE_BRICK = 3;

using namespace std;
using namespace m1;

Tema2::Tema2() {
}

Tema2::~Tema2() {
}

void Tema2::Init() {
    projectionMatrix = glm::perspective(RADIANS(75.0f), window->props.aspectRatio, 0.01f, 200.0f);

    Shader* shader = new Shader("SimpleShader");
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "Simple_VS.glsl"), GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "Simple_FS.glsl"), GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;

    Shader* shaderWater = new Shader("WaterShader");
    shaderWater->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "Water_VS.glsl"), GL_VERTEX_SHADER);
    shaderWater->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "Water_FS.glsl"), GL_FRAGMENT_SHADER);
    shaderWater->CreateAndLink();
    shaders[shaderWater->GetName()] = shaderWater;

    Shader* shaderUI = new Shader("UIShader");
    shaderUI->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "UI_VS.glsl"), GL_VERTEX_SHADER);
    shaderUI->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "UI_FS.glsl"), GL_FRAGMENT_SHADER);
    shaderUI->CreateAndLink();
    shaders[shaderUI->GetName()] = shaderUI;

    Shader* shaderUIMinimap = new Shader("UIMinimapShader");
    shaderUIMinimap->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "UI_minimap_VS.glsl"), GL_VERTEX_SHADER);
    shaderUIMinimap->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "Shaders", "UI_minimap_FS.glsl"), GL_FRAGMENT_SHADER);
    shaderUIMinimap->CreateAndLink();
    shaders[shaderUIMinimap->GetName()] = shaderUIMinimap;

    MakeLocomotive();
    MakeWagon();

    heightMap = vector<vector<float>>(MAP_SIZE, vector<float>(MAP_SIZE, 0));
    indexMap = vector<vector<unsigned int>>(MAP_SIZE, vector<unsigned int>(MAP_SIZE, INDEX_TERRAIN));
    GeneratePerlin();
    AddRiver();
    MakeRails();
    MakeStation();
    AddStations();
    AddRails();
    MakeRailway();
    MakeTerrain();
    MakeWater();
    MakeMinimapBase();

    auto coal = create_coal_vertices();
    CreateMesh("coal", coal.first, coal.second);

    auto wood = create_wood_vertices();
    CreateMesh("wood", wood.first, wood.second);

    auto brick = create_brick_vertices();
    CreateMesh("brick", brick.first, brick.second);

    auto minimap_main_station = create_square_vertices(1, 3, glm::vec3(0, 0, 0));
    CreateMesh("minimap_main_station", minimap_main_station.first, minimap_main_station.second);

    auto minimap_locomotive = create_triangle_vertices(glm::vec3(0.141, 0.89, 0.145));
    CreateMesh("minimap_locomotive", minimap_locomotive.first, minimap_locomotive.second);

    auto minimap_wagon = create_square_vertices(0.6, 0.8, glm::vec3(0.141, 0.89, 0.145));
    CreateMesh("minimap_wagon", minimap_wagon.first, minimap_wagon.second);

    railwayZ = vector<vector<LogicRail*>>(MAP_SIZE, vector<LogicRail*>(MAP_SIZE, nullptr));
    railwayX = vector<vector<LogicRail*>>(MAP_SIZE, vector<LogicRail*>(MAP_SIZE, nullptr));
    AddLogicRailway();
    AddTrain();
    lastDirectionPressed = PRESSED_S;

    resRotationHeight = 0;
    requiredResources = vector<int>();
    heldCoal = 0;
    heldWood = 0;
    heldBrick = 0;
    newCommandDelay = 5.0f;
    remainingTime = 120;
    gameLost = 0;
    score = 0;
    loadCooldown = 0.0f;
    windowWidth = window->GetResolution().x;
    windowHeight = window->GetResolution().y;
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, windowWidth, windowHeight);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "BitcountGridSingle_Roman-Light.ttf"), 140);
    heightScale = 1;
    widthScale = 1;
    pitch = 0.0f;

    camera = new cameraSpace::Camera(glm::vec3(3 * initialX, 3 * initialY + 15, 3 * initialZ - 20), glm::vec3(3 * initialX, 3 * initialY, 3 * initialZ), glm::normalize(glm::vec3(0, 20, 15)));
}

void Tema2::FrameStart() {
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0.278f, 0.82f, 0.91f, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

static float getInverse(float x) {
    return 1.0f / (x + 0.7f) - 0.7f;
}

void Tema2::Update(float deltaTimeSeconds) {
    if (gameLost) {
        RenderUI();
        return;
    }

    glm::mat4 terrainModel = transform::Scale(3, 3, 3);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    RenderSimpleMesh(meshes["terrain"], shaders["SimpleShader"], terrainModel);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);   // disable writing into the depth buffer
    RenderSimpleMesh(meshes["water"], shaders["WaterShader"], terrainModel);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    float centerX = 1.0f * stations[0].first - MAP_CENTER;
    float centerZ = 1.0f * stations[0].second - MAP_CENTER;
    glm::mat4 stationModel = transform::Translate(3 * centerX, 3 * heightMap[stations[0].first][stations[0].second] + 1.5f, 3 * centerZ) * transform::Scale(3, 3, 3);

    if (loadCooldown >= 0.0) {
        loadCooldown -= deltaTimeSeconds;
    }

    if (newCommandDelay - deltaTimeSeconds > 0.0f) {
        newCommandDelay -= deltaTimeSeconds;
    }
    else if (requiredResources.size() == 0) {
        score += remainingTime;
        for (int i = 0; i < 5; i++) {
            requiredResources.push_back(rand() % 3 + 1);
        }
        newCommandDelay = 0.0f;
        remainingTime = 120.0f;

    } else if (remainingTime > 0) {
        remainingTime -= deltaTimeSeconds;
    }
    else {
        gameLost = 1;
    }

    RenderSimpleMesh(meshes["mainStation"], shaders["SimpleShader"], stationModel);

    resRotationHeight += deltaTimeSeconds;
    for (int i = 1; i < 7; i++) {
        float centerX = 1.0f * stations[i].first - MAP_CENTER;
        float centerZ = 1.0f * stations[i].second - MAP_CENTER;
        glm::mat4 stationModel = transform::Translate(3 * centerX, 3 * heightMap[stations[i].first][stations[i].second] + 1.5f, 3 * centerZ) * transform::Scale(3, 3, 3);
        glm::mat4 resourceModel = transform::Translate(0, 7 + 2 * sin(2 * resRotationHeight), 0) * stationModel * transform::RotateOY(2 * resRotationHeight) * transform::Scale(0.5, 0.5, 0.5);

        if (i % 3 == RESOURCE_COAL) {
            RenderSimpleMesh(meshes["coalStation"], shaders["SimpleShader"], stationModel);
            RenderSimpleMesh(meshes["coal"], shaders["SimpleShader"], resourceModel);
        }
        else if (i % 3 == RESOURCE_BRICK) {
            RenderSimpleMesh(meshes["brickStation"], shaders["SimpleShader"], stationModel);
            RenderSimpleMesh(meshes["brick"], shaders["SimpleShader"], resourceModel);
        }
        else {
            RenderSimpleMesh(meshes["woodStation"], shaders["SimpleShader"], stationModel);
            RenderSimpleMesh(meshes["wood"], shaders["SimpleShader"], resourceModel * transform::RotateOZ(glm::radians(90.0f)) * transform::RotateOY(glm::radians(15.0f)));
        }
    }

    glDisable(GL_CULL_FACE);
    RenderSimpleMesh(meshes["railway"], shaders["SimpleShader"], terrainModel);
    glEnable(GL_CULL_FACE);

    float locomotiveX = 0;
    float locomotiveY = 0;
    float locomotiveZ = 0;
    float locomotiveOffsetX = 0;
    float locomotiveOffsetZ = 0;
    float locomotiveAngleY = 0;
    float locCornerAngle = 0;
    float wag1X = 0;
    float wag1Y = 0;
    float wag1Z = 0;
    float wag1OffsetX = 0;
    float wag1OffsetZ = 0;
    float wag1AngleY = 0;
    float wag1CornerAngle = 0;
    float wag2X = 0;
    float wag2Y = 0;
    float wag2Z = 0;
    float wag2OffsetX = 0;
    float wag2OffsetZ = 0;
    float wag2AngleY = 0;
    float wag2CornerAngle = 0;

    if (train.locSense == SENSE_PLUS) {
        float visualProg = train.locMoveProg - 0.25;
        if (visualProg < 0) {
            if (train.wag1Rail->heightStart > train.locomotiveRail->heightStart) {
                locomotiveAngleY = atan2(train.locomotiveRail->heightStart - train.wag1Rail->heightStart, 0.5);
                locomotiveY = lerp(train.wag1Rail->heightStart, train.locomotiveRail->heightStart, (1 + visualProg - 0.5f) * 2);
            }
            else {
                locomotiveY = train.locomotiveRail->heightStart;
            }
        }
        else if (visualProg < 0.5) {
            if (train.locomotiveRail->heightStart > train.locomotiveRail->heightEnd) {
                locomotiveY = train.locomotiveRail->heightStart;
            }
            else {
                locomotiveAngleY = atan2(train.locomotiveRail->heightEnd - train.locomotiveRail->heightStart, 0.5);
                locomotiveY = lerp(train.locomotiveRail->heightStart, train.locomotiveRail->heightEnd, visualProg * 2);
            }
        }
        else {
            if (train.locomotiveRail->heightStart > train.locomotiveRail->heightEnd) {
                locomotiveAngleY = atan2(train.locomotiveRail->heightEnd - train.locomotiveRail->heightStart, 0.5);
                locomotiveY = lerp(train.locomotiveRail->heightStart, train.locomotiveRail->heightEnd, (visualProg - 0.5) * 2);
            }
            else {
                locomotiveY = train.locomotiveRail->heightEnd;
            }
        }
    } else{
        float visualProg = train.locMoveProg - 0.25;
        if (visualProg < 0) {
            if (train.wag1Rail->heightEnd > train.locomotiveRail->heightEnd) {
                locomotiveAngleY = atan2(train.locomotiveRail->heightEnd - train.wag1Rail->heightEnd, 0.5);
                locomotiveY = lerp(train.wag1Rail->heightEnd, train.locomotiveRail->heightEnd, (1 + visualProg - 0.5f) * 2);
            }
            else {
                locomotiveY = train.locomotiveRail->heightEnd;
            }
        }
        else if (visualProg < 0.5) {
            if (train.locomotiveRail->heightEnd > train.locomotiveRail->heightStart) {
                locomotiveY = train.locomotiveRail->heightEnd;
            }
            else {
                locomotiveAngleY = atan2(train.locomotiveRail->heightStart - train.locomotiveRail->heightEnd, 0.5);
                locomotiveY = lerp(train.locomotiveRail->heightEnd, train.locomotiveRail->heightStart, visualProg * 2);
            }
        }
        else {
            if (train.locomotiveRail->heightEnd > train.locomotiveRail->heightStart) {
                locomotiveAngleY = atan2(train.locomotiveRail->heightStart - train.locomotiveRail->heightEnd, 0.5);
                locomotiveY = lerp(train.locomotiveRail->heightEnd, train.locomotiveRail->heightStart, (visualProg - 0.5) * 2);
            }
            else {
                locomotiveY = train.locomotiveRail->heightStart;
            }
        }
    }
    
    if (train.locomotiveRail->orientation == DIRECTION_Z) {
        float visualProg = train.locMoveProg - 0.25;
        locomotiveX = train.locomotiveRail->position;
        locomotiveOffsetX = 0;
        locomotiveOffsetZ = train.locSense == SENSE_PLUS ? -1.125 : 1.125;
        locomotiveZ = train.locSense == SENSE_PLUS ? lerp(train.locomotiveRail->start, train.locomotiveRail->end, train.locMoveProg):
            lerp(train.locomotiveRail->end, train.locomotiveRail->start, train.locMoveProg);
        if (train.wag2Rail->orientation == DIRECTION_X) {
            if (train.locSense == SENSE_MINUS && train.wag2Sense == SENSE_MINUS) {
                locomotiveOffsetX += 1 - train.locMoveProg;
                locomotiveOffsetZ += -1 + train.locMoveProg;
                locCornerAngle = 90 - 90 * (train.locMoveProg);
            }
            else if (train.locSense == SENSE_MINUS && train.wag2Sense == SENSE_PLUS) {
                locomotiveOffsetX += -1 + train.locMoveProg;
                locomotiveOffsetZ += -1 + train.locMoveProg;
                locCornerAngle = 270 + 90 * (train.locMoveProg);
            }
            else if (train.locSense == SENSE_PLUS && train.wag2Sense == SENSE_MINUS) {
                locomotiveOffsetX += 1 - train.locMoveProg;
                locomotiveOffsetZ += 1 - train.locMoveProg;
                locCornerAngle = 270 + 90 * train.locMoveProg;
            }
            else if (train.locSense == SENSE_PLUS && train.wag2Sense == SENSE_PLUS) {
                locomotiveOffsetX += -1 + train.locMoveProg;
                locomotiveOffsetZ += 1 - train.locMoveProg;
                locCornerAngle = 90 - 90 * (train.locMoveProg);
            }
        }
    }
    else {
        float visualProg = train.locMoveProg - 0.25;
        locomotiveOffsetX = train.locSense == SENSE_PLUS ? -1.125 : 1.125;
        locomotiveOffsetZ = 0;
        locomotiveZ = train.locomotiveRail->position;
        locomotiveX = train.locSense == SENSE_PLUS ? lerp(train.locomotiveRail->start, train.locomotiveRail->end, train.locMoveProg) :
            lerp(train.locomotiveRail->end, train.locomotiveRail->start, train.locMoveProg);
        if (train.wag2Rail->orientation == DIRECTION_Z) {
            if (train.locSense == SENSE_MINUS && train.wag2Sense == SENSE_MINUS) {
                locomotiveOffsetX += -1 + train.locMoveProg;
                locomotiveOffsetZ += 1 - train.locMoveProg;
                locCornerAngle = 270 + 90 * (train.locMoveProg);
            }
            else if (train.locSense == SENSE_MINUS && train.wag2Sense == SENSE_PLUS) {
                locomotiveOffsetX += -1 + train.locMoveProg;
                locomotiveOffsetZ += -1 + train.locMoveProg;
                locCornerAngle = 90 - 90 * (train.locMoveProg);
            }
            else if (train.locSense == SENSE_PLUS && train.wag2Sense == SENSE_MINUS) {
                locomotiveOffsetX += 1 - train.locMoveProg;
                locomotiveOffsetZ += 1 - train.locMoveProg;
                locCornerAngle = 90 - 90 * train.locMoveProg;
            }
            else if (train.locSense == SENSE_PLUS && train.wag2Sense == SENSE_PLUS) {
                locomotiveOffsetX += 1 - train.locMoveProg;
                locomotiveOffsetZ += -1 + train.locMoveProg;
                locCornerAngle = 270 + 90 * (train.locMoveProg);
            }
        }
    }

    glm::mat4 trainModel = transform::Translate(3 * locomotiveX + locomotiveOffsetX, 3 * 
        locomotiveY + 0.475, 3 * locomotiveZ + locomotiveOffsetZ) * transform::Scale(0.5, 0.5, 0.5);
    if (train.locomotiveRail->orientation == DIRECTION_X) {
        trainModel *= transform::RotateOY(glm::radians(90.0f));
    }
    if (train.locSense == SENSE_MINUS) {
        trainModel *= transform::RotateOY(glm::radians(180.0f));
    }
    trainModel *= transform::RotateOX(locomotiveAngleY);
    trainModel *= transform::RotateOY(glm::radians(locCornerAngle));
    RenderSimpleMesh(meshes["locomotive"], shaders["SimpleShader"], trainModel);

    camera->position.y = locomotiveY + 15;

    if (train.wag1Sense == SENSE_PLUS) {
        float visualProg = train.wag1MoveProg - 0.25;
        if (visualProg < 0) {
            if (train.wag2Rail->heightStart > train.wag1Rail->heightStart) {
                wag1AngleY = atan2(train.wag1Rail->heightStart - train.wag2Rail->heightStart, 0.5);
                wag1Y = lerp(train.wag2Rail->heightStart, train.wag1Rail->heightStart, (1 + visualProg - 0.5f) * 2);
            }
            else {
                wag1Y = train.wag1Rail->heightStart;
            }
        }
        else if (visualProg < 0.5) {
            if (train.wag1Rail->heightStart > train.wag1Rail->heightEnd) {
                wag1Y = train.wag1Rail->heightStart;
            }
            else {
                wag1AngleY = atan2(train.wag1Rail->heightEnd - train.wag1Rail->heightStart, 0.5);
                wag1Y = lerp(train.wag1Rail->heightStart, train.wag1Rail->heightEnd, visualProg * 2);
            }
        }
        else {
            if (train.wag1Rail->heightStart > train.wag1Rail->heightEnd) {
                wag1AngleY = atan2(train.wag1Rail->heightEnd - train.wag1Rail->heightStart, 0.5);
                wag1Y = lerp(train.wag1Rail->heightStart, train.wag1Rail->heightEnd, (visualProg - 0.5) * 2);
            }
            else {
                wag1Y = train.wag1Rail->heightEnd;
            }
        }
    }
    else {
        float visualProg = train.wag1MoveProg - 0.25;
        if (visualProg < 0) {
            if (train.wag2Rail->heightEnd > train.wag1Rail->heightEnd) {
                wag1AngleY = atan2(train.wag1Rail->heightEnd - train.wag2Rail->heightEnd, 0.5);
                wag1Y = lerp(train.wag2Rail->heightEnd, train.wag1Rail->heightEnd, (1 + visualProg - 0.5f) * 2);
            }
            else {
                wag1Y = train.wag1Rail->heightEnd;
            }
        }
        else if (visualProg < 0.5) {
            if (train.wag1Rail->heightEnd > train.wag1Rail->heightStart) {
                wag1Y = train.wag1Rail->heightEnd;
            }
            else {
                wag1AngleY = atan2(train.wag1Rail->heightStart - train.wag1Rail->heightEnd, 0.5);
                wag1Y = lerp(train.wag1Rail->heightEnd, train.wag1Rail->heightStart, visualProg * 2);
            }
        }
        else {
            if (train.wag1Rail->heightEnd > train.wag1Rail->heightStart) {
                wag1AngleY = atan2(train.wag1Rail->heightStart - train.wag1Rail->heightEnd, 0.5);
                wag1Y = lerp(train.wag1Rail->heightEnd, train.wag1Rail->heightStart, (visualProg - 0.5) * 2);
            }
            else {
                wag1Y = train.wag1Rail->heightStart;
            }
        }
    }

    if (train.wag1Rail->orientation == DIRECTION_Z) {
        float visualProg = train.wag1MoveProg - 0.25;
        wag1X = train.wag1Rail->position;
        wag1OffsetX = 0;
        wag1OffsetZ = (train.wag1Sense == SENSE_PLUS) ? -0.75 : 0.75;
        wag1Z = train.wag1Sense == SENSE_PLUS ? lerp(train.wag1Rail->start, train.wag1Rail->end, train.wag1MoveProg) :
            lerp(train.wag1Rail->end, train.wag1Rail->start, train.wag1MoveProg);
        if (train.wag1prevRail->orientation == DIRECTION_X) {
            if (train.wag1Sense == SENSE_MINUS && train.wag1prevSense == SENSE_MINUS) {
                wag1OffsetX += 1 - train.wag1MoveProg;
                wag1OffsetZ += -1 + train.wag1MoveProg;
                wag1CornerAngle = 90 - 90 * (train.wag1MoveProg);
            }
            else if (train.wag1Sense == SENSE_MINUS && train.wag1prevSense == SENSE_PLUS) {
                wag1OffsetX += -1 + train.wag1MoveProg;
                wag1OffsetZ += -1 + train.wag1MoveProg;
                wag1CornerAngle = 270 + 90 * (train.wag1MoveProg);
            }
            else if (train.wag1Sense == SENSE_PLUS && train.wag1prevSense == SENSE_MINUS) {
                wag1OffsetX += 1 - train.wag1MoveProg;
                wag1OffsetZ += 1 - train.wag1MoveProg;
                wag1CornerAngle = 270 + 90 * train.wag1MoveProg;
            }
            else if (train.wag1Sense == SENSE_PLUS && train.wag1prevSense == SENSE_PLUS) {
                wag1OffsetX += -1 + train.wag1MoveProg;
                wag1OffsetZ += 1 - train.wag1MoveProg;
                wag1CornerAngle = 90 - 90 * (train.wag1MoveProg);
            }
        }
    }
    else {
        float visualProg = train.wag1MoveProg - 0.25;
        wag1Z = train.wag1Rail->position;
        wag1OffsetZ = 0;
        wag1OffsetX = (train.wag1Sense == SENSE_PLUS) ? -0.75 : 0.75;
        wag1X = train.wag1Sense == SENSE_PLUS ? lerp(train.wag1Rail->start, train.wag1Rail->end, train.wag1MoveProg) :
            lerp(train.wag1Rail->end, train.wag1Rail->start, train.wag1MoveProg);
        if (train.wag1prevRail->orientation == DIRECTION_Z) {
            if (train.wag1Sense == SENSE_MINUS && train.wag1prevSense == SENSE_MINUS) {
                wag1OffsetX += -1 + train.wag1MoveProg;
                wag1OffsetZ += 1 - train.wag1MoveProg;
                wag1CornerAngle = 270 + 90 * (train.wag1MoveProg);
            }
            else if (train.wag1Sense == SENSE_MINUS && train.wag1prevSense == SENSE_PLUS) {
                wag1OffsetX += -1 + train.wag1MoveProg;
                wag1OffsetZ += -1 + train.wag1MoveProg;
                wag1CornerAngle = 90 - 90 * (train.wag1MoveProg);
            }
            else if (train.wag1Sense == SENSE_PLUS && train.wag1prevSense == SENSE_MINUS) {
                wag1OffsetX += 1 - train.wag1MoveProg;
                wag1OffsetZ += 1 - train.wag1MoveProg;
                wag1CornerAngle = 90 - 90 * train.wag1MoveProg;
            }
            else if (train.wag1Sense == SENSE_PLUS && train.wag1prevSense == SENSE_PLUS) {
                wag1OffsetX += 1 - train.wag1MoveProg;
                wag1OffsetZ += -1 + train.wag1MoveProg;
                wag1CornerAngle = 270 + 90 * (train.wag1MoveProg);
            }
        }
    }

    trainModel = transform::Translate(3 * wag1X + wag1OffsetX, 3 * wag1Y + 0.475, 3 * wag1Z + wag1OffsetZ) * transform::Scale(0.5, 0.5, 0.5);
    if (train.wag1Rail->orientation == DIRECTION_X) {
        trainModel *= transform::RotateOY(glm::radians(90.0f));
    }
    if (train.wag1Sense == SENSE_MINUS) {
        trainModel *= transform::RotateOY(glm::radians(180.0f));
    }
    trainModel *= transform::RotateOX(wag1AngleY);
    trainModel *= transform::RotateOY(glm::radians(wag1CornerAngle));
    RenderSimpleMesh(meshes["wagon"], shaders["SimpleShader"], trainModel);

    if (train.wag2Sense == SENSE_PLUS) {
        float visualProg = train.wag2MoveProg - 0.25;
        if (visualProg < 0) {
            if (train.prevRail->heightStart > train.wag2Rail->heightStart) {
                wag2AngleY = atan2(train.wag2Rail->heightStart - train.prevRail->heightStart, 0.5);
                wag2Y = lerp(train.prevRail->heightStart, train.wag2Rail->heightStart, (1 + visualProg - 0.5f) * 2);
            }
            else {
                wag2Y = train.wag2Rail->heightStart;
            }
        }
        else if (visualProg < 0.5) {
            if (train.wag2Rail->heightStart > train.wag2Rail->heightEnd) {
                wag2Y = train.wag2Rail->heightStart;
            }
            else {
                wag2AngleY = atan2(train.wag2Rail->heightEnd - train.wag2Rail->heightStart, 0.5);
                wag2Y = lerp(train.wag2Rail->heightStart, train.wag2Rail->heightEnd, visualProg * 2);
            }
        }
        else {
            if (train.wag2Rail->heightStart > train.wag2Rail->heightEnd) {
                wag2AngleY = atan2(train.wag2Rail->heightEnd - train.wag2Rail->heightStart, 0.5);
                wag2Y = lerp(train.wag2Rail->heightStart, train.wag2Rail->heightEnd, (visualProg - 0.5) * 2);
            }
            else {
                wag2Y = train.wag2Rail->heightEnd;
            }
        }
    }
    else {
        float visualProg = train.wag2MoveProg - 0.25;
        if (visualProg < 0) {
            if (train.prevRail->heightEnd > train.wag2Rail->heightEnd) {
                wag2AngleY = atan2(train.wag2Rail->heightEnd - train.prevRail->heightEnd, 0.5);
                wag2Y = lerp(train.prevRail->heightEnd, train.wag2Rail->heightEnd, (1 + visualProg - 0.5f) * 2);
            }
            else {
                wag2Y = train.wag2Rail->heightEnd;
            }
        }
        else if (visualProg < 0.5) {
            if (train.wag2Rail->heightEnd > train.wag2Rail->heightStart) {
                wag2Y = train.wag2Rail->heightEnd;
            }
            else {
                wag2AngleY = atan2(train.wag2Rail->heightStart - train.wag2Rail->heightEnd, 0.5);
                wag2Y = lerp(train.wag2Rail->heightEnd, train.wag2Rail->heightStart, visualProg * 2);
            }
        }
        else {
            if (train.wag2Rail->heightEnd > train.wag2Rail->heightStart) {
                wag2AngleY = atan2(train.wag2Rail->heightStart - train.wag2Rail->heightEnd, 0.5);
                wag2Y = lerp(train.wag2Rail->heightEnd, train.wag2Rail->heightStart, (visualProg - 0.5) * 2);
            }
            else {
                wag2Y = train.wag2Rail->heightStart;
            }
        }
    }

    if (train.wag2Rail->orientation == DIRECTION_Z) {
        float visualProg = train.wag2MoveProg - 0.25;
        wag2X = train.wag2Rail->position;
        wag2OffsetX = 0;
        wag2OffsetZ = (train.wag2Sense == SENSE_PLUS) ? -0.75 : 0.75;
        wag2Z = train.wag2Sense == SENSE_PLUS ? lerp(train.wag2Rail->start, train.wag2Rail->end, train.wag2MoveProg) :
            lerp(train.wag2Rail->end, train.wag2Rail->start, train.wag2MoveProg);
        if (train.prevRail->orientation == DIRECTION_X) {
            if (train.wag2Sense == SENSE_MINUS && train.prevSense == SENSE_MINUS) {
                wag2OffsetX += 1 - train.wag2MoveProg;
                wag2OffsetZ += -1 + train.wag2MoveProg;
                wag2CornerAngle = 90 - 90 * (train.wag2MoveProg);
            }
            else if (train.wag2Sense == SENSE_MINUS && train.prevSense == SENSE_PLUS) {
                wag2OffsetX += -1 + train.wag2MoveProg;
                wag2OffsetZ += -1 + train.wag2MoveProg;
                wag2CornerAngle = 270 + 90 * (train.wag2MoveProg);
            }
            else if (train.wag2Sense == SENSE_PLUS && train.prevSense == SENSE_MINUS) {
                wag2OffsetX += 1 - train.wag2MoveProg;
                wag2OffsetZ += 1 - train.wag2MoveProg;
                wag2CornerAngle = 270 + 90 * train.wag2MoveProg;
            }
            else if (train.wag2Sense == SENSE_PLUS && train.prevSense == SENSE_PLUS) {
                wag2OffsetX += -1 + train.wag2MoveProg;
                wag2OffsetZ += 1 - train.wag2MoveProg;
                wag2CornerAngle = 90 - 90 * (train.wag2MoveProg);
            }
        }
    }
    else {
        float visualProg = train.wag2MoveProg - 0.25;
        wag2Z = train.wag2Rail->position;
        wag2OffsetZ = 0;
        wag2OffsetX = (train.wag2Sense == SENSE_PLUS) ? -0.75 : 0.75;
        wag2X = train.wag2Sense == SENSE_PLUS ? lerp(train.wag2Rail->start, train.wag2Rail->end, train.wag2MoveProg) :
            lerp(train.wag2Rail->end, train.wag2Rail->start, train.wag2MoveProg);
        if (train.prevRail->orientation == DIRECTION_Z) {
            if (train.wag2Sense == SENSE_MINUS && train.prevSense == SENSE_MINUS) {
                wag2OffsetX += -1 + train.wag2MoveProg;
                wag2OffsetZ += 1 - train.wag2MoveProg;
                wag2CornerAngle = 270 + 90 * (train.wag2MoveProg);
            }
            else if (train.wag2Sense == SENSE_MINUS && train.prevSense == SENSE_PLUS) {
                wag2OffsetX += -1 + train.wag2MoveProg;
                wag2OffsetZ += -1 + train.wag2MoveProg;
                wag2CornerAngle = 90 - 90 * (train.wag2MoveProg);
            }
            else if (train.wag2Sense == SENSE_PLUS && train.prevSense == SENSE_MINUS) {
                wag2OffsetX += 1 - train.wag2MoveProg;
                wag2OffsetZ += 1 - train.wag2MoveProg;
                wag2CornerAngle = 90 - 90 * train.wag2MoveProg;
            }
            else if (train.wag2Sense == SENSE_PLUS && train.prevSense == SENSE_PLUS) {
                wag2OffsetX += 1 - train.wag2MoveProg;
                wag2OffsetZ += -1 + train.wag2MoveProg;
                wag2CornerAngle = 270 + 90 * (train.wag2MoveProg);
            }
        }
    }

    trainModel = transform::Translate(3 * wag2X + wag2OffsetX, 3 * wag2Y + 0.475, 3 * wag2Z + wag2OffsetZ) * transform::Scale(0.5, 0.5, 0.5);
    if (train.wag2Rail->orientation == DIRECTION_X) {
        trainModel *= transform::RotateOY(glm::radians(90.0f));
    }
    if (train.wag2Sense == SENSE_MINUS) {
        trainModel *= transform::RotateOY(glm::radians(180.0f));
    }
    trainModel *= transform::RotateOY(glm::radians(wag2CornerAngle));
    trainModel *= transform::RotateOX(wag2AngleY);
    RenderSimpleMesh(meshes["wagon"], shaders["SimpleShader"], trainModel);

    UpdateTrain(deltaTimeSeconds);
    HandleTrainDeliveries();

    RenderUI();
    if (gameLost) {
        return;
    }
}

void Tema2::FrameEnd()
{
}

// Controlling
void Tema2::OnInputUpdate(float deltaTime, int mods)
{
}


void Tema2::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_A) {
        lastDirectionPressed |= PRESSED_D;
        lastDirectionPressed &= (~PRESSED_S);
    }
    if (key == GLFW_KEY_D) {
        lastDirectionPressed |= PRESSED_A;
        lastDirectionPressed &= (~PRESSED_S);
    }
    if (key == GLFW_KEY_W) {
        lastDirectionPressed |= PRESSED_W;
        lastDirectionPressed &= (~PRESSED_S);
    }
    if (key == GLFW_KEY_S) {
        lastDirectionPressed |= PRESSED_S;
    }
}

void Tema2::OnKeyRelease(int key, int mods)
{
    if (key == GLFW_KEY_A) {
        lastDirectionPressed &= (~PRESSED_D);
    }
    if (key == GLFW_KEY_D) {
        lastDirectionPressed &= (~PRESSED_A);
    }
    /*if (key == GLFW_KEY_W) {
        lastDirectionPressed &= (~PRESSED_W);
    }*/
    if (key == GLFW_KEY_S) {
        lastDirectionPressed &= (~PRESSED_S);
    }
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;

        camera->RotateThirdPerson_OY(-deltaX * sensivityOX);

        float deltaPitch = -deltaY * sensivityOY;

        float maxPitch = glm::radians(40.0f);
        float minPitch = glm::radians(-10.0f);

        float newPitch = pitch + deltaPitch;

        if (newPitch > maxPitch) {
            deltaPitch = maxPitch - pitch;
            pitch = maxPitch;
        }
        else if (newPitch < minPitch) {
            deltaPitch = minPitch - pitch;
            pitch = minPitch;
        }
        else {
            pitch = newPitch;
        }

        camera->RotateThirdPerson_OX(deltaPitch);
    }
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{

}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{

}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    train.speed = glm::min(10.0f, glm::max(0.0f, train.speed + offsetY));
}


void Tema2::OnWindowResize(int width, int height)
{
    widthScale = width / 1280;
    heightScale = height / 720;
    windowHeight = height;
    windowWidth = width;
    textRenderer = new gfxc::TextRenderer(window->props.selfDir, windowWidth, windowHeight);
    textRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "BitcountGridSingle_Roman-Light.ttf"), 140);
}

// Helper functions for meshes
void Tema2::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    unsigned int VAO = 0;
    // Create the VAO and bind it
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create the VBO and bind it
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    // Create the IBO and bind it
    unsigned int IBO;
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    CheckOpenGLError();

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
    meshes[name]->vertices = vertices;
    meshes[name]->indices = indices;
}

void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Get shader location for uniform mat4 "Model"
    int modelUniformLoc = glGetUniformLocation(shader->GetProgramID(), "Model");

    // Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(modelUniformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Get shader location for uniform mat4 "View"
    int viewUniformLoc = glGetUniformLocation(shader->GetProgramID(), "View");

    // Set shader uniform "View" to viewMatrix
    glm::mat4 viewMatrix = camera->GetViewMatrix();

    // Get shader location for uniform mat4 "Projection"
    glUniformMatrix4fv(viewUniformLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Set shader uniform "Projection" to projectionMatrix
    int projUniformLoc = glGetUniformLocation(shader->GetProgramID(), "Projection");
    glUniformMatrix4fv(projUniformLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    //Set the light position uniform
    int lightPosLoc = glGetUniformLocation(shader->GetProgramID(), "lightPos");
    glm::vec3 lightPos = glm::vec3(0, 125, 0);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema2::MakeLocomotive() {
    auto locomotive = create_cube_vertices(1.0f, 1.0f, 1.0f, glm::vec3(0.557f, 0.89f, 0.004f));
    glm::mat4 newPos = glm::mat4(1);

    auto engine = create_tube_vertices(0.25f, 1.5f, glm::vec3(0.035f, 0.224f, 0.671f));
    newPos = transform::Translate(0, -0.25f, 1.25f) * transform::RotateOX(glm::radians(90.0f));
    add_vertices(locomotive, engine, newPos);

    auto platform = create_cube_vertices(1, 0.1f, 2.75f, glm::vec3(0.91f, 0.941f, 0.231f));
    newPos = transform::Translate(0, -0.55f, 0.875);
    add_vertices(locomotive, platform, newPos);

    auto nose = create_tube_vertices(0.05, 0.125, glm::vec3(0.169, 0.169, 0.149));
    newPos = transform::Translate(0, -0.25, 2) * transform::RotateOX(glm::radians(90.0f));
    add_vertices(locomotive, nose, newPos);

    auto wheel = create_tube_vertices(0.175, 0.2, glm::vec3(0.722, 0.275, 0.251));
    auto axle = create_tube_vertices(0.04, 0.95, glm::vec3(0.169, 0.169, 0.149));
    float z = -0.25;
    float step = 0.38;
    for (int i = 0; i < 7; i++) {
        newPos = transform::Translate(0.35, -0.775, z) * transform::RotateOZ(glm::radians(90.0f));
        add_vertices(locomotive, wheel, newPos);

        newPos = transform::Translate(-0.35, -0.775, z) * transform::RotateOZ(glm::radians(90.0f));
        add_vertices(locomotive, wheel, newPos);

        newPos = transform::Translate(0, -0.775, z) * transform::RotateOZ(glm::radians(90.0f));
        add_vertices(locomotive, axle, newPos);
        z += step;
    }

    auto hook = create_tube_vertices(0.045, 0.25f, glm::vec3(0.169, 0.169, 0.149));
    newPos = transform::Translate(0, -0.545, -0.6125) * transform::RotateOX(glm::radians(90.0f));
    add_vertices(locomotive, hook, newPos);

    auto hookEnd = create_cube_vertices(0.1, 0.1, 0.1, glm::vec3(0.169, 0.169, 0.149));
    newPos = transform::Translate(0, -0.55, -0.75);
    add_vertices(locomotive, hookEnd, newPos);

    CreateMesh("locomotive", locomotive.first, locomotive.second);
}

void Tema2::MakeWagon() {
    auto wagon = create_cube_vertices(1.0f, 1.0f, 2.75f, glm::vec3(0.557f, 0.89f, 0.004f));
    glm::mat4 newPos = glm::mat4(1);

    auto platform = create_cube_vertices(1, 0.1f, 2.75f, glm::vec3(0.91f, 0.941f, 0.231f));
    newPos = transform::Translate(0, -0.55f, 0);
    add_vertices(wagon, platform, newPos);

    auto wheel = create_tube_vertices(0.175, 0.2, glm::vec3(0.722, 0.275, 0.251));
    auto axle = create_tube_vertices(0.04, 0.95, glm::vec3(0.169, 0.169, 0.149));
    float z = -1.15;
    float step = 2.25;
    for (int i = 0; i < 2; i++) {
        newPos = transform::Translate(0.35, -0.775, z) * transform::RotateOZ(glm::radians(90.0f));
        add_vertices(wagon, wheel, newPos);

        newPos = transform::Translate(-0.35, -0.775, z) * transform::RotateOZ(glm::radians(90.0f));
        add_vertices(wagon, wheel, newPos);

        newPos = transform::Translate(0, -0.775, z) * transform::RotateOZ(glm::radians(90.0f));
        add_vertices(wagon, axle, newPos);
        z += step;
    }

    auto hook = create_tube_vertices(0.045, 0.125f, glm::vec3(0.169, 0.169, 0.149));
    newPos = transform::Translate(0, -0.545, -1.438) * transform::RotateOX(glm::radians(90.0f));
    add_vertices(wagon, hook, newPos);
    newPos = transform::Translate(0, -0.545, 1.438) * transform::RotateOX(glm::radians(90.0f));
    add_vertices(wagon, hook, newPos);

    auto hookEnd = create_cube_vertices(0.1, 0.1, 0.1, glm::vec3(0.169, 0.169, 0.149));
    newPos = transform::Translate(0, -0.55, -1.5);
    add_vertices(wagon, hookEnd, newPos);
    newPos = transform::Translate(0, -0.55, 1.5);
    add_vertices(wagon, hookEnd, newPos);

    CreateMesh("wagon", wagon.first, wagon.second);
}


// Terrain generation functions
static float getSmoothstepAt(float a, float b, float t) {
    return a + (b - a) * t * t * t * (t * (t * 6 - 15) + 10);
}

static float getDotAt(int i, int j, int cornerI, int cornerJ, glm::vec2 randVect) {
    int di = i - MAP_VECTOR_INTERVAL * cornerI;
    int dj = j - MAP_VECTOR_INTERVAL * cornerJ;
    return glm::dot(glm::vec2((float)di, (float)dj), randVect);
}

void Tema2::GeneratePerlin() {
    int corners = MAP_SIZE / MAP_VECTOR_INTERVAL + 1;
    vector<vector<glm::vec2>> gridCornerVectors(corners, vector<glm::vec2>(corners, glm::vec2(0)));

    for (int i = 0; i < corners; i++) {
        for (int j = 0; j < corners; j++) {
            float randomX = ((rand() % 1000) - 500) / 500.0f;
            float randomY = ((rand() % 1000) - 500) / 500.0f;
            gridCornerVectors[i][j] = glm::normalize(glm::vec2(randomX, randomY));
        }
    }

    for (int i = 0; i < MAP_SIZE - 1; i++) {
        for (int j = 0; j < MAP_SIZE - 1; j++) {
            int cornerJ0 = j / MAP_VECTOR_INTERVAL;
            int cornerJ1 = cornerJ0 + 1;
            int cornerI0 = i / MAP_VECTOR_INTERVAL;
            int cornerI1 = cornerI0 + 1;

            float topLeft = getDotAt(i, j, cornerI0, cornerJ0, gridCornerVectors[cornerI0][cornerJ0]);
            float topRight = getDotAt(i, j, cornerI0, cornerJ1, gridCornerVectors[cornerI0][cornerJ1]);
            float bottomLeft = getDotAt(i, j, cornerI1, cornerJ0, gridCornerVectors[cornerI1][cornerJ0]);
            float bottomRight = getDotAt(i, j, cornerI1, cornerJ1, gridCornerVectors[cornerI1][cornerJ1]);

            float ti = 1.0f * (i - cornerI0 * MAP_VECTOR_INTERVAL) / MAP_VECTOR_INTERVAL;
            float tj = 1.0f * (j - cornerJ0 * MAP_VECTOR_INTERVAL) / MAP_VECTOR_INTERVAL;

            float top = getSmoothstepAt(topLeft, topRight, tj);
            float bottom = getSmoothstepAt(bottomLeft, bottomRight, tj);
            heightMap[i][j] = max(0.5f * getSmoothstepAt(top, bottom, ti), -3.0f);

            if (heightMap[i][j] > 3.5) {
                heightMap[i][j] *= 3.5;
                indexMap[i][j] = INDEX_MOUNTAIN;
            }
        }
    }

    for (int i = 0; i < MAP_SIZE - 1; i++) {
        int cornerI0 = i / MAP_VECTOR_INTERVAL;
        int cornerI1 = cornerI0 + 1;
        int cornerJ = MAP_SIZE / MAP_VECTOR_INTERVAL;

        float top = getDotAt(i, MAP_SIZE - 1, cornerI0, cornerJ, gridCornerVectors[cornerI0][cornerJ]);
        float bottom = getDotAt(i, MAP_SIZE - 1, cornerI1, cornerJ, gridCornerVectors[cornerI1][cornerJ]);

        float ti = 1.0f * (i - cornerI0 * MAP_VECTOR_INTERVAL) / MAP_VECTOR_INTERVAL;
        heightMap[i][MAP_SIZE - 1] = max(0.5f * getSmoothstepAt(top, bottom, ti), -3.0f);

        if (heightMap[i][MAP_SIZE - 1] > 3.5) {
            heightMap[i][MAP_SIZE - 1] *= 3.5;
            indexMap[i][MAP_SIZE - 1] = INDEX_MOUNTAIN;
        }
    }

    for (int j = 0; j < MAP_SIZE - 1; j++) {
        int cornerJ0 = j / MAP_VECTOR_INTERVAL;
        int cornerJ1 = cornerJ0 + 1;
        int cornerI = MAP_SIZE / MAP_VECTOR_INTERVAL;

        float left = getDotAt(MAP_SIZE - 1, j, cornerI, cornerJ0, gridCornerVectors[cornerI][cornerJ0]);
        float right = getDotAt(MAP_SIZE - 1, j, cornerI, cornerJ1, gridCornerVectors[cornerI][cornerJ1]);

        float tj = 1.0f * (j - cornerJ0 * MAP_VECTOR_INTERVAL) / MAP_VECTOR_INTERVAL;
        heightMap[MAP_SIZE - 1][j] = max(0.5f * getSmoothstepAt(left, right, tj), -3.0f);

        if (heightMap[MAP_SIZE - 1][j] > 3.5) {
            heightMap[MAP_SIZE - 1][j] *= 3.5;
            indexMap[MAP_SIZE - 1][j] = INDEX_MOUNTAIN;
        }
    }
}

void Tema2::AddRiver() {
    // Generate a river, using a Bezier curve of order 3
    int leftI = rand() % MAP_SIZE;
    int leftMiddleI = 20;
    int rightMiddleI = 80;
    int rightI = rand() % MAP_SIZE;

    for (int j = 0; j < MAP_SIZE; j++) {
        float t = j / 100.0f;
        float pointI = pow(1 - t, 3) * leftI + 3 * pow(1 - t, 2) * t * leftMiddleI + 3 * (1 - t) * t * t * rightMiddleI + t * t * t * rightI;
        int I = (int)pointI;
        for (int i = I - 3; i <= I + 3; i++) {
            if (i < 0 || i >= MAP_SIZE) continue;
            if (abs(I - i) == 3) {
                heightMap[i][j] = -3.4f;
            }
            else if (abs(I - i) == 2) {
                heightMap[i][j] = -4.2f;
            }
            else if (abs(I - i) == 1) {
                heightMap[i][j] = -4.6f;
            }
            else {
                heightMap[i][j] = -5.0f;
            }
            indexMap[i][j] = INDEX_RIVER;
        }
    }
}

void Tema2::MakeTerrain() {
    // Bulky function, trust the process
    pair<vector<VertexFormat>, vector<unsigned int>> terrain = { vector<VertexFormat>(), vector<unsigned int>() };
    glm::vec3 terrain_color = glm::vec3(0.412f, 0.812f, 0.071f);
    glm::vec3 river_bed_color = glm::vec3(0.859f, 0.851f, 0.133f);
    glm::vec3 mountain_color = glm::vec3(0.51, 0.482, 0.459);
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            // i will be X and j will be Z - decided by a fair coin flip
            float offsetX = (i - MAP_CENTER);
            float offsetZ = (j - MAP_CENTER);

            float use_height = heightMap[i][j];

            glm::vec3 use_color = glm::vec3(1);
            if (indexMap[i][j] & INDEX_TERRAIN) {
                use_color = terrain_color;
            }
            else if (indexMap[i][j] & INDEX_RIVER) {
                use_color = river_bed_color;
            }
            else if (indexMap[i][j] & INDEX_MOUNTAIN) {
                use_color = mountain_color;
                if (indexMap[i][j] & INDEX_RAIL) {
                    use_height = 1.5f;
                }
            }

            // Y+
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(0, 1, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(0, 1, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(0, 1, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(0, 1, 0)));

            // Z+, ZN+
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(0, 0, 1)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(0, 0, 1)));

            // Z+, ZN-
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(0, 0, -1)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(0, 0, -1)));

            // Z- ZN+
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(0, 0, +1)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(0, 0, +1)));

            // Z- ZN-
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(0, 0, -1)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(0, 0, -1)));

            // X+ XN+
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(1, 0, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(1, 0, 0)));

            // X+ XN-
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(-1, 0, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(-1, 0, 0)));

            // X- XN+
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(1, 0, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(1, 0, 0)));

            // X- XN-
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ - 0.5), use_color, glm::vec3(-1, 0, 0)));
            terrain.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, use_height, offsetZ + 0.5), use_color, glm::vec3(-1, 0, 0)));

            int current_face_idx = 20 * (MAP_SIZE * i + j);

            // Render top face
            terrain.second.push_back(current_face_idx + 0);
            terrain.second.push_back(current_face_idx + 1);
            terrain.second.push_back(current_face_idx + 3);
            terrain.second.push_back(current_face_idx + 0);
            terrain.second.push_back(current_face_idx + 3);
            terrain.second.push_back(current_face_idx + 2);

            // Render the face towards i - 1 (Which I will consider X- here)
            // If the current height is bigger than the one at i - 1, that means the face is oriented towards X-
            // Otherwise, it is oriented towards X+

            if (i) {
                int top_face_idx = 20 * (MAP_SIZE * (i - 1) + j);
                if (heightMap[i][j] > heightMap[i - 1][j]) {
                    terrain.second.push_back(current_face_idx + 19);
                    terrain.second.push_back(current_face_idx + 18);
                    terrain.second.push_back(top_face_idx + 14);
                    terrain.second.push_back(current_face_idx + 19);
                    terrain.second.push_back(top_face_idx + 14);
                    terrain.second.push_back(top_face_idx + 15);
                }
                else {
                    terrain.second.push_back(current_face_idx + 17);
                    terrain.second.push_back(current_face_idx + 16);
                    terrain.second.push_back(top_face_idx + 12);
                    terrain.second.push_back(current_face_idx + 17);
                    terrain.second.push_back(top_face_idx + 12);
                    terrain.second.push_back(top_face_idx + 13);
                }
            }

            // Render the face towards j - 1 (Which I will consider Z- here)
            // If the current height is bigger than the one at j - 1, that means the face is oriented towards Z-
            // Otherwise, it is oriented towards Z+

            if (j) {
                int left_face_idx = 20 * (MAP_SIZE * i + j - 1);
                if (heightMap[i][j] > heightMap[i][j - 1]) {
                    terrain.second.push_back(current_face_idx + 10);
                    terrain.second.push_back(current_face_idx + 11);
                    terrain.second.push_back(left_face_idx + 6);
                    terrain.second.push_back(current_face_idx + 11);
                    terrain.second.push_back(left_face_idx + 7);
                    terrain.second.push_back(left_face_idx + 6);
                }
                else {
                    terrain.second.push_back(current_face_idx + 8);
                    terrain.second.push_back(current_face_idx + 9);
                    terrain.second.push_back(left_face_idx + 4);
                    terrain.second.push_back(current_face_idx + 9);
                    terrain.second.push_back(left_face_idx + 5);
                    terrain.second.push_back(left_face_idx + 4);
                }
            }
        }
    }

    int current_vertices = terrain.first.size();
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if ((indexMap[i][j] & (INDEX_MOUNTAIN | INDEX_RAIL)) == (INDEX_MOUNTAIN | INDEX_RAIL)) {
                float offsetX = (i - MAP_CENTER);
                float offsetZ = (j - MAP_CENTER);
                float height = (heightMap[i][j] - 1.5f) / 2;
                auto tunnelTopside = create_cube_vertices(1, height, 1, mountain_color);
                glm::mat4 newPos = transform::Translate(offsetX, heightMap[i][j] - height / 2, offsetZ);
                add_vertices(terrain, tunnelTopside, newPos);
            }
            
        }
    }

    CreateMesh("terrain", terrain.first, terrain.second);
}

void Tema2::MakeWater() {
    pair<vector<VertexFormat>, vector<unsigned int>> water = { vector<VertexFormat>(), vector<unsigned int>() };
    glm::vec3 color = glm::vec3(0.133f, 0.647f, 0.859f);
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (indexMap[i][j] & INDEX_RIVER) {
                float offsetX = (i - MAP_CENTER);
                float offsetZ = (j - MAP_CENTER);

                water.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, -3.2, offsetZ - 0.5), color, glm::vec3(0, 1, 0)));
                water.first.push_back(VertexFormat(glm::vec3(offsetX - 0.5, -3.2, offsetZ + 0.5), color, glm::vec3(0, 1, 0)));
                water.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, -3.2, offsetZ - 0.5), color, glm::vec3(0, 1, 0)));
                water.first.push_back(VertexFormat(glm::vec3(offsetX + 0.5, -3.2, offsetZ + 0.5), color, glm::vec3(0, 1, 0)));

                int current_face_idx = water.first.size() - 4;

                // Render top face
                water.second.push_back(current_face_idx + 0);
                water.second.push_back(current_face_idx + 1);
                water.second.push_back(current_face_idx + 3);
                water.second.push_back(current_face_idx + 0);
                water.second.push_back(current_face_idx + 3);
                water.second.push_back(current_face_idx + 2);
            }
        }
    }

    CreateMesh("water", water.first, water.second);
}

void Tema2::MakeRails() {
    glm::vec3 railColor = glm::vec3(0.271, 0.306, 0.31);

    vector<VertexFormat> verticesTerrainRail = {
        VertexFormat(glm::vec3(-0.25, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.25, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.25, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.25, 0, 0.25), railColor, glm::vec3(0, 1, 0))
    };

    vector<unsigned int> indicesTerrainRail = {
        0, 1, 3,
        0, 3, 2
    };

    CreateMesh("terrain_rail", verticesTerrainRail, indicesTerrainRail);

    vector<VertexFormat> verticesBridgeRail = {
        VertexFormat(glm::vec3(-0.25, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.25, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.15, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.15, 0, 0.25), railColor, glm::vec3(0, 1, 0)),

        VertexFormat(glm::vec3(-0.05, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.05, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.05, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.05, 0, 0.25), railColor, glm::vec3(0, 1, 0)),

        VertexFormat(glm::vec3(0.15, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.15, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.25, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.25, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
    };

    vector<unsigned int> indicesBridgeRail = {
        0, 1, 3,
        0, 3, 2,

        4, 5, 7,
        4, 7, 6,

        8, 9, 11,
        8, 11, 10
    };

    CreateMesh("bridge_rail", verticesBridgeRail, indicesBridgeRail);

    vector<VertexFormat> verticesTunnelRail = {
        VertexFormat(glm::vec3(-0.25, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.25, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.1, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.1, 0, 0.25), railColor, glm::vec3(0, 1, 0)),

        VertexFormat(glm::vec3(-0.1, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.1, 0, -0.15), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.1, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.1, 0, -0.15), railColor, glm::vec3(0, 1, 0)),

        VertexFormat(glm::vec3(-0.1, 0, 0.15), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(-0.1, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.1, 0, 0.15), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.1, 0, 0.25), railColor, glm::vec3(0, 1, 0)),

        VertexFormat(glm::vec3(0.1, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.1, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.25, 0, -0.25), railColor, glm::vec3(0, 1, 0)),
        VertexFormat(glm::vec3(0.25, 0, 0.25), railColor, glm::vec3(0, 1, 0)),
    };

    vector<unsigned int> indicesTunnelRail = {
        0, 1, 3,
        0, 3, 2,

        4, 5, 7,
        4, 7, 6,

        8, 9, 11,
        8, 11, 10,

        12, 13, 15,
        12, 15, 14
    };

    CreateMesh("tunnel_rail", verticesTunnelRail, indicesTunnelRail);
}

void Tema2::MakeStation() {
    auto woodStation = create_cube_vertices(1, 1, 1, glm::vec3(0.6f, 0.545f, 0.62f));
    auto roofShort = create_pyramid_vertices(1, 1, glm::vec3(0.678f, 0.282f, 0.169f));

    glm::mat4 newPosition = glm::mat4(1);
    newPosition = transform::Translate(0, 0.5f, 0);
    add_vertices(woodStation, roofShort, newPosition);

    auto coalStation = create_cube_vertices(1, 1, 1, glm::vec3(0.6f, 0.545f, 0.62f));
    auto chimney = create_cube_vertices(0.2, 1, 0.2, glm::vec3(1, 1, 1));
    newPosition = transform::Translate(0, 0.5, 0.3);
    add_vertices(coalStation, chimney, newPosition);

    auto mainStation = create_cube_vertices(1, 1, 3, glm::vec3(0.6f, 0.545f, 0.62f));
    auto roofTall = create_pyramid_vertices(2, 1, glm::vec3(0.678f, 0.282f, 0.169f));
    newPosition = transform::Translate(0, 0.5, 0);
    add_vertices(mainStation, roofShort, newPosition);
    newPosition = transform::Translate(0, 0.5, 1);
    add_vertices(mainStation, roofTall, newPosition);
    newPosition = transform::Translate(0, 0.5, -1);
    add_vertices(mainStation, roofTall, newPosition);

    auto brickStation = create_cube_vertices(0.5, 0.5, 1, glm::vec3(0.6f, 0.545f, 0.62f));
    auto building = create_cube_vertices(1, 1, 1, glm::vec3(0.6f, 0.545f, 0.62f));
    newPosition = transform::Translate(0, 0, 1);
    add_vertices(brickStation, building, newPosition);
    newPosition = transform::Translate(0, 0.5, 1);
    add_vertices(brickStation, roofShort, newPosition);
    newPosition = transform::Translate(0, 0, -1);
    add_vertices(brickStation, building, newPosition);

    CreateMesh("mainStation", mainStation.first, mainStation.second);
    CreateMesh("woodStation", woodStation.first, woodStation.second);
    CreateMesh("coalStation", coalStation.first, coalStation.second);
    CreateMesh("brickStation", brickStation.first, brickStation.second);
}

void Tema2::AddStations() {
    // Place down the main station
    while (1) {
        unsigned int posX = rand() % 50 + 25;
        unsigned int posZ = rand() % 50 + 25;

        if ((indexMap[posX][posZ - 1] & INDEX_TERRAIN) && (indexMap[posX][posZ] & INDEX_TERRAIN) && (indexMap[posX][posZ + 1] & INDEX_TERRAIN)) {
            float minHeight = min(min(heightMap[posX][posZ - 1], heightMap[posX][posZ]), heightMap[posX][posZ + 1]);

            heightMap[posX][posZ - 1] = minHeight;
            heightMap[posX][posZ] = minHeight;
            heightMap[posX][posZ + 1] = minHeight;

            indexMap[posX][posZ - 1] |= INDEX_STATION;
            indexMap[posX][posZ] |= INDEX_STATION;
            indexMap[posX][posZ + 1] |= INDEX_STATION;
            stations.push_back({ posX, posZ });
            break;
        }
    }

    for (int i = 1; i < 7; i++) {
        while (1) {
            unsigned int posX = rand() % (MAP_SIZE - 10) + 5;
            unsigned int posZ = rand() % (MAP_SIZE - 10) + 5;

            bool bad_placement = 0;
            for (int i = 0; i < stations.size(); i++) {
                if (posX == stations[i].first || posX + 1 == stations[i].first || posX - 1 == stations[i].first
                    || posZ == stations[i].second || posZ + 1 == stations[i].second || posZ - 1 == stations[i].second) {
                    bad_placement = 1;
                }
            }
            if (bad_placement) continue;

            // Coal stations
            if (i % 3 == RESOURCE_COAL) {
                if (indexMap[posX][posZ] & INDEX_TERRAIN) {

                    indexMap[posX][posZ] |= INDEX_STATION;
                    stations.push_back({ posX, posZ });
                    break;
                }
            }
            // Brick stations
            else if (i % 3 == RESOURCE_BRICK) {
                if ((indexMap[posX][posZ - 1] & INDEX_TERRAIN) && (indexMap[posX][posZ] & INDEX_TERRAIN) && (indexMap[posX][posZ + 1] & INDEX_TERRAIN)) {
                    float minHeight = min(min(heightMap[posX][posZ - 1], heightMap[posX][posZ]), heightMap[posX][posZ + 1]);

                    heightMap[posX][posZ - 1] = minHeight;
                    heightMap[posX][posZ] = minHeight;
                    heightMap[posX][posZ + 1] = minHeight;

                    indexMap[posX][posZ - 1] |= INDEX_STATION;
                    indexMap[posX][posZ] |= INDEX_STATION;
                    indexMap[posX][posZ + 1] |= INDEX_STATION;
                    stations.push_back({ posX, posZ });
                    break;
                }
            }
            // Wood stations
            else {
                if (indexMap[posX][posZ] & INDEX_TERRAIN) {

                    indexMap[posX][posZ] |= INDEX_STATION;
                    stations.push_back({ posX, posZ });
                    break;
                }
            }
        }
    }
}

void Tema2::ConnectWithRail(int pt1X, int pt1Z, int pt2X, int pt2Z) {
    int direction = rand() % 2;
    if (direction) {
        indexMap[pt1X][pt1Z] |= INDEX_RAIL;
        while (pt1X != pt2X) {
            if (pt1X < pt2X) {
                pt1X++;
            }
            else {
                pt1X--;
            }
            indexMap[pt1X][pt1Z] |= INDEX_RAIL;
        }
        while (pt1Z != pt2Z) {
            if (pt1Z < pt2Z) {
                pt1Z++;
            }
            else {
                pt1Z--;
            }
            indexMap[pt1X][pt1Z] |= INDEX_RAIL;
        }
    }
    else {
        indexMap[pt1X][pt1Z] |= INDEX_RAIL;
        while (pt1Z != pt2Z) {
            if (pt1Z < pt2Z) {
                pt1Z++;
            }
            else {
                pt1Z--;
            }
            indexMap[pt1X][pt1Z] |= INDEX_RAIL;
        }
        while (pt1X != pt2X) {
            if (pt1X < pt2X) {
                pt1X++;
            }
            else {
                pt1X--;
            }
            indexMap[pt1X][pt1Z] |= INDEX_RAIL;
        }
    }
}

void Tema2::AddRails() {
    // Pick the position of the intersections randomly (won't make the most sense, but whatever ...)
    for (int i = 0; i < 3; i++) {
        while (1) {
            unsigned int posX = rand() % 60 + 20;
            unsigned int posZ = rand() % 60 + 20;

            for (int i = 0; i < intersections.size(); i++) {
                if (posX == intersections[i].first || posX + 1 == intersections[i].first || posX - 1 == intersections[i].first
                    || posZ == intersections[i].second || posZ + 1 == intersections[i].second || posZ - 1 == intersections[i].second) {
                    continue;
                }
            }

            bool bad_placement = 0;
            for (int i = 0; i < stations.size(); i++) {
                if (posX == stations[i].first || posX + 1 == stations[i].first || posX - 1 == stations[i].first
                    || posZ == stations[i].second || posZ + 1 == stations[i].second || posZ - 1 == stations[i].second) {
                    bad_placement = 1;
                }
            }
            if (bad_placement) {
                continue;
            }
            intersections.push_back({ posX, posZ });
            break;
        }
    }

    // Make the rail from main station to first intersection
    int int1X = intersections[0].first;
    int int1Z = intersections[0].second;
    int mainX = stations[0].first;
    int mainZ = stations[0].second;

    if (int1X > mainX) {
        mainX++;
    }
    else {
        mainX--;
    }
    ConnectWithRail(int1X, int1Z, mainX, mainZ);

    // Make rails from the first 3 resource stations to the 2nd intersection
    for (int i = 1; i <= 3; i++) {
        int int2X = intersections[1].first;
        int int2Z = intersections[1].second;
        int statX = stations[i].first;
        int statZ = stations[i].second;

        if (int2X > statX) {
            statX++;
        }
        else {
            statX--;
        }
        ConnectWithRail(int2X, int2Z, statX, statZ);
    }

    // Make rails from the last 3 resource stations to the 3nd intersection
    for (int i = 4; i <= 6; i++) {
        int int3X = intersections[2].first;
        int int3Z = intersections[2].second;
        int statX = stations[i].first;
        int statZ = stations[i].second;

        if (int3X > statX) {
            statX++;
        }
        else {
            statX--;
        }
        ConnectWithRail(int3X, int3Z, statX, statZ);
    }

    // Connect the intersections
    int int2X = intersections[1].first;
    int int2Z = intersections[1].second;

    if (int1X > int2X) {
        int2X++;
    }
    else {
        int2X--;
    }
    ConnectWithRail(int1X, int1Z, int2X, int2Z);

    int int3X = intersections[2].first;
    int int3Z = intersections[2].second;

    if (int1X > int3X) {
        int3X++;
    }
    else {
        int3X--;
    }
    ConnectWithRail(int1X, int1Z, int3X, int3Z);

    // Make the rails no longer go through the stations, and make the stations be surrounded by a rail
    for (int i = 0; i < stations.size(); i++) {
        if (i == 0) {
            indexMap[stations[i].first][stations[i].second] &= ~(INDEX_RAIL);
            indexMap[stations[i].first][stations[i].second + 1] &= ~(INDEX_RAIL);
            indexMap[stations[i].first][stations[i].second - 1] &= ~(INDEX_RAIL);

            for (int k1 = -2; k1 <= 2; k1++) {
                for (int k2 = -1; k2 <= 1; k2++) {
                    if (k2 == 0 && k1 < 2 && k1 > -2) continue;
                    indexMap[stations[i].first + k2][stations[i].second + k1] |= (INDEX_RAIL);
                }
            }
        }
        else if (i % 3 == RESOURCE_COAL) {
            indexMap[stations[i].first][stations[i].second] &= ~(INDEX_RAIL);
            for (int k1 = -1; k1 <= 1; k1++) {
                for (int k2 = -1; k2 <= 1; k2++) {
                    if (k2 == 0 && k1 == 0) continue;
                    indexMap[stations[i].first + k2][stations[i].second + k1] |= (INDEX_RAIL);
                }
            }
        }
        else if (i % 3 == RESOURCE_BRICK) {
            indexMap[stations[i].first][stations[i].second] &= ~(INDEX_RAIL);
            indexMap[stations[i].first][stations[i].second + 1] &= ~(INDEX_RAIL);
            indexMap[stations[i].first][stations[i].second - 1] &= ~(INDEX_RAIL);

            for (int k1 = -2; k1 <= 2; k1++) {
                for (int k2 = -1; k2 <= 1; k2++) {
                    if (k2 == 0 && k1 < 2 && k1 > -2) continue;
                    indexMap[stations[i].first + k2][stations[i].second + k1] |= (INDEX_RAIL);
                }
            }
        }
        else {
            indexMap[stations[i].first][stations[i].second] &= ~(INDEX_RAIL);
            for (int k1 = -1; k1 <= 1; k1++) {
                for (int k2 = -1; k2 <= 1; k2++) {
                    if (k2 == 0 && k1 == 0) continue;
                    indexMap[stations[i].first + k2][stations[i].second + k1] |= (INDEX_RAIL);
                }
            }
        }
    }
}

void Tema2::MakeRailway() {
    pair<vector<VertexFormat>, vector<unsigned int>> railway = { vector<VertexFormat>(), vector<unsigned int>() };
    pair<vector<VertexFormat>, vector<unsigned int>> terrain_rail = { meshes["terrain_rail"]->vertices, meshes["terrain_rail"]->indices };
    pair<vector<VertexFormat>, vector<unsigned int>> bridge_rail = { meshes["bridge_rail"]->vertices, meshes["bridge_rail"]->indices };
    pair<vector<VertexFormat>, vector<unsigned int>> tunnel_rail = { meshes["tunnel_rail"]->vertices, meshes["tunnel_rail"]->indices };
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            if (indexMap[i][j] & INDEX_RAIL) {
                float offsetX = (i - MAP_CENTER);
                float offsetZ = (j - MAP_CENTER);
                glm::mat4 railPos = glm::mat4(1);

                if (indexMap[i][j] & INDEX_TERRAIN) {
                    if (indexMap[i - 1][j] & INDEX_RAIL) {
                        float nextHeight = heightMap[i - 1][j];
                        if (indexMap[i - 1][j] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i - 1][j] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - heightMap[i][j]);
                        float len = 0.5;
                        if (nextHeight > heightMap[i][j]) {
                            railPos = transform::Translate(offsetX - 0.25, (heightMap[i][j] + nextHeight) / 2 + 0.01f, offsetZ)
                                * transform::RotateOZ(-glm::atan2(height, len)) * transform::Scale(2 * glm::sqrt(height * height + len * len), 1, 1);
                        }
                        else {
                            railPos = transform::Translate(offsetX - 0.25, heightMap[i][j] + 0.01f, offsetZ);
                        }
                        add_vertices(railway, terrain_rail, railPos);
                    }
                    if (indexMap[i + 1][j] & INDEX_RAIL) {
                        float nextHeight = heightMap[i + 1][j];
                        if (indexMap[i + 1][j] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i + 1][j] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - heightMap[i][j]);
                        float len = 0.5;
                        if (nextHeight > heightMap[i][j]) {
                            railPos = transform::Translate(offsetX + 0.25, (heightMap[i][j] + nextHeight) / 2 + 0.01f, offsetZ)
                                * transform::RotateOZ(glm::atan2(height, len)) * transform::Scale(2 * glm::sqrt(height * height + len * len), 1, 1);
                        }
                        else {
                            railPos = transform::Translate(offsetX + 0.25, heightMap[i][j] + 0.01f, offsetZ);
                        }
                        add_vertices(railway, terrain_rail, railPos);
                    }

                    if (indexMap[i][j - 1] & INDEX_RAIL) {
                        float nextHeight = heightMap[i][j - 1];
                        if (indexMap[i][j - 1] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i][j - 1] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - heightMap[i][j]);
                        float len = 0.5;
                        if (nextHeight > heightMap[i][j]) {
                            railPos = transform::Translate(offsetX, (heightMap[i][j] + nextHeight) / 2 + 0.01f, offsetZ - 0.25)
                                * transform::RotateOX(-glm::atan2(height, len)) * transform::Scale(1, 1, 2 * glm::sqrt(height * height + len * len));
                        }
                        else {
                            railPos = transform::Translate(offsetX, heightMap[i][j] + 0.01f, offsetZ - 0.25);
                        }
                        add_vertices(railway, terrain_rail, railPos);
                    }
                    if (indexMap[i][j + 1] & INDEX_RAIL) {
                        float nextHeight = heightMap[i][j + 1];
                        if (indexMap[i][j + 1] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i][j + 1] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - heightMap[i][j]);
                        float len = 0.5;
                        if (nextHeight > heightMap[i][j]) {
                            railPos = transform::Translate(offsetX, (heightMap[i][j] + nextHeight) / 2 + 0.01f, offsetZ + 0.25)
                                * transform::RotateOX(glm::atan2(height, len)) * transform::Scale(1, 1, 2 * glm::sqrt(height * height + len * len));
                        }
                        else {
                            railPos = transform::Translate(offsetX, heightMap[i][j] + 0.01f, offsetZ + 0.25);
                        }
                        add_vertices(railway, terrain_rail, railPos);
                    }
                }
                if (indexMap[i][j] & INDEX_RIVER) {
                    if (indexMap[i - 1][j] & INDEX_RAIL) {
                        float nextHeight = heightMap[i - 1][j];
                        float current_heght = -1.5f;
                        if (indexMap[i - 1][j] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i - 1][j] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX - 0.25, (current_heght + nextHeight) / 2 + 0.01f, offsetZ)
                                * transform::RotateOZ(-glm::atan2(height, len)) * transform::Scale(2 * glm::sqrt(height * height + len * len), 1, 1);
                        }
                        else {
                            railPos = transform::Translate(offsetX - 0.25, current_heght + 0.01f, offsetZ);
                        }
                        railPos *= transform::RotateOY(glm::radians(90.0f));
                        add_vertices(railway, bridge_rail, railPos);
                    }
                    if (indexMap[i + 1][j] & INDEX_RAIL) {
                        float nextHeight = heightMap[i + 1][j];
                        float current_heght = -1.5f;
                        if (indexMap[i + 1][j] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i + 1][j] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX + 0.25, (current_heght + nextHeight) / 2 + 0.01f, offsetZ)
                                * transform::RotateOZ(glm::atan2(height, len)) * transform::Scale(2 * glm::sqrt(height * height + len * len), 1, 1);
                        }
                        else {
                            railPos = transform::Translate(offsetX + 0.25, current_heght + 0.01f, offsetZ);
                        }
                        railPos *= transform::RotateOY(glm::radians(90.0f));
                        add_vertices(railway, bridge_rail, railPos);
                    }

                    if (indexMap[i][j - 1] & INDEX_RAIL) {
                        float nextHeight = heightMap[i][j - 1];
                        float current_heght = -1.5f;
                        if (indexMap[i][j - 1] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i][j - 1] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX, (current_heght + nextHeight) / 2 + 0.01f, offsetZ - 0.25)
                                * transform::RotateOX(-glm::atan2(height, len)) * transform::Scale(1, 1, 2 * glm::sqrt(height * height + len * len));
                        }
                        else {
                            railPos = transform::Translate(offsetX, current_heght + 0.01f, offsetZ - 0.25);
                        }
                        add_vertices(railway, bridge_rail, railPos);
                    }
                    if (indexMap[i][j + 1] & INDEX_RAIL) {
                        float current_heght = -1.5f;
                        float nextHeight = heightMap[i][j + 1];
                        if (indexMap[i][j + 1] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i][j + 1] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX, (current_heght + nextHeight) / 2 + 0.01f, offsetZ + 0.25)
                                * transform::RotateOX(glm::atan2(height, len)) * transform::Scale(1, 1, 2 * glm::sqrt(height * height + len * len));
                        }
                        else {
                            railPos = transform::Translate(offsetX, current_heght + 0.01f, offsetZ + 0.25);
                        }
                        add_vertices(railway, bridge_rail, railPos);
                    }
                }
                if (indexMap[i][j] & INDEX_MOUNTAIN) {
                    if (indexMap[i - 1][j] & INDEX_RAIL) {
                        float nextHeight = heightMap[i - 1][j];
                        float current_heght = 1.5f;
                        if (indexMap[i - 1][j] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i - 1][j] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX - 0.25, (current_heght + nextHeight) / 2 + 0.01f, offsetZ)
                                * transform::RotateOZ(-glm::atan2(height, len)) * transform::Scale(2 * glm::sqrt(height * height + len * len), 1, 1);
                        }
                        else {
                            railPos = transform::Translate(offsetX - 0.25, current_heght + 0.01f, offsetZ);
                        }
                        railPos *= transform::RotateOY(glm::radians(90.0f));
                        add_vertices(railway, tunnel_rail, railPos);
                    }
                    if (indexMap[i + 1][j] & INDEX_RAIL) {
                        float nextHeight = heightMap[i + 1][j];
                        float current_heght = 1.5f;
                        if (indexMap[i + 1][j] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i + 1][j] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX + 0.25, (current_heght + nextHeight) / 2 + 0.01f, offsetZ)
                                * transform::RotateOZ(glm::atan2(height, len)) * transform::Scale(2 * glm::sqrt(height * height + len * len), 1, 1);
                        }
                        else {
                            railPos = transform::Translate(offsetX + 0.25, current_heght + 0.01f, offsetZ);
                        }
                        railPos *= transform::RotateOY(glm::radians(90.0f));
                        add_vertices(railway, tunnel_rail, railPos);
                    }

                    if (indexMap[i][j - 1] & INDEX_RAIL) {
                        float nextHeight = heightMap[i][j - 1];
                        float current_heght = 1.5f;
                        if (indexMap[i][j - 1] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i][j - 1] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX, (current_heght + nextHeight) / 2 + 0.01f, offsetZ - 0.25)
                                * transform::RotateOX(-glm::atan2(height, len)) * transform::Scale(1, 1, 2 * glm::sqrt(height * height + len * len));
                        }
                        else {
                            railPos = transform::Translate(offsetX, current_heght + 0.01f, offsetZ - 0.25);
                        }
                        add_vertices(railway, tunnel_rail, railPos);
                    }
                    if (indexMap[i][j + 1] & INDEX_RAIL) {
                        float current_heght = 1.5f;
                        float nextHeight = heightMap[i][j + 1];
                        if (indexMap[i][j + 1] & (INDEX_RIVER)) {
                            nextHeight = -1.5f;
                        }
                        if (indexMap[i][j + 1] & INDEX_MOUNTAIN) {
                            nextHeight = 1.5f;
                        }
                        float height = (nextHeight - current_heght);
                        float len = 0.5;
                        if (nextHeight > current_heght) {
                            railPos = transform::Translate(offsetX, (current_heght + nextHeight) / 2 + 0.01f, offsetZ + 0.25)
                                * transform::RotateOX(glm::atan2(height, len)) * transform::Scale(1, 1, 2 * glm::sqrt(height * height + len * len));
                        }
                        else {
                            railPos = transform::Translate(offsetX, current_heght + 0.01f, offsetZ + 0.25);
                        }
                        add_vertices(railway, tunnel_rail, railPos);
                    }
                }
            }
        }
    }

    CreateMesh("railway", railway.first, railway.second);
}

void Tema2::AddLogicRailway() {
    for (int i = 1; i < MAP_SIZE - 1; i++) {
        for (int j = 1; j < MAP_SIZE - 1; j++) {
            float offsetX = (i - MAP_CENTER);
            float offsetZ = (j - MAP_CENTER);

            float startHeight = heightMap[i][j];
            if (indexMap[i][j] & INDEX_MOUNTAIN) {
                startHeight = 1.5f;
            }
            else if (indexMap[i][j] & INDEX_RIVER) {
                startHeight = -1.5f;
            }

            float endHeight = heightMap[i][j + 1];
            if (indexMap[i][j + 1] & INDEX_MOUNTAIN) {
                endHeight = 1.5f;
            }
            else if (indexMap[i][j + 1] & INDEX_RIVER) {
                endHeight = -1.5f;
            }

            if ((indexMap[i][j] & INDEX_RAIL) && (indexMap[i][j + 1] & INDEX_RAIL)) {
                railwayZ[i][j] = new LogicRail(DIRECTION_Z, offsetZ, offsetZ + 1, offsetX, startHeight, endHeight);
            }

            endHeight = heightMap[i + 1][j];
            if (indexMap[i + 1][j] & INDEX_MOUNTAIN) {
                endHeight = 1.5f;
            }
            else if (indexMap[i + 1][j] & INDEX_RIVER) {
                endHeight = -1.5f;
            }

            if ((indexMap[i][j] & INDEX_RAIL) && (indexMap[i + 1][j] & INDEX_RAIL)) {
                railwayX[i][j] = new LogicRail(DIRECTION_X, offsetX, offsetX + 1, offsetZ, startHeight, endHeight);
            }

            if ((indexMap[i][j - 1] & INDEX_RAIL) && (indexMap[i][j] & INDEX_RAIL) && (indexMap[i][j + 1] & INDEX_RAIL)) {
                railwayZ[i][j - 1]->railZplusXplus = railwayZ[i][j];
                railwayZ[i][j]->railZminusXplus = railwayZ[i][j - 1];
            }

            if ((indexMap[i][j + 1] & INDEX_RAIL) && (indexMap[i][j] & INDEX_RAIL) && (indexMap[i - 1][j] & INDEX_RAIL)) {
                railwayZ[i][j]->railXminusZminus = railwayX[i - 1][j];
                railwayX[i - 1][j]->railZplusXplus = railwayZ[i][j];
            }

            if ((indexMap[i][j + 1] & INDEX_RAIL) && (indexMap[i][j] & INDEX_RAIL) && (indexMap[i - 1][j + 1] & INDEX_RAIL)) {
                railwayZ[i][j]->railXminusZplus = railwayX[i - 1][j + 1];
                railwayX[i - 1][j + 1]->railZminusXplus = railwayZ[i][j];
            }
            
            if ((indexMap[i - 1][j] & INDEX_RAIL) && (indexMap[i][j] & INDEX_RAIL) && (indexMap[i + 1][j] & INDEX_RAIL)) {
                railwayX[i - 1][j]->railXplusZplus = railwayX[i][j];
                railwayX[i][j]->railXminusZplus = railwayX[i - 1][j];
            }

            if ((indexMap[i + 1][j] & INDEX_RAIL) && (indexMap[i][j] & INDEX_RAIL)  && (indexMap[i][j - 1] & INDEX_RAIL)) {
                railwayZ[i][j - 1]->railXplusZplus = railwayX[i][j];
                railwayX[i][j]->railZminusXminus = railwayZ[i][j - 1];
            }

            if ((indexMap[i + 1][j] & INDEX_RAIL) && (indexMap[i][j] & INDEX_RAIL) && (indexMap[i][j + 1] & INDEX_RAIL)) {
                railwayZ[i][j]->railXplusZminus = railwayX[i][j];
                railwayX[i][j]->railZplusXminus = railwayZ[i][j];
            }
        }
    }
}

void Tema2::AddTrain() {
    int posX = stations[0].first;
    int posZ = stations[0].second;
    train = LogicTrain(railwayZ[posX + 1][posZ + 1], railwayZ[posX + 1][posZ], railwayZ[posX + 1][posZ], railwayZ[posX + 1][posZ - 1], railwayZ[posX + 1][posZ - 1]);
    initialX = (int)(posX + 1) - MAP_CENTER;
    initialZ = (int)(posZ + 1) - MAP_CENTER;
    initialY = heightMap[posX + 1][posZ + 1];
}

float Tema2::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void Tema2::UpdateTrain(float deltaTimeSeconds) {
    if (lastDirectionPressed & PRESSED_S) {
        return;
    }

    // Logic for moving the camera to follow the train
    if (train.locMoveProg < 1.0) {
        train.locMoveProg += train.speed * deltaTimeSeconds;
        if (train.locMoveProg > 1.0) {
            float prevMove = 3 * (train.speed * deltaTimeSeconds - (train.locMoveProg - 1));
            float newMove = 3 * (train.locMoveProg - 1);

            if (train.wag2Rail->orientation == DIRECTION_X) {
                if (train.wag2Sense == SENSE_PLUS) {
                    camera->position.x += prevMove;
                }
                else {
                    camera->position.x -= prevMove;
                }
            }
            else {
                if (train.wag2Sense == SENSE_PLUS) {
                    camera->position.z += prevMove;
                }
                else {
                    camera->position.z -= prevMove;
                }
            }
        }
        else {
            float move = 3 * train.speed * deltaTimeSeconds;;
            if (train.locomotiveRail->orientation == DIRECTION_X) {
                if (train.locSense == SENSE_PLUS) {
                    camera->position.x += move;
                }
                else {
                    camera->position.x -= move;
                }
            }
            else {
                if (train.locSense == SENSE_PLUS) {
                    camera->position.z += move;
                }
                else {
                    camera->position.z -= move;
                }
            }
        }
    }
    else {

        // Logic for choosing the next rail for the locomotive. The wagons just get to be on the rail the previos car was on
        LogicRail* currentRail = train.locomotiveRail;
        bool currentSense = train.locSense;
        if (currentRail->orientation == DIRECTION_X && train.locSense == SENSE_MINUS) {
            if (lastDirectionPressed & PRESSED_A && currentRail->railZminusXminus) {
                train.locomotiveRail = currentRail->railZminusXminus;
            }
            else if (lastDirectionPressed & PRESSED_D && currentRail->railZplusXminus) {
                train.locSense = SENSE_PLUS;
                train.locomotiveRail = currentRail->railZplusXminus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railXminusZplus) {
                train.locomotiveRail = currentRail->railXminusZplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railZminusXminus && !currentRail->railZplusXminus) {
                train.locomotiveRail = currentRail->railZminusXminus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railZplusXminus && !currentRail->railZminusXminus) {
                train.locSense = SENSE_PLUS;
                train.locomotiveRail = currentRail->railZplusXminus;
            }
            else {
                return;
            }
        }
        else if (currentRail->orientation == DIRECTION_X && train.locSense == SENSE_PLUS) {
            if (lastDirectionPressed & PRESSED_A && currentRail->railZplusXplus) {
                train.locomotiveRail = currentRail->railZplusXplus;
            }
            else if (lastDirectionPressed & PRESSED_D && currentRail->railZminusXplus) {
                train.locSense = SENSE_MINUS;
                train.locomotiveRail = currentRail->railZminusXplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railXplusZplus) {
                train.locomotiveRail = currentRail->railXplusZplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railZminusXplus && !currentRail->railZplusXplus) {
                train.locSense = SENSE_MINUS;
                train.locomotiveRail = currentRail->railZminusXplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railZplusXplus && !currentRail->railZminusXplus) {
                train.locomotiveRail = currentRail->railZplusXplus;
            }
            else {
                return;
            }
        }
        else if (currentRail->orientation == DIRECTION_Z && train.locSense == SENSE_MINUS) {
            if (lastDirectionPressed & PRESSED_A && currentRail->railXplusZminus) {
                train.locSense = SENSE_PLUS;
                train.locomotiveRail = currentRail->railXplusZminus;
            }
            else if (lastDirectionPressed & PRESSED_D && currentRail->railXminusZminus) {
                train.locomotiveRail = currentRail->railXminusZminus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railZminusXplus) {
                train.locomotiveRail = currentRail->railZminusXplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railXplusZminus && !currentRail->railXminusZminus) {
                train.locSense = SENSE_PLUS;
                train.locomotiveRail = currentRail->railXplusZminus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railXminusZminus && !currentRail->railXplusZminus) {
                train.locomotiveRail = currentRail->railXminusZminus;
            }
            else {
                return;
            }
        }
        else if (currentRail->orientation & DIRECTION_Z && train.locSense == SENSE_PLUS) {
            if (lastDirectionPressed & PRESSED_A && currentRail->railXminusZplus) {
                train.locSense = SENSE_MINUS;
                train.locomotiveRail = currentRail->railXminusZplus;
            }
            else if (lastDirectionPressed & PRESSED_D && currentRail->railXplusZplus) {
                train.locomotiveRail = currentRail->railXplusZplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railZplusXplus) {
                train.locomotiveRail = currentRail->railZplusXplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railXminusZplus && !currentRail->railXplusZplus) {
                train.locSense = SENSE_MINUS;
                train.locomotiveRail = currentRail->railXminusZplus;
            }
            else if (lastDirectionPressed & PRESSED_W && currentRail->railXplusZplus && !currentRail->railXminusZplus) {
                train.locomotiveRail = currentRail->railXplusZplus;
            }
            else {
                return;
            }
        }

        train.locMoveProg = 0.0f;
        train.wag1MoveProg = 0.5f;
        train.wag2MoveProg = 1.0f;
    }
    if (train.wag1MoveProg < 1.0) {
        train.wag1MoveProg += train.speed * deltaTimeSeconds;
    }
    else {
        train.wag1prevRail = train.wag1Rail;
        train.wag1Rail = train.locomotiveRail;
        train.wag1prevSense = train.wag1Sense;
        train.wag1Sense = train.locSense;
        train.wag1MoveProg -= 1.0f;
    }
    if (train.wag2MoveProg < 1.0) {
        train.wag2MoveProg += train.speed * deltaTimeSeconds;
    }
    else {
        train.prevRail = train.wag2Rail;
        train.wag2Rail = train.wag1Rail;
        train.prevSense = train.wag2Sense;
        train.wag2Sense = train.wag1Sense;
        train.wag2MoveProg -= 1.0f;
    }
}

void Tema2::HandleTrainDeliveries() {
    int trainI = ((train.locomotiveRail->orientation == DIRECTION_X) ? train.locomotiveRail->start : train.locomotiveRail->position) + MAP_CENTER;
    int trainJ = ((train.locomotiveRail->orientation == DIRECTION_X) ? train.locomotiveRail->position : train.locomotiveRail->start) + MAP_CENTER;
    if (train.locomotiveRail->orientation == DIRECTION_X) {
        if (train.locSense == SENSE_MINUS) {
            trainI += 1;
        }
    }
    else {
        if (train.locSense == SENSE_MINUS) {
            trainJ += 1;
        }
    }
    if (glm::abs(trainI - stations[0].first) <= 1 && glm::abs(trainJ - stations[0].second) <= 2) {
        // "Colision" with main station
        while (heldCoal) {
            for (int j = 0; j < requiredResources.size(); j++) {
                if (requiredResources[j] == COAL_REQUIRED) {
                    requiredResources.erase(requiredResources.begin() + j);
                    break;
                }
            }
            heldCoal -= HAVE_COAL;
        }
        while (heldWood) {
            for (int j = 0; j < requiredResources.size(); j++) {
                if (requiredResources[j] == WOOD_REQUIRED) {
                    requiredResources.erase(requiredResources.begin() + j);
                    break;
                }
            }
            heldWood -= HAVE_WOOD;
        }
        while (heldBrick) {
            for (int j = 0; j < requiredResources.size(); j++) {
                if (requiredResources[j] == BRICK_REQUIRED) {
                    requiredResources.erase(requiredResources.begin() + j);
                    break;
                }
            }
            heldBrick -= HAVE_BRICK;
        }
    }
    if (newCommandDelay <= 0 && requiredResources.size() == 0) {
        newCommandDelay = 5.0f;
    }

    
    for (int i = 1; i < 7; i++) {
        if (i % 3 == RESOURCE_COAL) {
            if (glm::abs(trainI - stations[i].first) <= 1 && glm::abs(trainJ - stations[i].second) <= 1 && loadCooldown <= 0) {
                // "Colision" with coal station
                heldCoal += HAVE_COAL;
                loadCooldown = 5.0f;
            }
        }
        else if (i % 3 == RESOURCE_WOOD) {
            if (glm::abs(trainI - stations[i].first) <= 1 && glm::abs(trainJ - stations[i].second) <= 1 && loadCooldown <= 0) {
                // "Colision" with wood station
                heldWood += HAVE_WOOD;
                loadCooldown = 5.0f;
            }
        }
        else {
            if (glm::abs(trainI - stations[i].first) <= 1 && glm::abs(trainJ - stations[i].second) <= 2 && loadCooldown <= 0) {
                // "Colision" with brick station
                heldBrick += HAVE_BRICK;
                loadCooldown = 5.0f;
            }
        }
    }
}

void Tema2::RenderUI() {
    if (gameLost) {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        char buf[6];

        textRenderer->RenderText("Game over", windowWidth * 0.25, windowHeight * 0.3, 1 * max(widthScale, heightScale));
        textRenderer->RenderText("Score: ", windowWidth * 0.3, windowHeight * 0.5, 0.7 * max(widthScale, heightScale));
        textRenderer->RenderText(string(itoa(score, buf, 10)), windowWidth * 0.7, windowHeight * 0.5, 0.7 * max(widthScale, heightScale));
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glm::mat4 elementPos = glm::mat4(1);
    textRenderer->RenderText("Current Command", windowWidth * 0.025, windowHeight * 0.05, 0.2 * max(widthScale, heightScale));

    float baseX = -0.5;

    for (int i = 0; i < requiredResources.size(); i++) {
        if (requiredResources[i] == COAL_REQUIRED) {
            elementPos = transform::Translate(baseX, 0.86, -1) * transform::Scale(0.06, 0.11, 0.1);
            RenderSimpleMesh(meshes["coal"], shaders["UIShader"], elementPos);
        }
        if (requiredResources[i] == WOOD_REQUIRED) {
            elementPos = transform::Translate(baseX, 0.84, -1) * transform::Scale(0.06, 0.1, 0.1) * transform::RotateOX(glm::radians(90.0f)) * transform::RotateOY(glm::radians(75.0f));
            RenderSimpleMesh(meshes["wood"], shaders["UIShader"], elementPos);
        }
        if (requiredResources[i] == BRICK_REQUIRED) {
            elementPos = transform::Translate(baseX, 0.86, -1) * transform::Scale(0.06, 0.11, 0.1);
            RenderSimpleMesh(meshes["brick"], shaders["UIShader"], elementPos);
        }
        baseX += 0.135;
    }
    
    textRenderer->RenderText("Collected", windowWidth * 0.025, windowHeight * 0.15, 0.2 * max(widthScale, heightScale));

    baseX = -0.5;
    int tempCoal = heldCoal;
    int tempWood = heldWood;
    int tempBrick = heldBrick;
    while (tempCoal) {
        tempCoal -= HAVE_COAL;
        elementPos = transform::Translate(baseX, 0.66, -1) * transform::Scale(0.06, 0.11, 0.1);
        RenderSimpleMesh(meshes["coal"], shaders["UIShader"], elementPos);
        baseX += 0.135;
    }
    while (tempWood) {
        tempWood -= HAVE_WOOD;
        elementPos = transform::Translate(baseX, 0.64, -1) * transform::Scale(0.06, 0.1, 0.1) * transform::RotateOX(glm::radians(90.0f)) * transform::RotateOY(glm::radians(75.0f));
        RenderSimpleMesh(meshes["wood"], shaders["UIShader"], elementPos);
        baseX += 0.135;
    }
    while (tempBrick) {
        tempBrick -= HAVE_BRICK;
        elementPos = transform::Translate(baseX, 0.66, -1) * transform::Scale(0.06, 0.11, 0.1);
        RenderSimpleMesh(meshes["brick"], shaders["UIShader"], elementPos);
        baseX += 0.135;
    }

    char buf[8];
    if (newCommandDelay > 0) {
        sprintf(buf, "%.2f", newCommandDelay);
        textRenderer->RenderText("Until new command", windowWidth * 0.725, windowHeight * 0.95, 0.15 * max(widthScale, heightScale));
    }
    else {
        sprintf(buf, "%.2f", remainingTime);
        textRenderer->RenderText("Remaining time", windowWidth * 0.725, windowHeight * 0.95, 0.15 * max(widthScale, heightScale));
    }
    
    textRenderer->RenderText(string(buf), windowWidth * 0.925, windowHeight * 0.95, 0.15 * max(widthScale, heightScale));

    float offsetX = -((int)stations[0].first - MAP_CENTER);
    float offsetY = (int)stations[0].second - MAP_CENTER;
    glm::vec3 greenStation = glm::vec3(0.341, 0.929, 0.325);
    glm::vec3 redStation = glm::vec3(0.89, 0.145, 0.145);

    elementPos = transform::Translate(-0.72, -0.58, -0.02) * transform::Scale(0.005, 0.008, 1);
    RenderSimpleMesh(meshes["minimap_base"], shaders["UIShader"], elementPos);

    elementPos = transform::Translate(offsetX * 0.005 - 0.72, offsetY * 0.008 - 0.58, -0.01) * transform::Scale(0.005, 0.008, 1);
    RenderChangingMesh(meshes["minimap_main_station"], shaders["UIMinimapShader"], elementPos, redStation, greenStation, remainingTime / 90.0f);

    float X;
    float Y;
    float rotAngle;
    if (train.locomotiveRail->orientation == DIRECTION_Z) {
        X = -train.locomotiveRail->position;
        Y = train.locSense == SENSE_PLUS ? lerp(train.locomotiveRail->start, train.locomotiveRail->end, train.locMoveProg) :
            lerp(train.locomotiveRail->end, train.locomotiveRail->start, train.locMoveProg);
        rotAngle = (train.locSense == SENSE_PLUS) ? 0 : 180;
    }
    else {
        Y = train.locomotiveRail->position;
        X = train.locSense == SENSE_PLUS ? -lerp(train.locomotiveRail->start, train.locomotiveRail->end, train.locMoveProg) :
            -lerp(train.locomotiveRail->end, train.locomotiveRail->start, train.locMoveProg);
        rotAngle = (train.locSense == SENSE_PLUS) ? 90 : 270;
    }

    elementPos = transform::Translate(X * 0.005 - 0.72, Y * 0.008 - 0.58, -0.1) * transform::Scale(0.008, 0.011, 1) * transform::RotateOZ(glm::radians(rotAngle));
    RenderMesh(meshes["minimap_locomotive"], shaders["UIShader"], elementPos);

    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Tema2::MakeMinimapBase() {
    pair<vector<VertexFormat>, vector<unsigned int>> minimap;

    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            float offsetX = -(i - MAP_CENTER);
            float offsetY = (j - MAP_CENTER);

            glm::vec3 color = glm::vec3(1);
            if (indexMap[i][j] & INDEX_RAIL) {
                color = glm::vec3(0.149, 0.149, 0.114);
            }
            else if (indexMap[i][j] & INDEX_TERRAIN) {
                color = glm::vec3(0, 0.7, 0.18);
            }
            else if (indexMap[i][j] & INDEX_RIVER) {
                color = glm::vec3(0.341, 0.686, 0.878);
            }
            else if (indexMap[i][j] & INDEX_MOUNTAIN) {
                color = glm::vec3(0.361, 0.165, 0.055);
            }

            glm::mat4 newPos = transform::Translate(offsetX, offsetY, 0);
            add_vertices(minimap, create_square_vertices(1, 1, color), newPos);
        }
    }

    for (int i = 1; i < 7; i++) {
        int posX = stations[i].first;
        int posY = stations[i].second;
        float offsetX = -(posX - MAP_CENTER);
        float offsetY = (posY - MAP_CENTER);
        glm::vec3 color = glm::vec3(1);

        if (i % 3 == RESOURCE_COAL) {
            color = glm::vec3(0, 0, 0);
            glm::mat4 newPos = glm::mat4(1);
            newPos = transform::Translate(offsetX, offsetY, 0.01f);
            add_vertices(minimap, create_square_vertices(1, 1, color), newPos);
        }
        else if (i % 3 == RESOURCE_WOOD) {
            color = glm::vec3(0.702, 0.353, 0);
            glm::mat4 newPos = glm::mat4(1);
            newPos = transform::Translate(offsetX, offsetY, 0.01f);
            add_vertices(minimap, create_square_vertices(1, 1, color), newPos);
        }
        else {
            color = glm::vec3(0.922, 0.525, 0.525);
            glm::mat4 newPos = glm::mat4(1);
            newPos = transform::Translate(offsetX, offsetY, 0.01f);
            add_vertices(minimap, create_square_vertices(1, 1, color), newPos);
            newPos = transform::Translate(offsetX, offsetY + 1, 0.01f);
            add_vertices(minimap, create_square_vertices(1, 1, color), newPos);
            newPos = transform::Translate(offsetX, offsetY - 1, 0.01f);
            add_vertices(minimap, create_square_vertices(1, 1, color), newPos);
        }
    }

    CreateMesh("minimap_base", minimap.first, minimap.second);
}


void Tema2::RenderChangingMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::vec3 color1, glm::vec3 color2, float t) {
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Get shader location for uniform mat4 "Model"
    int modelUniformLoc = glGetUniformLocation(shader->GetProgramID(), "Model");

    // Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(modelUniformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Get shader location for uniform mat4 "View"
    int viewUniformLoc = glGetUniformLocation(shader->GetProgramID(), "View");

    // Set shader uniform "View" to viewMatrix
    glm::mat4 viewMatrix = camera->GetViewMatrix();

    // Get shader location for uniform mat4 "Projection"
    glUniformMatrix4fv(viewUniformLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Set shader uniform "Projection" to projectionMatrix
    int projUniformLoc = glGetUniformLocation(shader->GetProgramID(), "Projection");
    glUniformMatrix4fv(projUniformLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    //Set the light position uniform
    int lightPosLoc = glGetUniformLocation(shader->GetProgramID(), "lightPos");
    glm::vec3 lightPos = glm::vec3(0, 125, 0);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    int colorLoc = glGetUniformLocation(shader->GetProgramID(), "color");
    glm::vec3 color = glm::vec3(
        lerp(color1.r, color2.r, t),
        lerp(color1.g, color2.g, t),
        lerp(color1.b, color2.b, t)
    );
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}
