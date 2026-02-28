#include "lab_m1/lab3/lab3.h"

#include <vector>
#include <iostream>

#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab3::Lab3()
{
}


Lab3::~Lab3()
{
}


void Lab3::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    glm::vec3 corner = glm::vec3(0, 0, 0);
    float squareSide = 100;
    direction = 200;
    scaleDirection = 1;

    // TODO(student): Compute coordinates of a square's center, and store
    // then in the `cx` and `cy` class variables (see the header). Use
    // `corner` and `squareSide`. These two class variables will be used
    // in the `Update()` function. Think about it, why do you need them?

    cx = corner.x + squareSide / 2;
    cy = corner.y + squareSide / 2;

    // Initialize tx and ty (the translation steps)
    translateX = 0;
    translateY = 0;

    // Initialize sx and sy (the scale factors)
    scaleX = 1;
    scaleY = 1;

    // Initialize angularStep
    angularStep = 0;

    Mesh* square1 = object2D::CreateSquare("square1", corner, squareSide, glm::vec3(1, 0, 0), true);
    AddMeshToList(square1);

    Mesh* square2 = object2D::CreateSquare("square2", corner, squareSide, glm::vec3(0, 1, 0));
    AddMeshToList(square2);

    Mesh* square3 = object2D::CreateSquare("square3", corner, squareSide, glm::vec3(0, 0, 1));
    AddMeshToList(square3);
}


void Lab3::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab3::Update(float deltaTimeSeconds)
{
    // TODO(student): Update steps for translation, rotation and scale,
    // in order to create animations. Use the class variables in the
    // class header, and if you need more of them to complete the task,
    // add them over there!

    modelMatrix = glm::mat3(1);

    if (translateY >= 400 && direction > 0) {
        direction = -direction;
    }
    else if (translateY <= -100 && direction < 0) {
        direction = -direction;
    }
    translateY += direction * deltaTimeSeconds;

    glm::mat3 translateMatrix = transform2D::Translate(0, translateY);

    modelMatrix *= transform2D::Translate(150, 250);
    // TODO(student): Create animations by multiplying the current
    // transform matrix with the matrices you just implemented.
    // Remember, the last matrix in the chain will take effect first!

    

    RenderMesh2D(meshes["square1"], shaders["VertexColor"], translateMatrix * modelMatrix);

    glm::mat3 centrateMatrix = transform2D::Translate(-cx, -cy);
    glm::mat3 centrateInverseMatrix = transform2D::Translate(cx, cy);

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(400, 250);
    // TODO(student): Create animations by multiplying the current
    // transform matrix with the matrices you just implemented
    // Remember, the last matrix in the chain will take effect first!

    if (scaleX >= 3.0f && scaleDirection > 0) {
        scaleDirection = -scaleDirection;
        scaleX = 3.0f; // clamp
    }
    else if (scaleX <= 0.5f && scaleDirection < 0) {
        scaleDirection = -scaleDirection;
        scaleX = 0.5f; // clamp
    }

    scaleX += scaleDirection * deltaTimeSeconds;
    scaleY += scaleDirection * deltaTimeSeconds;

    glm::mat3 scaleMatrix = transform2D::Scale(scaleX, scaleY);

    RenderMesh2D(meshes["square2"], shaders["VertexColor"], modelMatrix * glm::inverse(centrateMatrix) * scaleMatrix * centrateMatrix);

    modelMatrix = glm::mat3(1);
    modelMatrix *= transform2D::Translate(650, 250);
    // TODO(student): Create animations by multiplying the current
    // transform matrix with the matrices you just implemented
    // Remember, the last matrix in the chain will take effect first!

    angularStep += 0.4 * deltaTimeSeconds;
    glm::mat3 rotationMatrix = transform2D::Rotate(angularStep);

    RenderMesh2D(meshes["square3"], shaders["VertexColor"], modelMatrix * glm::inverse(centrateMatrix) * rotationMatrix * centrateMatrix);

    glm::mat3 pos = transform2D::Translate(300, 300);

    glm::mat3 rotateAround = transform2D::Translate(-50, 66);
    glm::mat3 rotateAroundInverse = transform2D::Translate(50, -66);

    glm::mat3 firstRot = transform2D::Rotate(0 + angularStep);
    glm::mat3 secondRot = transform2D::Rotate(1.25 + angularStep);
    glm::mat3 thirdRot = transform2D::Rotate(2.51327 + angularStep);
    glm::mat3 fourthRot = transform2D::Rotate(3.76991 + angularStep);
    glm::mat3 fifthRot = transform2D::Rotate(5.02655 + angularStep);

    RenderMesh2D(meshes["square3"], shaders["VertexColor"], pos * rotateAroundInverse * firstRot * rotateAround);
    RenderMesh2D(meshes["square3"], shaders["VertexColor"], pos * rotateAroundInverse * secondRot * rotateAround);
    RenderMesh2D(meshes["square3"], shaders["VertexColor"], pos * rotateAroundInverse * thirdRot * rotateAround);
    RenderMesh2D(meshes["square3"], shaders["VertexColor"], pos * rotateAroundInverse * fourthRot * rotateAround);
    RenderMesh2D(meshes["square3"], shaders["VertexColor"], pos * rotateAroundInverse * fifthRot * rotateAround);
}


void Lab3::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab3::OnInputUpdate(float deltaTime, int mods)
{
}


void Lab3::OnKeyPress(int key, int mods)
{
    // Add key press event
}


void Lab3::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab3::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab3::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab3::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab3::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab3::OnWindowResize(int width, int height)
{
}
