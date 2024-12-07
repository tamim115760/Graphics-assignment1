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
#include "pointLight.h"


#include <iostream>

using namespace std;

#define PI 3.14159265359

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void drawFan(unsigned int VAO, Shader lightingShader, glm::mat4 translateMatrix, glm::mat4 sm);
int drawAll(Shader lightingShader, unsigned int VAO, glm::mat4 parentTrans);
void drawCube1(unsigned int& VAO, Shader& lightingShader, glm::mat4 model, glm::vec3 color);

void drawCube(
    Shader lightingShader, unsigned int VAO, glm::mat4 parentTrans,
    float posX = 0.0, float posY = 0.0, float posz = 0.0,
    float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0,
    float scX = 1.0, float scY = 1.0, float scZ = 1.0,
    float r = 0.0, float g = 0.0, float b = 0.0);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//screen
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float aspectRatio = 4.0f / 3.0f;


//camera
float eyeX = 1.5, eyeY = 3.8, eyeZ = 10.0;
float lookAtX = 4.0, lookAtY = 4.0, lookAtZ = 6.0;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

bool on = false;


bool birdEye = false;
glm::vec3 cameraPos(1.0f, 2.5f, 3.0f);
glm::vec3 target(1.0f, 0.0f, 0.0f);
float birdEyeSpeed = 1.0f;

//rotation around a point
float theta = 0.0f; // Angle around the Y-axis
float radius = 2.0f;

//directional light
bool directionLightOn = true;
bool directionalAmbient = true;
bool directionalDiffuse = true;
bool directionalSpecular = true;

//spot light
bool spotLightOn = true;

//point light
bool point1 = true;
bool point2 = true;

//custom projection matrix
float fov = glm::radians(camera.Zoom);
float aspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;
float near = 0.1f;
float far = 100.0f;
float tanHalfFOV = tan(fov / 2.0f);

//positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(2.0f,  3.0f,  2.0f),
    glm::vec3(2.0f,  3.0f,  5.0f),
};

PointLight pointlight1(
    pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z,       // position
    0.2f, 0.2f, 0.2f,       //ambient
    0.8f, 0.8f, 0.8f,       //diffuse
    1.0f, 1.0f, 1.0f,       //specular
    1.0f,       //k_c
    0.09f,      //k_l
    0.032f,     //k_q
    1       //light number
);

PointLight pointlight2(
    pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z,
    0.2f, 0.2f, 0.2f,
    0.8f, 0.8f, 0.8f,
    1.0f, 1.0f, 1.0f,
    1.0f,
    0.09f,
    0.032f,
    2
);


int initGlfw(GLFWwindow*& window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // glfw window creation
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment-3(1907060)", NULL, NULL);
    if (window == NULL) { cout << "Failed to create GLFW window" << endl; glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
   // glfwSetCursorPosCallback(window, mouse_callback);
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

void initBinding(unsigned int& VAO, unsigned int& VBO, unsigned int& EBO, Shader& lightingShader, float* cube_vertices, int verticesSize, unsigned int* cube_indices, int indicesSize) {
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

    //normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    lightingShader.use();
    lightingShader.setVec3("viewPos", camera.Position);

}


int main()
{
    GLFWwindow* window = nullptr;
    if (initGlfw(window)) return -1;

    glEnable(GL_DEPTH_TEST);


    //build and compile our shader program
    Shader lightingShader("vertexShaderForGouraudShading.vs", "fragmentShaderForGouraudShading.fs");
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");
    Shader constantShader("vertexShader.vs", "fragmentShaderV2.fs");
    glm::vec3 color;


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cube_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f
    };
    unsigned int cube_indices[] = {
        0, 3, 2,
        2, 1, 0,

        4, 5, 7,
        7, 6, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };

    unsigned int VBO, VAO, EBO;
    initBinding(VAO, VBO, EBO, ourShader, cube_vertices, sizeof(cube_vertices), cube_indices, sizeof(cube_indices));

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    //note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    float r = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //pointlight setup
        pointlight1.setUpPointLight(lightingShader);
        pointlight2.setUpPointLight(lightingShader);

  

         //spot light set up
        lightingShader.setVec3("spotLight.position", 4.0f, 4.5f, 6.0f);
        lightingShader.setVec3("spotLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setVec3("spotLight.ambient", 0.5f, 0.5f, 0.5f);
        lightingShader.setVec3("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.k_c", 1.0f);
        lightingShader.setFloat("spotLight.k_l", 0.09);
        lightingShader.setFloat("spotLight.k_q", 0.032);
        lightingShader.setFloat("spotLight.cos_theta", glm::cos(glm::radians(40.0f)));
        lightingShader.setBool("spotLightOn", spotLightOn);


        //directional light set up
        lightingShader.setVec3("directionalLight.direction", 0.0f, -1.0f, 0.0f);
        lightingShader.setVec3("directionalLight.ambient", 0.1f, 0.1f, 0.1f);
        lightingShader.setVec3("directionalLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("directionalLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setBool("directionLightOn", directionLightOn);

        //handle for changes in directional light directly from shedder
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("ambientLight", !directionalAmbient);
                directionalAmbient = !directionalAmbient;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("diffuseLight", !directionalDiffuse);
                directionalDiffuse = !directionalDiffuse;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
            if (directionLightOn) {
                lightingShader.setBool("specularLight", !directionalSpecular);
                directionalSpecular = !directionalSpecular;
            }
        }



        glm::mat4 projection(0.0f);
        projection[0][0] = 1.0f / (aspect * tanHalfFOV);
        projection[1][1] = 1.0f / tanHalfFOV;
        projection[2][2] = -(far + near) / (far - near);
        projection[2][3] = -1.0f;
        projection[3][2] = -(2.0f * far * near) / (far - near);
 
        lightingShader.setMat4("projection", projection);


        // camera/view transformation
        glm::mat4 view;

        if (birdEye) {
            glm::vec3 up(0.0f, 1.0f, 0.0f);
            view = glm::lookAt(cameraPos, target, up);
        }
        else {
            view = camera.GetViewMatrix();
        }

        //glm::mat4 view = basic_camera.createViewMatrix();
        lightingShader.setMat4("view", view);
        //constantShader.setMat4("view", view);
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered, translateMatrixprev;
        translateMatrix = identityMatrix;
        glm::vec3 color;

        lightingShader.setVec3("material.emissive", glm::vec3(0.0f, 0.0f, 0.0f));
        // drawing
        drawAll(lightingShader, VAO, identityMatrix);
        //light holder 1 with emissive material property
        translateMatrix = glm::translate(identityMatrix, glm::vec3(2.08f, 3.5f, 2.08f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.04f, -0.5f, 0.04f));
        model = translateMatrix * scaleMatrix;
        color = glm::vec3(0.1f, 0.0f, 0.0f);

        lightingShader.setVec3("material.ambient", color);
        lightingShader.setVec3("material.diffuse", color);
        lightingShader.setVec3("material.specular", color);
        lightingShader.setVec3("material.emissive", color);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        //light holder 2 with emissive material property
        translateMatrix = glm::translate(identityMatrix, glm::vec3(2.08f, 3.5f, 5.08f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.04f, -0.5f, 0.04f));
        model = translateMatrix * scaleMatrix;
        color = glm::vec3(0.2f, 0.3f, 0.1f);

        lightingShader.setVec3("material.ambient", color);
        lightingShader.setVec3("material.diffuse", color);
        lightingShader.setVec3("material.specular", color);
        lightingShader.setVec3("material.emissive", color);
        lightingShader.setFloat("material.shininess", 32.0f);

        lightingShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //draw the lamp object(s)
        ourShader.use();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);
        glBindVertexArray(lightCubeVAO);

        //we now draw as many light bulbs as we have point lights.
        
        for (unsigned int i = 0; i < 2; i++)
        {
            translateMatrix = glm::translate(identityMatrix, pointLightPositions[i]);
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -0.2f, 0.2f));
            model = translateMatrix * scaleMatrix;
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }
   

        
        // drawing above

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    //glfw terminate, clearing all previously allocated GLFW resources
    glfwTerminate();
    return 0;
    
}

float r = 0.0f;

int drawAll(Shader lightingShader, unsigned int VAO, glm::mat4 identityMatrix) {
    // floor
   drawCube(lightingShader, VAO, identityMatrix, 0, 0, 0, 0, 0, 0, 6, .1, 6, 0.76, 0.57, 0.37);
   
  
      // ceiling
    drawCube(lightingShader, VAO, identityMatrix, 0, 5, 0, 0, 0, 0, 6, .1, 6, 1.0, 0.99, 0.82);

    // right wall
    drawCube(lightingShader, VAO, identityMatrix, 0, .1, -.05, 0, 0, 0, 6, 5, .1, 0.678, 0.847, 0.902);
    // left wall
    drawCube(lightingShader, VAO, identityMatrix, -0.05, .1, 0, 0, 0, 0, .1, 5, 6, 0.678, 0.847, 0.902);

    // right shelf
    drawCube(lightingShader, VAO, identityMatrix, 0.1, 1.5, .1, 0, 0, 0, 4, .1, 1.2, 1.0, 0.99, 0.82);
    // left shelf
    drawCube(lightingShader, VAO, identityMatrix, 0.1, 1.5, .1, 0, 0, 0, 1.2, .1, 5.9, 1.0, 0.99, 0.82);

    // left wall shelf
    int total = 4;
    for (int i = 0; i < total; i++) {
        float gap = (1 / 10.0);
        float width = .8;

        drawCube(lightingShader, VAO, identityMatrix, 0, 2.5, (i * width + i * gap),
            0, 0, 0, .6, 1, width, 0.70, 0.45, 0.56
        );

        if (i == total - 1) continue;
        drawCube(lightingShader, VAO, identityMatrix, 0, 2.5, (i * width + i * gap) + width,
            0, 0, 0, .6, 1, gap, 1.0, 0.99, 0.82
        );
    }
    // right wall shelf
    drawCube(lightingShader, VAO, identityMatrix, .65, 2.5, 0, 0, 0, 0, .8, 1, .6, 0.70, 0.45, 0.56);
    // right wall shelf white
    drawCube(lightingShader, VAO, identityMatrix, .65, 2.55, .6, 0, 0, 0, .7, .9, .05, 0.99, 0.99, 0.99);

    // right wall window?
    drawCube(lightingShader, VAO, identityMatrix, 2, 2, .1, 0, 0, 0, 2, 1.5, .1, 0.70, 0.45, 0.56);
    // right wall window? white
    drawCube(lightingShader, VAO, identityMatrix, 2.05, 2.05, .15, 0, 0, 0, .9, 1.4, .1, 0.99, 0.99, 0.99);
    drawCube(lightingShader, VAO, identityMatrix, 3.05, 2.05, .15, 0, 0, 0, .9, 1.4, .1, 0.99, 0.99, 0.99);

    // lower shelf left
    total = 6;
    for (int i = 0; i < total; i++) {
        float gap = (1 / 10.0);
        float width = .8;

        drawCube(lightingShader, VAO, identityMatrix, 0, 0, .5 + (i * width + i * gap),
            0, 0, 0, 1.2, 1.5, width, 0.70, 0.45, 0.56
        );

        if (i == total - 1) continue;
        drawCube(lightingShader, VAO, identityMatrix, 0, 0, .5 + (i * width + i * gap) + width,
            0, 0, 0, 1.2, 1.5, gap, 0.99, 0.99, 0.99
        );
    }

    // right wall shelf bottom
    total = 4;
    for (int i = 0; i < total; i++) {
        float gap = (1 / 10.0);
        float width = .6;

        drawCube(lightingShader, VAO, identityMatrix, 1.2 + (i * width + i * gap), 0, 0,
            0, 0, 0, width, 1.5, 1.2, 0.70, 0.45, 0.56
        );

        if (i == total - 1) continue;
        drawCube(lightingShader, VAO, identityMatrix, 1.2 + (i * width + i * gap + width), 0, 0,
            0, 0, 0, gap, 1.5, 1.2, 0.99, 0.99, 0.99
        );
    }

    // refrigerator
    drawCube(lightingShader, VAO, identityMatrix, 4, 0, 0, 0, 0, 0, 2, 3.5, 1.5, 0.70, 0.45, 0.56);
    drawCube(lightingShader, VAO, identityMatrix, 4.05, 0, 1.5, 0, 0, 0, .95, 3.5, .05, 0.99, 0.99, 0.99);
    drawCube(lightingShader, VAO, identityMatrix, 5.05, 0, 1.5, 0, 0, 0, .95, 3.5, .05, 0.99, 0.99, 0.99);
    // refrigerator handle
    drawCube(lightingShader, VAO, identityMatrix, 4.9, 1.5, 1.55, 0, 0, 0, .05, 1.1, .05, 20 / 255.0, 20 / 255.0, 20 / 255.0);
    drawCube(lightingShader, VAO, identityMatrix, 5.1, 1.5, 1.55, 0, 0, 0, .05, 1.1, .05, 20 / 255.0, 20 / 255.0, 20 / 255.0);

    // table-top
    drawCube(lightingShader, VAO, identityMatrix, 3, 1.5, 4, 0, 0, 0, 2, .1, 1.5, 0.70, 0.45, 0.56);
    // left top leg
    drawCube(lightingShader, VAO, identityMatrix, 3, 0, 4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);
    // right top leg
    drawCube(lightingShader, VAO, identityMatrix, 4.9, 0, 4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);
    // left bottom leg
    drawCube(lightingShader, VAO, identityMatrix, 3, 0, 5.4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);
    // right bottom leg
    drawCube(lightingShader, VAO, identityMatrix, 4.9, 0, 5.4, 0, 0, 0, .1, 1.5, .1, 0.99, 0.99, 0.99);

    for (int z = 0; z < 2; z++) {
        for (int x = 0; x < 2; x++) {
            float width = 0.5;
            float gap = 0.4;
            int zz = (z == 0) ? 1 : 0;

            // chairs
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.2, .8, (z * 2 + 3), 0, 0, 0, .5, .1, .5, 0.70, 0.10, 0.17);
            // left top leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.2, 0, (z * 2 + 3), 0, 0, 0, .1, (.8 + zz * .7), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);


            // left left leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.33, 0.9, (z * 2 + 3 + .5 * z - z * .1), 0, 0, 0, .05, .6, .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
            // left mid leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.44, 0.9, (z * 2 + 3 + .5 * z - z * .1), 0, 0, 0, .05, .6, .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
            // left right leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.55, 0.9, (z * 2 + 3 + .5 * z - z * .1), 0, 0, 0, .05, .6, .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);


            // right top leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.6, 0, (z * 2 + 3), 0, 0, 0, .1, (.8 + .7 * zz), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);

            zz = !zz;
            // left bottom leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.2, 0, (z * 2 + 3.4), 0, 0, 0, .1, (.8 + .7 * zz), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
            // right bottom leg
            drawCube(lightingShader, VAO, identityMatrix, (width + gap) * x + 3.6, 0, (z * 2 + 3.4), 0, 0, 0, .1, (.8 + .7 * zz), .1, 75 / 255.0, 62 / 255.0, 53 / 255.0);
        }
    }

    //sink
    drawCube(lightingShader, VAO, identityMatrix, 2, 1.6, .1, 0, 0, 0, 1.5, .02, 1.3, 24 / 255.0, 21 / 255.0, 22 / 255.0);
    total = 20;
    float unit = (1.3 / (2 * total));
    for (int z = 0; z < total; z++) {
        drawCube(lightingShader, VAO, identityMatrix, 2, 1.63, (z + 1) * 2 * unit, 0, 0, 0, 1, .01, unit / 2, 60 / 255.0, 60 / 255.0, 60 / 255.0);
    }




    //// the real tap
    drawCube(lightingShader, VAO, identityMatrix, 3.2, 1.6, .3, 0, 0, 0, .05, .5, .05, 200 / 255.0, 200 / 255.0, 200 / 255.0);
    drawCube(lightingShader, VAO, identityMatrix, 3.2, 2.1, .3, 0, 0, 0, .05, .05, .3, 200 / 255.0, 200 / 255.0, 200 / 255.0);
    drawCube(lightingShader, VAO, identityMatrix, 3.2, 2.0, .6, 0, 0, 0, .05, .2, .05, 200 / 255.0, 200 / 255.0, 200 / 255.0);

    // oven
    drawCube(lightingShader, VAO, identityMatrix, 0.1, 1.6, 4, 0, 0, 0, .8, .5, 1.2, 154 / 255.0, 134 / 255.0, 108 / 255.0);
    drawCube(lightingShader, VAO, identityMatrix, 0.9, 1.6, 4.35, 0, 0, 0, .01, .5, .8, 20 / 255.0, 20 / 255.0, 20 / 255.0);

    total = 15;
    unit = (.5 / (2 * total));
    for (int z = 0; z < total; z++) {
        drawCube(lightingShader, VAO, identityMatrix, 0.9, 1.6 + (z + 1) * 2 * unit, 4.05, 0, 0, 0, .01, unit / 4, .3, 255 / 255.0, 255 / 255.0, 255 / 255.0);
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
    lightingShader.setMat4("model", model);
    lightingShader.setVec4("color", glm::vec4(0.0, 0.0, 0.0, 1.0));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    // fan rotation
    translateMatrixprev = translateMatrix;
    glm::mat4 translateMatrix2, translateMatrixBack, test;

    translateMatrix2 = glm::translate(identityMatrix, glm::vec3(3.025, 4.0, 3.025));
    translateMatrixBack = glm::translate(identityMatrix, glm::vec3(-3.025, -4.0, -3.02));
    rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0, 1.0, 0.0));
    model = translateMatrixBack * rotateYMatrix * translateMatrix2;
    drawFan(VAO, lightingShader, translateMatrix, rotateYMatrix);


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
    ourShader.setVec4("shapeColor", glm::vec4(0.27, 0.12, 0.13, 1.0));
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

void drawCube1(unsigned int& VAO, Shader& lightingShader, glm::mat4 model, glm::vec3 color)
{
    //use the shadder
    lightingShader.use();
    //color = glm::vec3(0.624f, 0.416f, 0.310f);

    //define lighting properties
    lightingShader.setVec3("material.ambient", color);
    lightingShader.setVec3("material.diffuse", color);
    lightingShader.setVec3("material.specular", color);
    lightingShader.setFloat("material.shininess", 32.0f);

    lightingShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}



// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //make sure the viewport matches the new window dimensions; note that width and height will be significantly larger than specified on retina displays.
    float aspectRatio = 4.0f / 3.0f;
    int viewWidth, viewHeight;

    if (width / (float)height > aspectRatio) {
        //Window is too wide, fit height and adjust width
        viewHeight = height;
        viewWidth = (int)(height * aspectRatio);
    }
    else {
        //Window is too tall, fit width and adjust height
        viewWidth = width;
        viewHeight = (int)(width / aspectRatio);
    }

    //Center the viewport
    int xOffset = (width - viewWidth) / 2;
    int yOffset = (height - viewHeight) / 2;

    glViewport(xOffset, yOffset, viewWidth, viewHeight);
    //glViewport(0, 0, width, height);
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
    float yoffset = lastY - ypos;       //reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


// If you are confused with it's usage, then pass an identity matrix to it, and everything will be fine 
void drawCube(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
    float posX, float posY, float posZ,
    float rotX, float rotY, float rotZ,
    float scX, float scY, float scZ,
    float r, float g, float b) {


    //int colorLoc = glGetUniformLocation(shaderProgram.ID, "shapeColor");
    //glUniform3f(colorLoc, 1.0f, 0.0f, 1.0f);
    //glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(r, g, b)));

    shaderProgram.use();
    glm::vec3 color = glm::vec3(r, g, b);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    //modelCentered = glm::translate(model, glm::vec3(-0.25, -0.25, -0.25));

    //define lighting properties
    shaderProgram.setVec3("material.ambient", color);
    shaderProgram.setVec3("material.diffuse", color);
    shaderProgram.setVec3("material.specular", color);
    shaderProgram.setFloat("material.shininess", 32.0f);

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




    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(P_UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(P_DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(Y_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(Y_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(R_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!birdEye)
            camera.ProcessKeyboard(R_RIGHT, deltaTime);
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
  /*  if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        eyeZ += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }*/
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        eyeZ -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }
  /*  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyeY += 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }*/
   /* if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        eyeY -= 2.5 * deltaTime;
        basic_camera.eye = glm::vec3(eyeX, eyeY, eyeZ);
    }*/

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        birdEye = !birdEye;
    }

    if (birdEye) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos.z -= birdEyeSpeed * deltaTime; // Move forward along Z
            target.z -= birdEyeSpeed * deltaTime;
            if (cameraPos.z <= 2.0) {
                cameraPos.z = 2.0;
            }
            if (target.z <= -4.0) {
                target.z = -4.0;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos.z += birdEyeSpeed * deltaTime; // Move backward along Z
            target.z += birdEyeSpeed * deltaTime;
            if (cameraPos.z >= 13.5) {
                cameraPos.z = 13.5;
            }
            if (target.z >= 7.5) {
                target.z = 7.5;
            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!birdEye) {
            theta += 0.01f;
            camera.Position.x = lookAtX + radius * sin(theta);
            camera.Position.y = lookAtY;
            camera.Position.z = lookAtZ + radius * cos(theta);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        directionLightOn = !directionLightOn;
    }

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        spotLightOn = !spotLightOn;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        if (pointlight1.ambientOn > 0 && pointlight1.diffuseOn > 0 && pointlight1.specularOn > 0) {
            pointlight1.turnOff();
            point1 = false;
        }
        else {
            pointlight1.turnOn();
            point1 = true;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        if (pointlight2.ambientOn > 0 && pointlight2.diffuseOn > 0 && pointlight2.specularOn > 0) {
            pointlight2.turnOff();
            point2 = false;
        }
        else {
            pointlight2.turnOn();
            point2 = true;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (pointlight1.ambientOn > 0 || pointlight2.ambientOn > 0) {
            if (point1)
                pointlight1.turnAmbientOff();
            if (point2)
                pointlight2.turnAmbientOff();
        }
        else {
            if (point1)
                pointlight1.turnAmbientOn();
            if (point2)
                pointlight2.turnAmbientOn();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (pointlight1.diffuseOn > 0 || pointlight2.diffuseOn > 0) {
            if (point1)
                pointlight1.turnDiffuseOff();
            if (point2)
                pointlight2.turnDiffuseOff();
        }
        else {
            if (point1)
                pointlight1.turnDiffuseOn();
            if (point2)
                pointlight2.turnDiffuseOn();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        if (pointlight1.specularOn > 0 || pointlight2.specularOn > 0) {
            if (point1)
                pointlight1.turnSpecularOff();
            if (point2)
                pointlight2.turnSpecularOff();
        }
        else {
            if (point1)
                pointlight1.turnSpecularOn();
            if (point2)
                pointlight2.turnSpecularOn();
        }
    }
}
