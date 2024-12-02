//
//  main.cpp
//  3D Object Drawing
//
//  Created by Nazirul Hasan on 4/9/23.
//  modified by Badiuzzaman on 3/11/24.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "basic_camera.h"
#include "camera.h"


#include <iostream>

using namespace std;

#define PI 3.14159265359

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 translateMatrix, glm::mat4 sm);

int drawAll(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans);

void drawCube(
    Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
    float posX = 0.0, float posY = 0.0, float posz = 0.0,
    float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0,
    float scX = 1.0, float scY = 1.0, float scZ = 1.0,
    float r = 0.0, float g = 0.0, float b = 0.0);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// modelling transform
float rotateAngle_X = 45.0;
float rotateAngle_Y = 45.0;
float rotateAngle_Z = 45.0;
float rotateAxis_X = 0.0;
float rotateAxis_Y = 0.0;
float rotateAxis_Z = 1.0;
float translate_X = 0.0;
float translate_Y = 0.0;
float translate_Z = 0.0;
float scale_X = 1.0;
float scale_Y = 1.0;
float scale_Z = 1.0;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float eyeX = 1.35, eyeY = 4.8, eyeZ = 10.0;
float lookAtX = 4.0, lookAtY = 4.0, lookAtZ = 6.0;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);



// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

bool on = false;

//bool birdEyeView = false;
//glm::vec3 birdEyePosition(1.0f, 2.5f, 3.0f); // Initial position (10 units above)
//glm::vec3 birdEyeTarget(1.0f, 0.0f, 0.0f);   // Focus point
//float birdEyeSpeed = 1.0f;

//birds eye
//bool birdEye = false;
//glm::vec3 cameraPos(-2.0f, 5.0f, 13.0f);
//glm::vec3 target(-2.0f, 0.0f, 5.5f);
//float birdEyeSpeed = 1.0f;
bool birdEye = false;
glm::vec3 cameraPos(1.0f, 2.5f, 3.0f);
glm::vec3 target(1.0f, 0.0f, 0.0f);
float birdEyeSpeed = 1.0f;


int initGlfw(GLFWwindow*& window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LabOne", NULL, NULL);
    if (window == NULL) { cout << "Failed to create GLFW window" << endl; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { cout << "Failed to initialize GLAD" << endl; return -1; }

    // build and compile our shader program
    return 0;
}

void safeTerminate(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO) {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwTerminate();
}

void initBinding(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, Shader& ourShader, float* cube_vertices, int verticesSize, unsigned int* cube_indices, int indicesSize) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ARRAY_BUFFER, verticesSize, cube_vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);
    ourShader.use();
}


int main()
{
    GLFWwindow* window = nullptr;
    if (initGlfw(window)) return -1;

    glEnable(GL_DEPTH_TEST);

    Shader ourShader(
        "VertexShader.vs",
        "FragmentShader.fs"
    );

    Shader constantShader(
        "VertexShader.vs",
        "FragmentShaderV2.fs"
    );

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cube_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.3f, 0.8f, 0.5f,
        1.0f, 0.0f, 0.0f, 0.5f, 0.4f, 0.3f,
        1.0f, 1.0f, 0.0f, 0.2f, 0.7f, 0.3f,
        0.0f, 1.0f, 0.0f, 0.6f, 0.2f, 0.8f,
        0.0f, 0.0f, 1.0f, 0.8f, 0.3f, 0.6f,
        1.0f, 0.0f, 1.0f, 0.4f, 0.4f, 0.8f,
        1.0f, 1.0f, 1.0f, 0.2f, 0.3f, 0.6f,
        0.0f, 1.0f, 1.0f, 0.7f, 0.5f, 0.4f
    };
    unsigned int cube_indices[] = {
       0, 3, 2,
       2, 1, 0,

       1, 2, 6,
       6, 5, 1,

       5, 6, 7,
       7 ,4, 5,

       4, 7, 3,
       3, 0, 4,

       6, 2, 3,
       3, 7, 6,

       1, 5, 4,
       4, 0, 1
    };

    unsigned int VBO, VAO, EBO;
    initBinding(VAO, VBO, EBO, ourShader, cube_vertices, sizeof(cube_vertices), cube_indices, sizeof(cube_indices));

    float r = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        constantShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view;

        if (birdEye) {
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            view = glm::lookAt(cameraPos, target, up);
        }
        else {
            view = basic_camera.createViewMatrix();
        }

        //glm::mat4 view = basic_camera.createViewMatrix();
        ourShader.setMat4("view", view);
        //constantShader.setMat4("view", view);
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered, translateMatrixprev;
        translateMatrix = identityMatrix;

        // drawing
        drawAll(ourShader, VAO, identityMatrix);
        // drawing above

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    safeTerminate(VAO, VBO, EBO);
    return 0;
}

float r = 0.0f;

int drawAll(Shader ourShader, unsigned int VAO, glm::mat4 identityMatrix) {
    // floor
    drawCube(ourShader, VAO, identityMatrix, 0, 0, 0, 0, 0, 0, 6, .1, 6, 0.76, 0.57, 0.37);

    // ceiling
    drawCube(ourShader, VAO, identityMatrix, 0, 5, 0, 0, 0, 0, 6, .1, 6, 1.0, 0.99, 0.82);

    // right wall
    drawCube(ourShader, VAO, identityMatrix, 0, .1, -.05, 0, 0, 0, 6, 5, .1, 0.678, 0.847, 0.902);
    // left wall
    drawCube(ourShader, VAO, identityMatrix, -0.05, .1, 0, 0, 0, 0, .1, 5, 6, 0.678, 0.847, 0.902);

    // right shelf
    drawCube(ourShader, VAO, identityMatrix, 0.1, 1.5, .1, 0, 0, 0, 4, .1, 1.2, 1.0, 0.99, 0.82);
    // left shelf
    drawCube(ourShader, VAO, identityMatrix, 0.1, 1.5, .1, 0, 0, 0, 1.2, .1, 5.9, 1.0, 0.99, 0.82);

    // left wall shelf
    int total = 4;
    for (int i = 0; i < total; i++) {
        float gap = (1 / 10.0);
        float width = .8;

        drawCube(ourShader, VAO, identityMatrix, 0, 2.5, (i * width + i * gap),
            0, 0, 0, .6, 1, width, 0.70,0.45,0.56
        );

        if (i == total - 1) continue;
        drawCube(ourShader, VAO, identityMatrix, 0, 2.5, (i * width + i * gap) + width,
            0, 0, 0, .6, 1, gap, 1.0, 0.99, 0.82
        );
    }
    // right wall shelf
    drawCube(ourShader, VAO, identityMatrix, .65, 2.5, 0, 0, 0, 0, .8, 1, .6, 0.70, 0.45, 0.56);
    // right wall shelf white
    drawCube(ourShader, VAO, identityMatrix, .65, 2.55, .6, 0, 0, 0, .7, .9, .05, 0.99,0.99,0.99);

    // right wall window?
    drawCube(ourShader, VAO, identityMatrix, 2, 2, .1, 0, 0, 0, 2, 1.5, .1, 0.70, 0.45, 0.56);
    // right wall window? white
    drawCube(ourShader, VAO, identityMatrix, 2.05, 2.05, .15, 0, 0, 0, .9, 1.4, .1, 0.99, 0.99, 0.99);
    drawCube(ourShader, VAO, identityMatrix, 3.05, 2.05, .15, 0, 0, 0, .9, 1.4, .1, 0.99, 0.99, 0.99);

    // lower shelf left
    total = 6;
    for (int i = 0; i < total; i++) {
        float gap = (1 / 10.0);
        float width = .8;

        drawCube(ourShader, VAO, identityMatrix, 0, 0, .5 + (i * width + i * gap),
            0, 0, 0, 1.2, 1.5, width, 0.70, 0.45, 0.56
        );

        if (i == total - 1) continue;
        drawCube(ourShader, VAO, identityMatrix, 0, 0, .5 + (i * width + i * gap) + width,
            0, 0, 0, 1.2, 1.5, gap, 0.99, 0.99, 0.99
        );
    }

    // right wall shelf bottom
    total = 4;
    for (int i = 0; i < total; i++) {
        float gap = (1 / 10.0);
        float width = .6;

        drawCube(ourShader, VAO, identityMatrix, 1.2 + (i * width + i * gap), 0, 0,
            0, 0, 0, width, 1.5, 1.2, 0.70, 0.45, 0.56
        );

        if (i == total - 1) continue;
        drawCube(ourShader, VAO, identityMatrix, 1.2 + (i * width + i * gap + width), 0, 0,
            0, 0, 0, gap, 1.5, 1.2, 0.99, 0.99, 0.99
        );
    }

    // refrigerator
    drawCube(ourShader, VAO, identityMatrix, 4, 0, 0, 0, 0, 0, 2, 3.5, 1.5, 0.70, 0.45, 0.56);
    drawCube(ourShader, VAO, identityMatrix, 4.05, 0, 1.5, 0, 0, 0, .95, 3.5, .05, 0.99, 0.99, 0.99);
    drawCube(ourShader, VAO, identityMatrix, 5.05, 0, 1.5, 0, 0, 0, .95, 3.5, .05, 0.99, 0.99, 0.99);
    // refrigerator handle
    drawCube(ourShader, VAO, identityMatrix, 4.9, 1.5, 1.55, 0, 0, 0, .05, 1.1, .05, 20 / 255.0, 20 / 255.0, 20 / 255.0);
    drawCube(ourShader, VAO, identityMatrix, 5.1, 1.5, 1.55, 0, 0, 0, .05, 1.1, .05, 20 / 255.0, 20 / 255.0, 20 / 255.0);

    // table-top
    drawCube(ourShader, VAO, identityMatrix, 3, 1.5, 4, 0, 0, 0, 2, .1, 1.5, 0.70, 0.45, 0.56);
    // left top leg
    drawCube(ourShader, VAO, identityMatrix, 3, 0, 4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);
    // right top leg
    drawCube(ourShader, VAO, identityMatrix, 4.9, 0, 4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);
    // left bottom leg
    drawCube(ourShader, VAO, identityMatrix, 3, 0, 5.4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);
    // right bottom leg
    drawCube(ourShader, VAO, identityMatrix, 4.9, 0, 5.4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);

    for (int z = 0; z < 2; z++) {
        for (int x = 0; x < 2; x++) {
            float width = 0.5;
            float gap = 0.4;
            int zz = (z == 0) ? 1 : 0;

            // chairs
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.2, .8, (z * 2 + 3), 0, 0, 0, .5, .1, .5, 0.70,0.10,0.17);
            // left top leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.2, 0, (z * 2 + 3), 0, 0, 0, .1, (.8 + zz * .7), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);


            // left left leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.33, 0.9, (z * 2 + 3 + .5 * z - z * .1), 0, 0, 0, .05, .6, .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
            // left mid leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.44, 0.9, (z * 2 + 3 + .5 * z - z * .1), 0, 0, 0, .05, .6, .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
            // left right leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.55, 0.9, (z * 2 + 3 + .5 * z - z * .1), 0, 0, 0, .05, .6, .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);


            // right top leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.6, 0, (z * 2 + 3), 0, 0, 0, .1, (.8 + .7 * zz), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);

            zz = !zz;
            // left bottom leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.2, 0, (z * 2 + 3.4), 0, 0, 0, .1, (.8 + .7 * zz), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
            // right bottom leg
            drawCube(ourShader, VAO, identityMatrix, (width + gap) * x + 3.6, 0, (z * 2 + 3.4), 0, 0, 0, .1, (.8 + .7 * zz), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
        }
    }

     //sink
    drawCube(ourShader, VAO, identityMatrix, 2, 1.6, .1, 0, 0, 0, 1.5, .02, 1.3, 24 / 255.0, 21 / 255.0, 22 / 255.0);
    total = 20;
    float unit = (1.3 / (2 * total));
    for (int z = 0; z < total; z++) {
        drawCube(ourShader, VAO, identityMatrix, 2, 1.63, (z + 1) * 2 * unit, 0, 0, 0, 1, .01, unit / 2, 60 / 255.0, 60 / 255.0, 60 / 255.0);
    }
    //// the real tap
   /* drawCube(ourShader, VAO, identityMatrix, 3.2, 1.6, .3, 0, 0, 0, .05, .5, .05, 200 / 255.0, 200 / 255.0, 200 / 255.0);
    drawCube(ourShader, VAO, identityMatrix, 3.2, 2.1, .3, 0, 0, 0, .05, .05, .3, 200 / 255.0, 200 / 255.0, 200 / 255.0);
    drawCube(ourShader, VAO, identityMatrix, 3.2, 2.0, .6, 0, 0, 0, .05, .2, .05, 200 / 255.0, 200 / 255.0, 200 / 255.0);*/

    // oven
    drawCube(ourShader, VAO, identityMatrix, 0.1, 1.6, 4, 0, 0, 0, .8, .5, 1.2, 154 / 255.0, 134 / 255.0, 108 / 255.0);
    drawCube(ourShader, VAO, identityMatrix, 0.9, 1.6, 4.35, 0, 0, 0, .01, .5, .8, 20 / 255.0, 20 / 255.0, 20 / 255.0);

    total = 15;
    unit = (.5 / (2 * total));
    for (int z = 0; z < total; z++) {
        drawCube(ourShader, VAO, identityMatrix, 0.9, 1.6 + (z + 1) * 2 * unit, 4.05, 0, 0, 0, .01, unit / 4, .3, 255 / 255.0, 255 / 255.0, 255 / 255.0);
    }

    // fan, 6, 5, 6
    //on = true;
    if (on) {
        r += 1;
    }
    else
    {
        r = 0.0f;
    }

    glm::mat4 translateMatrix, scaleMatrix, model, translateMatrixprev, rotateYMatrix;
    //fan stick
    translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0, 4.0, 3.0));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 0.9f, 0.1));
    model = translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(0.0, 0.0, 0.0, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // fan rotation
    translateMatrixprev = translateMatrix;
    glm::mat4 translateMatrix2, translateMatrixBack, test;

    translateMatrix2 = glm::translate(identityMatrix, glm::vec3(3.025, 4.0, 3.025));
    translateMatrixBack = glm::translate(identityMatrix, glm::vec3(-3.025, -4.0, -3.02));
    rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0, 1.0, 0.0));
    model = translateMatrixBack * rotateYMatrix * translateMatrix2;
    drawFan(VAO, ourShader, translateMatrix, rotateYMatrix);

  
    return 0;
}

void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 translateMatrix, glm::mat4 sm)
{
    glm::mat4 identityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
    glm::mat4 rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered, translateMatrixprev;
    glm::mat4 middleTranslate, leftBladeTranslate, frontBladeTranslate, rightBladeTranslate, backBladeTranslate;
    //fan middle part
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(-0.2, -1.5, -0.2));
    middleTranslate = glm::translate(identityMatrix, glm::vec3(-0.2, 0.0, -0.2));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, -0.1f, 0.5));
    model = translateMatrix * sm * middleTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.27, 0.12, 0.13, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    translateMatrixprev = translateMatrix;
    //left fan
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(0.0, -0.075, 0.0));
    leftBladeTranslate = glm::translate(identityMatrix, glm::vec3(-0.2, 0.0, -0.2));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(-2.0f, -0.1f, 0.5));
    model = translateMatrix * sm * leftBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.27,0.12,0.13, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //front fan
    //translateMatrix = translateMatrixprev * glm::translate(identityMatrix, glm::vec3(0.0, -0.075, 0.5));
    frontBladeTranslate = glm::translate(identityMatrix, glm::vec3(-0.2, 0.0, 0.3));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, -0.1f, 2.0));
    model = translateMatrix * sm * frontBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.27, 0.12, 0.13, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //right fan
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(0.5, 0.0, 0.0));
    rightBladeTranslate = glm::translate(identityMatrix, glm::vec3(0.25, 0.0, 0.25));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(2.0f, -0.1f, -0.5));
    model = translateMatrix * sm * rightBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.27, 0.12, 0.13, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //back fan
    //translateMatrix = translateMatrix * glm::translate(identityMatrix, glm::vec3(0.0, 0.0, -0.5));
    backBladeTranslate = glm::translate(identityMatrix, glm::vec3(0.25, 0.0, -0.25));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.5f, -0.1f, -2.0));
    model = translateMatrix * sm * backBladeTranslate * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("shapeColor", glm::vec4(0.27, 0.12, 0.13, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// Track whether the mouse button is pressed
bool isMousePressed = false;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            firstMouse = true;
            isMousePressed = true;
        }
        else if (action == GLFW_RELEASE) {
            isMousePressed = false;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    basic_camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    basic_camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// If you are confused with it's usage, then pass an identity matrix to it, and everything will be fine 
void drawCube(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
    float posX, float posY, float posZ,
    float rotX, float rotY, float rotZ,
    float scX, float scY, float scZ,
    float r, float g, float b) {


    int colorLoc = glGetUniformLocation(shaderProgram.ID, "shapeColor");
    glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
    glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(r, g, b)));

    shaderProgram.use();

    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    //modelCentered = glm::translate(model, glm::vec3(-0.25, -0.25, -0.25));

    shaderProgram.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}



void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) on = true;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) on = false;

    if (birdEye) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos.z -= birdEyeSpeed * deltaTime; // Move forward along Z
            target.z -= birdEyeSpeed * deltaTime;
            if (cameraPos.z <= -1.0) {
                cameraPos.z = -1.0;
            }
            if (target.z <= -4.0) {
                target.z = -4.0;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos.z += birdEyeSpeed * deltaTime; // Move backward along Z
            target.z += birdEyeSpeed * deltaTime;
            if (cameraPos.z >= 3.0) {
                cameraPos.z = 3.0;
            }
            if (target.z >= 0.0) {
                target.z = 0.0;
            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        birdEye= !birdEye;
    }

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) translate_Y += 0.01;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) translate_Y -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) translate_X += 0.01;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) translate_X -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) translate_Z += 0.01;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) translate_Z -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) scale_X += 0.01;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) scale_X -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) scale_Y += 0.01;
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) scale_Y -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) scale_Z += 0.01;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) scale_Z -= 0.01;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        rotateAngle_X += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        rotateAngle_Y += 1;
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        rotateAngle_Z += 1;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    {
        eyeX += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        eyeX -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        eyeZ += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        eyeZ -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyeY += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        eyeY -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        lookAtX += 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        lookAtX -= 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        lookAtY += 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        lookAtY -= 2.5 * deltaTime;
        basic_camera.lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
    }
}