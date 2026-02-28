#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>
#include <queue>

#include "core/engine.h"
#include "utils/gl_utils.h"
#include "transform2D.h"

constexpr auto SHIP_BLOCK = 1;
constexpr auto THRUSTER = 2;
constexpr auto BUMPER = 3;
constexpr auto GUN = 4;
constexpr auto NO_OBJECT_HELD = 0;
constexpr auto BLUE_SQUARE_BLOCK = 0;
constexpr auto NO_BLOCK = 9;

constexpr auto GRID_WIDTH = 17;
constexpr auto GRID_HEIGHT = 9;

using namespace std;
using namespace m1;

Tema1_editor::Tema1_editor()
{
}


Tema1_editor::~Tema1_editor()
{
}

// Scene initialization and scene update
void Tema1_editor::Init() {

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

    // How many more pieces can the player place on the grid, and the grid itself
    remaining_pieces = 10;
    grid = vector<vector<unsigned int>>(GRID_HEIGHT, vector<unsigned int>(GRID_WIDTH, BLUE_SQUARE_BLOCK));

    // Is the player holding any object with their mouse - 0 - none, 1 - 
    // ship hull block, 2 - thruster, 3 - bumper, 4 - gun,
    held_object = NO_OBJECT_HELD;
    
    // Create simple object meshes and vertices / indices
    auto semi_circle = create_endpoints_circle(glm::vec3(0.722, 0.784, 0.8), false);
    CreateMesh("semi", semi_circle.first, semi_circle.second);

    auto circle = create_endpoints_circle(glm::vec3(0.722, 0.784, 0.8), true);
    CreateMesh("circle", circle.first, circle.second);

    auto square_blue = create_endpoints_square(glm::vec3(0, 0.5f, 1), glm::vec3(0, 0.5f, 1), 1, 1);
    CreateMesh("square_blue", square_blue.first, square_blue.second);

    auto square_red = create_endpoints_square(glm::vec3(1, 0, 0), glm::vec3(0.7, 0, 0), 1, 1);
    CreateMesh("square_red", square_red.first, square_red.second);

    auto square_gray = create_endpoints_square(glm::vec3(0.804, 0.871, 0.89), glm::vec3(0.6, 0.671, 0.7), 1, 1);
    CreateMesh("square_gray", square_gray.first, square_blue.second);

    auto square_green = create_endpoints_square(glm::vec3(0, 1, 0), glm::vec3(0, 0.7, 0), 1, 1);
    CreateMesh("square_green", square_green.first, square_green.second);

    auto thruster_base = create_endpoints_square(glm::vec3(0.98, 0.596, 0.031), glm::vec3(0.78, 0.553, 0), 1, 1);
    CreateMesh("thruster_base", thruster_base.first, thruster_base.second);

    auto gun_barrel = create_endpoints_square(glm::vec3(0.169, 0.169, 0.145), glm::vec3(0.169, 0.169, 0.145), 1, 2.2);
    CreateMesh("gun_barrel", gun_barrel.first, gun_barrel.second);

    auto fire_trail = create_endpoints_fire(glm::vec3(0.98, 0.337, 0.031), glm::vec3(0.71, 0.094, 0.004));
    CreateMesh("fire_trail", fire_trail.first, fire_trail.second);

    auto k_green = create_endpoints_K(glm::vec3(0, 1, 0));
    CreateMesh("k_green", k_green.first, k_green.second);

    auto k_red = create_endpoints_K(glm::vec3(1, 0, 0));
    CreateMesh("k_red", k_red.first, k_red.second);

    auto ship_block = create_endpoints_square(glm::vec3(0.675, 0.71, 0.78), glm::vec3(0.475, 0.6, 0.58), 1, 1);
    CreateMesh("ship_block", ship_block.first, ship_block.second);
    ship_block_vertices = ship_block.first;
    ship_block_indices = ship_block.second;

    // Create more complex meshes - thruster, bumper and gun

    for (int i = 0; i < thruster_base.first.size(); i++) {
        thruster_vertices.push_back(thruster_base.first[i]);
    }
    int current_vertices = thruster_vertices.size();
    for (int i = 0; i < fire_trail.first.size(); i++) {
        thruster_vertices.push_back(fire_trail.first[i]);
    }

    for (int i = 0; i < thruster_base.second.size(); i++) {
        thruster_indices.push_back(thruster_base.second[i]);
    }
    for (int i = 0; i < fire_trail.second.size(); i++) {
        thruster_indices.push_back(fire_trail.second[i] + current_vertices);
    }
    CreateMesh("thruster", thruster_vertices, thruster_indices);

    for (int i = 0; i < semi_circle.first.size(); i++) {
        bumper_vertices.push_back(semi_circle.first[i]);
        bumper_vertices[i].position.x *= 1.5;
    }
    current_vertices = bumper_vertices.size();
    for (int i = 0; i < square_gray.first.size(); i++) {
        bumper_vertices.push_back(square_gray.first[i]);
        bumper_vertices[i + current_vertices].position.x -= 0.5f;
        bumper_vertices[i + current_vertices].position.y -= 1;
    }

    for (int i = 0; i < semi_circle.second.size(); i++) {
        bumper_indices.push_back(semi_circle.second[i]);
    }
    for (int i = 0; i < square_gray.second.size(); i++) {
        bumper_indices.push_back(square_gray.second[i] + current_vertices);
    }
    CreateMesh("bumper", bumper_vertices, bumper_indices);

    for (int i = 0; i < semi_circle.first.size(); i++) {
        gun_vertices.push_back(semi_circle.first[i]);
        gun_vertices[i].position *= 0.5;
        gun_vertices[i].position.z = 0.001;
    }
    current_vertices = semi_circle.first.size();
    for (int i = 0; i < square_gray.first.size(); i++) {
        gun_vertices.push_back(square_gray.first[i]);
        gun_vertices[i + current_vertices].position.x -= 0.5;
        gun_vertices[i + current_vertices].position.y -= 0.8;
    }
    for (int i = 0; i < semi_circle.second.size(); i++) {
        gun_indices.push_back(semi_circle.second[i]);
    }
    for (int i = 0; i < square_gray.second.size(); i++) {
        gun_indices.push_back(square_gray.second[i] + current_vertices);
    }
    current_vertices = gun_vertices.size();
    for (int i = 0; i < gun_barrel.first.size(); i++) {
        gun_vertices.push_back(gun_barrel.first[i]);
        gun_vertices[i + current_vertices].position.x *= 0.8;
        gun_vertices[i + current_vertices].position.x -= 0.4;
        gun_vertices[i + current_vertices].position.z = -0.001;
    }
    for (int i = 0; i < gun_barrel.second.size(); i++) {
        gun_indices.push_back(gun_barrel.second[i] + current_vertices);
    }
    CreateMesh("gun", gun_vertices, gun_indices);
}

void Tema1_editor::FrameStart() {
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
    SetViewportArea(viewSpace, glm::vec3(0), true);
}

void Tema1_editor::Update(float deltaTimeSeconds) {
    
    visMatrix = glm::mat3(1);
    visMatrix *= VisualizationTransf2DUnif(logicSpace, viewSpace);
    inverseVis = glm::inverse(visMatrix);

    // Render unique static models

    glm::mat3 bumper_model = glm::mat3(1);
    bumper_model = visMatrix * transform2D::Translate(12, 87.5f) * transform2D::Scale(7.5, 7.5);

    RenderMesh2D(meshes["bumper"], shaders["VertexColor"], bumper_model);

    bool conex = CheckConnectivity();

    glm::mat3 k_model = glm::mat3(1);
    k_model = visMatrix * transform2D::Translate(152, 87.5) * transform2D::Scale(7.5, 7.5);

    if (remaining_pieces >= 0 && remaining_pieces < 10 && conex) {
        RenderMesh2D(meshes["k_green"], shaders["VertexColor"], k_model);
        can_start = true;
    }
    else {
        RenderMesh2D(meshes["k_red"], shaders["VertexColor"], k_model);
        can_start = false;
    }

    glm::mat3 thruster_model = glm::mat3(1);
    thruster_model = visMatrix * transform2D::Translate(8.25, 37.5) * transform2D::Scale(7.5, 7.5);

    RenderMesh2D(meshes["thruster"], shaders["VertexColor"], thruster_model);

    glm::mat3 gun_model = glm::mat3(1);
    gun_model = visMatrix * transform2D::Translate(12, 57.5) * transform2D::Scale(7.5, 7.5);

    RenderMesh2D(meshes["gun"], shaders["VertexColor"], gun_model);

    glm::mat3 ship_block_model = glm::mat3(1);
    ship_block_model = visMatrix * transform2D::Translate(8.25, 8.75) * transform2D::Scale(7.5, 7.5);

    RenderMesh2D(meshes["ship_block"], shaders["VertexColor"], ship_block_model);

    // Render the number of green squares showing how many pieces can still be placed
    // or red squares for number of pieces that must be removed to make the game startable
    glm::mat3 base_rem_square = glm::mat3(1);
    base_rem_square = transform2D::Translate(30, 90) * transform2D::Scale(5, 5);

    if (remaining_pieces >= 0) {
        for (int i = 0; i < remaining_pieces; i++) {
            glm::mat3 current_green_square = visMatrix * transform2D::Translate(i * 12, 0) * base_rem_square;
            RenderMesh2D(meshes["square_green"], shaders["VertexColor"], current_green_square);
        }
    }
    else {
        for (int i = 0; i < min(-remaining_pieces, 10); i++) {
            glm::mat3 current_red_square = visMatrix * transform2D::Translate(i * 12, 0) * base_rem_square;
            RenderMesh2D(meshes["square_red"], shaders["VertexColor"], current_red_square);
        }
    }
    

    // Render the grid of small blue squares, or whatever the player decided to build, based on the
    // keys in the grid matrix: 0 - blue square, 1 - ship hull block, 2 - thruster base position, 
    // 3 - bumper base position, 4 - gun base position, 9 - emtpy block, which cannot be placed
    // into because it has a part of another piece, or because there are some restrictions that
    // interdict using it

    glm::mat3 base_grid_square = glm::mat3(1);
    base_grid_square = transform2D::Translate(24, 8.75) * transform2D::Scale(5, 5);

    glm::mat3 base_grid_piece = glm::mat3(1);
    base_grid_piece = transform2D::Translate(24, 8.75) * transform2D::Scale(7.5, 7.5);

    for (int i = 0; i < grid.size(); i++) {
        for (int j = 0; j < grid[0].size(); j++) {
            switch (grid[i][j]) {
                case BLUE_SQUARE_BLOCK: {
                    glm::mat3 blue_square_pos = visMatrix * transform2D::Translate(j * 7.5 + 1.25, i * 7.5 + 1.25) * base_grid_square;
                    RenderMesh2D(meshes["square_blue"], shaders["VertexColor"], blue_square_pos);
                    break;
                }
                case SHIP_BLOCK: {
                    glm::mat3 ship_block_pos = visMatrix * transform2D::Translate(j * 7.5, i * 7.5) * base_grid_piece;
                    RenderMesh2D(meshes["ship_block"], shaders["VertexColor"], ship_block_pos);
                    break;
                }
                case THRUSTER: {
                    glm::mat3 thruster_pos = visMatrix * transform2D::Translate(j * 7.5, i * 7.5) * base_grid_piece;
                    RenderMesh2D(meshes["thruster"], shaders["VertexColor"], thruster_pos);
                    break;
                }
                case BUMPER: {
                    glm::mat3 bumper_pos = visMatrix * transform2D::Translate(j * 7.5 + 3.75, (i + 1) * 7.5) * base_grid_piece;
                    RenderMesh2D(meshes["bumper"], shaders["VertexColor"], bumper_pos);
                    break;
                }
                case GUN: {
                    glm::mat3 gun_pos = visMatrix * transform2D::Translate(j * 7.5 + 3.75, i * 7.5 + 6) * base_grid_piece;
                    RenderMesh2D(meshes["gun"], shaders["VertexColor"], gun_pos);
                    break;
                }
                default: {
                    // Don't calculate or draw anything if we don't have to ...
                }
            }
        }

        // show ship model
        if (meshes.count("ship")) {
            glm::mat3 ship_model = visMatrix * transform2D::Translate(80, 50) * transform2D::Scale(3, 3);
            RenderMesh2D(meshes["ship"], shaders["VertexColor"], ship_model);
        }
    }

    // Render the object currently held by the player

    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat3 base_held = glm::mat3(1);
    base_held = transform2D::Translate(mouse_coords.x, mouse_coords.y) * transform2D::Scale(7.5, 7.5);

    switch (held_object) {
        case SHIP_BLOCK: {
            glm::mat3 held = visMatrix * transform2D::Translate(-3.75, -3.75) * base_held;
            RenderMesh2D(meshes["ship_block"], shaders["VertexColor"], held);
            break;
        }
        case THRUSTER: {
            glm::mat3 held = visMatrix * transform2D::Translate(-3.75, -3.75) * base_held;
            RenderMesh2D(meshes["thruster"], shaders["VertexColor"], held);
            break;
        }
        case BUMPER: {
            glm::mat3 held = visMatrix * transform2D::Translate(0, 2.25) * base_held;
            RenderMesh2D(meshes["bumper"], shaders["VertexColor"], held);
            break;
        }
        case GUN: {
            glm::mat3 held = visMatrix * transform2D::Translate(0, 3.75) * base_held;
            RenderMesh2D(meshes["gun"], shaders["VertexColor"], held);
            break;
        }
        default: {
            // If the player isn't holding anything, don't draw anything ...
        }
    }
}

void Tema1_editor::FrameEnd() {
}

// Controlling
void Tema1_editor::OnInputUpdate(float deltaTime, int mods)
{
}


void Tema1_editor::OnKeyPress(int key, int mods)
{

}

void Tema1_editor::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1_editor::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    glm::vec3 view_coords = glm::vec3(mouseX, mouseY, 1);
    glm::vec3 logic_coords = inverseVis * view_coords;
    logic_coords.y = logicSpace.height - logic_coords.y;

    mouse_coords = logic_coords;
    // Very epic numbers
    // cout << logic_coords << endl;
}


void Tema1_editor::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT)) {
        if (mouse_coords.x >= 152 && mouse_coords.x <= 159.5 && mouse_coords.y >= 87.5 && mouse_coords.y <= 95) {
            if (can_start == 1) {
                can_start = 2;
                cout << "Game started" << endl;
                vector<shipCollider> colliders;
                auto ship = CreateShipMesh(colliders);
                CreateMesh("ship", ship.first, ship.second);

                gfxc::SimpleScene::player->meshes["ship"] = meshes["ship"];
                gfxc::SimpleScene::player->meshes["circle"] = meshes["circle"];
                gfxc::SimpleScene::editorWorld = false;
                gfxc::SimpleScene::player->shipColliders = colliders;
            }
            else {
                cout << "Error: could not start the game just yet, perhaps check the ship design?" << endl;
            }
        }
        else if (mouse_coords.x >= 8.25 && mouse_coords.x <= 15.75 && mouse_coords.y >= 80 && mouse_coords.y <= 87.5) {
            held_object = BUMPER;
        }
        else if (mouse_coords.x >= 8.25 && mouse_coords.x <= 15.75 && mouse_coords.y >= 51.5 && mouse_coords.y <= 58) {
            held_object = GUN;
        }
        else if (mouse_coords.x >= 8.25 && mouse_coords.x <= 15.75 && mouse_coords.y >= 37.5 && mouse_coords.y <= 45) {
            held_object = THRUSTER;
        }
        else if (mouse_coords.x >= 8.25 && mouse_coords.x <= 15.75 && mouse_coords.y >= 8.75 && mouse_coords.y <= 16.25) {
            held_object = SHIP_BLOCK;
        }
        else {
            held_object = NO_OBJECT_HELD;
        }
    }

    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT)) {
        if (mouse_coords.x >= 24 && mouse_coords.x <= 152 && mouse_coords.y >= 8.75 && mouse_coords.y <= 76.25) {
            unsigned int i = (mouse_coords.y - 8.75) / 7.5;
            unsigned int j = (mouse_coords.x - 24) / 7.5;
            if (grid[i][j] == SHIP_BLOCK) {
                remaining_pieces++;
                grid[i][j] = BLUE_SQUARE_BLOCK;
            }
            else if (grid[i][j] == THRUSTER) {
                remaining_pieces++;
                for (int k = i; k >= 0; k--) {
                    grid[k][j] = BLUE_SQUARE_BLOCK;
                }
            }
            else if (grid[i][j] == BUMPER) {
                remaining_pieces++;
                grid[i][j] = BLUE_SQUARE_BLOCK;
                for (int k = i + 1; k < 9; k++) {
                    grid[k][j - 1] = BLUE_SQUARE_BLOCK;
                    grid[k][j] = BLUE_SQUARE_BLOCK;
                    grid[k][j + 1] = BLUE_SQUARE_BLOCK;
                }
            }
            else if (grid[i][j] == GUN) {
                remaining_pieces++;
                for (int k = i; k < 9; k++) {
                    grid[k][j] = BLUE_SQUARE_BLOCK;
                }
            }
            
        }
    }
    // cout << held_object << endl;
}


void Tema1_editor::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    unsigned int held_object_copy = held_object;
    held_object = NO_OBJECT_HELD;
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_LEFT) && held_object_copy) {
        if (mouse_coords.x >= 24 && mouse_coords.x <= 152 && mouse_coords.y >= 8.75 && mouse_coords.y <= 76.25) {
            unsigned int i = (mouse_coords.y - 8.75) / 7.5;
            unsigned int j = (mouse_coords.x - 24) / 7.5;
            
            if (held_object_copy == SHIP_BLOCK) {
                if (grid[i][j] != BLUE_SQUARE_BLOCK) {
                    return;
                }
                remaining_pieces--;
                grid[i][j] = SHIP_BLOCK;
            }
            if (held_object_copy == THRUSTER) {
                if (i == 0) {
                    return;
                }
                for (int k = i; k >= 0; k--) {
                    if (grid[k][j] != BLUE_SQUARE_BLOCK) {
                        return;
                    }
                }
                remaining_pieces--;
                for (int k = i - 1; k >= 0; k--) {
                    grid[k][j] = NO_BLOCK;
                }
                grid[i][j] = THRUSTER;
            }
            else if (held_object_copy == BUMPER) {
                if (j == 0 || j == 16 || i == 8) {
                    return;
                }
                for (int k = i + 1; k < 9; k++) {
                    if (grid[k][j - 1] != BLUE_SQUARE_BLOCK || grid[k][j] != BLUE_SQUARE_BLOCK || grid[k][j + 1] != BLUE_SQUARE_BLOCK) {
                        return;
                    }
                }
                if (grid[i][j] != BLUE_SQUARE_BLOCK) {
                    return;
                }
                remaining_pieces--;
                for (int k = i + 1; k < 9; k++) {
                    grid[k][j - 1] = NO_BLOCK;
                    grid[k][j] = NO_BLOCK;
                    grid[k][j + 1] = NO_BLOCK;
                }
                grid[i][j] = BUMPER;
            }
            else if (held_object_copy == GUN) {
                if (i > 6) {
                    return;
                }
                for (int k = i; k < 9; k++) {
                    if (grid[k][j] != BLUE_SQUARE_BLOCK) {
                        return;
                    }
                }
                remaining_pieces--;
                for (int k = i + 1; k < 9; k++) {
                    grid[k][j] = NO_BLOCK;
                }
                grid[i][j] = GUN;
            }
        }
    }
}


void Tema1_editor::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema1_editor::OnWindowResize(int width, int height)
{
}

// Other functions for different purposes
void Tema1_editor::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
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

bool Tema1_editor::CheckConnectivity() {
    vector<pair<int, int>> directions = { {-1, 0}, {1, 0}, {0, 1}, {0, -1} };
    vector<vector<unsigned int>> visited = vector<vector<unsigned int>>(GRID_HEIGHT, vector<unsigned int>(GRID_WIDTH, 0));

    bool first_component = true;
    queue<pair<int, int>> Q;

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if ((grid[i][j] == SHIP_BLOCK || grid[i][j] == BUMPER || grid[i][j] == THRUSTER || grid[i][j] == GUN) && !visited[i][j]) {
                if (first_component) {
                    Q.push({ i, j });
                    while (Q.size()) {
                        int i1 = Q.front().first;
                        int j1 = Q.front().second;
                        Q.pop();
                        visited[i1][j1] = 1;

                        for (auto dir : directions) {
                            if (i1 + dir.first < 0 || i1 + dir.first >= GRID_HEIGHT || j1 + dir.second < 0 || j1 + dir.second >= GRID_WIDTH) {
                                continue;
                            }
                            if ((grid[i1 + dir.first][j1 + dir.second] == SHIP_BLOCK || grid[i1 + dir.first][j1 + dir.second] == BUMPER ||
                                grid[i1 + dir.first][j1 + dir.second] == THRUSTER || grid[i1 + dir.first][j1 + dir.second] == GUN) &&
                                !visited[i1 + dir.first][j1 + dir.second]) {
                                Q.push({ i1 + dir.first, j1 + dir.second });
                            }
                        }
                    }
                    first_component = false;
                }
                else {
                    return false;
                }
            }
        }
    }

    return true;
}

// Viewport - Window
glm::mat3 Tema1_editor::VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
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

glm::mat3 Tema1_editor::VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
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


void Tema1_editor::SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear)
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

void Tema1_editor::findShipCenter(float& index_i, float& index_j) {
    unsigned int maxi = 0;
    unsigned int mini = GRID_HEIGHT;
    unsigned int maxj = 0;
    unsigned int minj = GRID_WIDTH;

    for (unsigned int i = 0; i < GRID_HEIGHT; i++) {
        for (unsigned int j = 0; j < GRID_WIDTH; j++) {
            if (grid[i][j] == SHIP_BLOCK || grid[i][j] == THRUSTER || grid[i][j] == BUMPER || grid[i][j] == GUN) {
            maxi = max(maxi, i);
            mini = min(mini, i);
            maxj = max(maxj, j);
            minj = min(minj, j);
            }
        }
    }

    index_i = 1.0 * (maxi + mini) / 2;
    index_j = 1.0 * (maxj + minj) / 2;
}

pair<vector<VertexFormat>, vector<unsigned int>> Tema1_editor::CreateShipMesh(vector<shipCollider> &colliders) {
    float center_i;
    float center_j;
    findShipCenter(center_i, center_j);

    vector<VertexFormat> ship_vertices;
    vector<unsigned int> ship_indices;
    int current_vertices = 0;
    int last_vertices = 0;

    for (int i = 0; i < GRID_HEIGHT; i++) {
        for (int j = 0; j < GRID_WIDTH; j++) {
            if (grid[i][j] == SHIP_BLOCK || grid[i][j] == THRUSTER || grid[i][j] == BUMPER || grid[i][j] == GUN) {
                float offset_x = (j - center_j);
                float offset_y = (i - center_i);

                switch (grid[i][j]) {
                    case SHIP_BLOCK: {
                        for (int k = 0; k < ship_block_vertices.size(); k++) {
                            ship_vertices.push_back(ship_block_vertices[k]);
                            ship_vertices[k + current_vertices].position.x += offset_x - 0.5f;
                            ship_vertices[k + current_vertices].position.y += offset_y - 0.5f;
                        }
                        current_vertices += ship_block_vertices.size();
                        for (int k = 0; k < ship_block_indices.size(); k++) {
                            ship_indices.push_back(ship_block_indices[k] + last_vertices);
                        }
                        last_vertices = current_vertices;
                        
                        colliders.push_back(shipCollider(offset_x - 0.5f, offset_y - 0.5f, 1, 1, false));
                        break;
                    }
                    case THRUSTER: {
                        for (int k = 0; k < thruster_vertices.size(); k++) {
                            ship_vertices.push_back(thruster_vertices[k]);
                            ship_vertices[k + current_vertices].position.x += offset_x - 0.5f;
                            ship_vertices[k + current_vertices].position.y += offset_y - 0.5f;
                        }
                        current_vertices += thruster_vertices.size();
                        for (int k = 0; k < thruster_indices.size(); k++) {
                            ship_indices.push_back(thruster_indices[k] + last_vertices);
                        }
                        last_vertices = current_vertices;

                        colliders.push_back(shipCollider(offset_x - 0.5f, offset_y - 0.5f, 1, 1, false));
                        break;
                    }
                    case BUMPER: {
                        for (int k = 0; k < bumper_vertices.size(); k++) {
                            ship_vertices.push_back(bumper_vertices[k]);
                            ship_vertices[k + current_vertices].position.x += offset_x;
                            ship_vertices[k + current_vertices].position.y += offset_y + 0.5f;
                        }
                        current_vertices += bumper_vertices.size();
                        for (int k = 0; k < bumper_indices.size(); k++) {
                            ship_indices.push_back(bumper_indices[k] + last_vertices);
                        }
                        last_vertices = current_vertices;
                        colliders.push_back(shipCollider(offset_x - 1.5f, offset_y + 0.5f, 3, 1, true));
                        colliders.push_back(shipCollider(offset_x - 0.5f, offset_y - 0.5f, 1, 1, false));
                        break;
                    }
                    case GUN: {
                        for (int k = 0; k < gun_vertices.size(); k++) {
                            ship_vertices.push_back(gun_vertices[k]);
                            ship_vertices[k + current_vertices].position.x += offset_x;
                            ship_vertices[k + current_vertices].position.y += offset_y + 0.3f;
                        }
                        current_vertices += gun_vertices.size();
                        for (int k = 0; k < gun_indices.size(); k++) {
                            ship_indices.push_back(gun_indices[k] + last_vertices);
                        }
                        last_vertices = current_vertices;

                        colliders.push_back(shipCollider(offset_x - 0.5f, offset_y - 0.5f, 1, 1, false));
                        break;
                    }
                }
            }
        }
    }

    return { ship_vertices, ship_indices };
}