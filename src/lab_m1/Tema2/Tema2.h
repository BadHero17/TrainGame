#pragma once

#include <vector>

#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include "camera.h"
using namespace std;

pair<vector<VertexFormat>, vector<unsigned int>> create_cube_vertices(float scaleX, float scaleY, float scaleZ, glm::vec3 color);
pair<vector<VertexFormat>, vector<unsigned int>> create_tube_vertices(float radius, float height, glm::vec3 color);
pair<vector<VertexFormat>, vector<unsigned int>> create_pyramid_vertices(float height, float base, glm::vec3(base_color));
pair<vector<VertexFormat>, vector<unsigned int>> create_wood_vertices();
pair<vector<VertexFormat>, vector<unsigned int>> create_brick_vertices();
pair<vector<VertexFormat>, vector<unsigned int>> create_coal_vertices();
pair<vector<VertexFormat>, vector<unsigned int>> create_square_vertices(float scaleX, float scaleY, glm::vec3 color);
pair<vector<VertexFormat>, vector<unsigned int>> create_triangle_vertices(glm::vec3 color);
void add_vertices(pair<vector<VertexFormat>, vector<unsigned int>>& baseV, pair<vector<VertexFormat>, vector<unsigned int>>& addV, glm::mat4 modelTransform);

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

        struct LogicRail
        {
            bool orientation;
            float start;
            float end;
            float heightStart;
            float heightEnd;
            // constant coordinate
            float position;

            // On the rails with orientation along Z, we only use railZ*plusX
            // and on the ones along X, we only use railX*plusZ
            LogicRail* railXplusZminus = nullptr;
            LogicRail* railXplusZplus = nullptr;
            LogicRail* railXminusZminus = nullptr;
            LogicRail* railXminusZplus = nullptr;
            LogicRail* railZplusXminus = nullptr;
            LogicRail* railZplusXplus = nullptr;
            LogicRail* railZminusXminus = nullptr;
            LogicRail* railZminusXplus = nullptr;

            LogicRail(bool orientation, float start, float end, float position, float heightStart, float heightEnd) :
                orientation(orientation), start(start), end(end), position(position), heightStart(heightStart), heightEnd(heightEnd) {};
        };

        struct LogicTrain
        {
            bool locSense = 0;
            bool wag1Sense = 0;
            bool wag2Sense = 0;
            bool prevSense = 0;
            bool wag1prevSense = 0;
            LogicRail* locomotiveRail;
            LogicRail* wag1Rail;
            LogicRail* wag2Rail;
            LogicRail* wag1prevRail;
            LogicRail* prevRail;
            float speed = 0.5f;
            float locMoveProg = 0.0f;
            float wag1MoveProg = 0.5f;
            float wag2MoveProg = 0.0f;

            LogicTrain(LogicRail* locomotiveRail, LogicRail* wag1Rail, LogicRail* wag2Rail, LogicRail* prevRail, LogicRail* wag1prevRail) :
                locomotiveRail(locomotiveRail), wag1Rail(wag1Rail), wag2Rail(wag2Rail), prevRail(prevRail), wag1prevRail(wag1prevRail) {};

            LogicTrain() : locomotiveRail(nullptr), wag1Rail(nullptr), wag2Rail(nullptr), prevRail(nullptr), wag1prevRail(wag1prevRail) {};
        };

    private:
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        void RenderChangingMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::vec3 color1, glm::vec3 color2, float t);
        void MakeLocomotive();
        void MakeWagon();
        void GeneratePerlin();
        void MakeTerrain();
        void AddRiver();
        void MakeWater();
        void MakeRails();
        void MakeStation();
        void AddStations();
        void ConnectWithRail(int pt1X, int pt1Z, int pt2X, int pt2Z);
        void AddRails();
        void MakeRailway();
        void AddLogicRailway();
        void AddTrain();
        float lerp(float a, float b, float t);
        void UpdateTrain(float deltaTimeSeconds);
        void HandleTrainDeliveries();
        void RenderUI();
        void MakeMinimapBase();

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    protected:
        int windowHeight;
        int windowWidth;
        int heightScale;
        int widthScale;

        cameraSpace::Camera* camera;
        float initialZ;
        float initialX;
        float initialY;
        float pitch;

        glm::mat4 projectionMatrix;
        vector<vector<float>> heightMap;
        vector<vector<unsigned int>> indexMap;
        vector<pair<unsigned int, unsigned int>> stations;
        vector<pair<unsigned int, unsigned int>> intersections;
        vector<vector<LogicRail*>> railwayZ;
        vector<vector<LogicRail*>> railwayX;
        gfxc::TextRenderer* textRenderer;

        LogicTrain train;
        int lastDirectionPressed;

        vector<int> requiredResources;
        int heldCoal;
        int heldWood;
        int heldBrick;
        float loadCooldown;
        float resRotationHeight;
        float newCommandDelay;
        float remainingTime;

        bool gameLost;
        int score;
    };
};