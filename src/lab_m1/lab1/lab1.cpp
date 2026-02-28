#include "lab_m1/lab1/lab1.h"

#include <vector>
#include <iostream>
#include <string>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab1::Lab1()
{
    // TODO(student): Never forget to initialize class variables!
    clearRed = 0;
    clearGreen = 0;
    clearBlue = 0;

    teapotX = 0.5;
    teapotY = -1;
    teapotZ = 0;

    objectX = 0.5;
    objectY = 1;
    objectZ = 0;

    offset = 0;
    speed = 0;
    gravity = -10;

    objectIndex = 0;
    cycleObjects.push_back("box");
    cycleObjects.push_back("sphere");
    cycleObjects.push_back("teapot");
}


Lab1::~Lab1()
{
}


void Lab1::Init()
{
    // Load a mesh from file into GPU memory. We only need to do it once,
    // no matter how many times we want to draw this mesh.
    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // TODO(student): Load some more meshes. The value of RESOURCE_PATH::MODELS
    // is actually a path on disk, go there and you will find more meshes.

    Mesh* mesh1 = new Mesh("sphere");
    mesh1->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
    meshes[mesh1->GetMeshID()] = mesh1;

    Mesh* mesh2 = new Mesh("teapot");
    mesh2->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "teapot.obj");
    meshes[mesh2->GetMeshID()] = mesh2;

}


void Lab1::FrameStart()
{
}


void Lab1::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->props.resolution;

    speed += deltaTimeSeconds * gravity;
    offset = MAX(0, offset + deltaTimeSeconds * speed);

    // Sets the clear color for the color buffer

    // TODO(student): Generalize the arguments of `glClearColor`.
    // You can, for example, declare three variables in the class header,
    // that will store the color components (red, green, blue).
    glClearColor(clearRed, clearGreen, clearBlue, 1);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);

    // Render the object
    RenderMesh(meshes["box"], glm::vec3(1, 0.5f, 0), glm::vec3(0.5f));

    // Render the object again but with different properties
    RenderMesh(meshes["box"], glm::vec3(-1, 0.5f, 0));

    // TODO(student): We need to render (a.k.a. draw) the mesh that
    // was previously loaded. We do this using `RenderMesh`. Check the
    // signature of this function to see the meaning of its parameters.
    // You can draw the same mesh any number of times.

    RenderMesh(meshes[cycleObjects[objectIndex]], glm::vec3(objectX, objectY + offset, objectZ), glm::vec3(0.5f, 1, 0.75f));

    RenderMesh(meshes["teapot"], glm::vec3(teapotX, teapotY + offset, teapotZ));
}


void Lab1::FrameEnd()
{
    DrawCoordinateSystem();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab1::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input

    // TODO(student): Add some key hold events that will let you move
    // a mesh instance on all three axes. You will also need to
    // generalize the position used by `RenderMesh`.

    if ((window->GetSpecialKeyState() & GLFW_MOD_SHIFT) && window->KeyHold(GLFW_KEY_Z))
    {
        teapotX += 0.5 * deltaTime;
        teapotY += 0.5 * deltaTime;
        teapotZ += 0.5 * deltaTime;
    }

    if ((window->GetSpecialKeyState() & GLFW_MOD_SHIFT) && window->KeyHold(GLFW_KEY_X))
    {
        teapotX -= 0.5 * deltaTime;
        teapotY -= 0.5 * deltaTime;
        teapotZ -= 0.5 * deltaTime;
    }

    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) return;

    if (window->KeyHold(GLFW_KEY_A)) objectX -= deltaTime;
    if (window->KeyHold(GLFW_KEY_D)) objectX += deltaTime;
    if (window->KeyHold(GLFW_KEY_W)) objectZ += deltaTime;
    if (window->KeyHold(GLFW_KEY_S)) objectZ -= deltaTime;
    if (window->KeyHold(GLFW_KEY_Q)) objectY -= deltaTime;
    if (window->KeyHold(GLFW_KEY_E)) objectY += deltaTime;

}


void Lab1::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F) {
        // TODO(student): Change the values of the color components.
        clearRed = clearRed == 0.5 ? 0 : 0.5;
        clearBlue = clearBlue == 0.75 ? 0 : 0.75;
    }

    // TODO(student): Add a key press event that will let you cycle
    // through at least two meshes, rendered at the same position.
    // You will also need to generalize the mesh name used by `RenderMesh`.

    if (key == GLFW_KEY_G) {
        objectIndex = (objectIndex + 1) % 3;
    }

    if (key == GLFW_KEY_SPACE) {
        speed = 10;
    }

}


void Lab1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Lab1::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
