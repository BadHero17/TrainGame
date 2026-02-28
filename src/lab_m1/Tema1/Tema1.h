#pragma once

#include <vector>

#include "components/simple_scene.h"
#include "components/text_renderer.h"
using namespace std;

pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_circle(glm::vec3 color, bool full);
pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_square(glm::vec3 color, glm::vec3 center_color, double scaleX, double scaleY);
pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_K(glm::vec3 color);
pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_fire(glm::vec3 color1, glm::vec3 color2);

namespace m1
{
    class Tema1_editor : public gfxc::SimpleScene
    {
    public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {
            }
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {
            }
            float x;
            float y;
            float width;
            float height;
        };

        Tema1_editor();
        ~Tema1_editor();

        void Init() override;

    private:
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        glm::mat3 VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);
        glm::mat3 VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);

        void SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor = glm::vec3(0), bool clear = true);

        bool CheckConnectivity();
        void findShipCenter(float& indexI, float& indexJ);
        pair<vector<VertexFormat>, vector<unsigned int>> Tema1_editor::CreateShipMesh(vector<shipCollider>& colliders);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        // Viewport - Window transformation variables
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        glm::mat3 modelMatrix, visMatrix, inverseVis;

        // Logical variables for ship editing
        int remaining_pieces;
        vector<vector<unsigned int>> grid;
        glm::vec3 mouse_coords;
        unsigned int held_object;

        // Vertices and indexes for the main components of the ship
        vector<VertexFormat> thruster_vertices;
        vector<unsigned int> thruster_indices;
        vector<VertexFormat> bumper_vertices;
        vector<unsigned int> bumper_indices;
        vector<VertexFormat> gun_vertices;
        vector<unsigned int> gun_indices;
        vector<VertexFormat> ship_block_vertices;
        vector<unsigned int> ship_block_indices;

        // Everything is set and the game may be started, or not
        bool can_start = false;
    };

    class Tema1_player : public gfxc::SimpleScene
    {
    public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {
            }
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {
            }
            float x;
            float y;
            float width;
            float height;
        };

        struct Animation
        {
            float x;
            float y;
            float directionX;
            float directionY;
            float remainingDuration;
            int animationType;
            int particleColor;
            float speed;

            Animation(float x, float y,
                      float directionX, float directionY,
                      float remainingDuration,
                      int animationType,
                      int particleColor,
                      float speed): x(x), y(y), directionX(directionX), directionY(directionY), remainingDuration(remainingDuration), animationType(animationType), particleColor(particleColor), speed(speed) { }
            ~Animation() {}
        };

        Tema1_player();
        ~Tema1_player();

        void Init() override;

    private:
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        glm::mat3 VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);
        glm::mat3 VisualizationTransf2DUnif(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);

        void SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor = glm::vec3(0), bool clear = true);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        // Viewport - Window transformation variables
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        glm::mat3 modelMatrix, visMatrix, inverseVis;

        // Game logic
        vector<vector<unsigned int>> grid;

        // Determine the position of the ship. The ship cannot change it's y
        float shipOffsetX;

        // Determine the state of the ball
        bool ballThrown;
        float ballX, ballY;
        float ballSpeedX, ballSpeedY;

        // Render text
        gfxc::TextRenderer* textRenderer;

        // Game state
        unsigned int lives;
        unsigned int score;
        bool gameFinished;
        bool gameLost;
        bool gameWon;

        // Keep track of all currently ongoing animation events
        vector<Animation> animations;

        // Screen shake animation
        float ssDuration;
        float ssSpeedX;
        float ssSpeedY;
    };

}   // namespace m1
