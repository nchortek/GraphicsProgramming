#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shaders/shader.h>
#include <Camera/camera.h>
#include <Textures/stb_image.h>

#include <iostream>

// Forward Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int configureTexture(const char* texturePath);

// Screen setting constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* objVertShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/objectShader.vs";
const char* objFragShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/objectShader.fs";
const char* lightVertShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/lightShader.vs";
const char* lightFragShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/lightShader.fs";
const char* diffuseMapPath = "C:/GraphicsProgramming/LearnOpenGL/Textures/container2.png";
const char* specularMapPath = "C:/GraphicsProgramming/LearnOpenGL/Textures/container2_specular.png";
//const char* texturePath = "C:/GraphicsProgramming/LearnOpenGL/Textures/container.jpg";
//const char* texturePath = "C:/GraphicsProgramming/LearnOpenGL/Textures/awesomeface.png";

// Time between current frame and last frame
float deltaTime = 0.0f;
// Time of last frame
float lastFrame = 0.0f;

// Camera / Mouse Positions
glm::vec3 cameraStartPos = glm::vec3(0.0f, 1.0f, 5.0f);
Camera camera(cameraStartPos);
float lastX = SCR_WIDTH / 2,
    lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

// Camera speed and orientation
const float cameraSpeed = 2.5f;
glm::vec3 cameraPos = cameraStartPos,
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f),
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.31f),
    objectPosition = glm::vec3(0.0f, 0.0f, 0.0f),
    lightPosition = glm::vec3(0.0f, 1.0f, 0.0f);

int main()
{
    // Init glfw, setting to OpenGL 3.3 and the core-profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the context of our new window to be current for this thread
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set callbacks after window is created but before we initiate the render loop
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Initialize GLAD (loads all OpenGL function pointers)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enable depth testing via the z-buffer
    glEnable(GL_DEPTH_TEST);

    // Create shader program
    Shader objectShader(objVertShaderPath, objFragShaderPath);
    Shader lightShader(lightVertShaderPath, lightFragShaderPath);

    // TODO: Should these be placed in dedicated mesh files?
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // Create and load textures
    unsigned int diffuseMap = configureTexture(diffuseMapPath);
    unsigned int specularMap = configureTexture(specularMapPath);

    // Create a VAO and VBO
    unsigned int objectVAO, lightVAO, VBO;
    glGenVertexArrays(1, &objectVAO);
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(objectVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Set texture uniforms
    objectShader.useProgram();
    objectShader.setInt("material.diffuseMap", 0);
    objectShader.setInt("material.specularMap", 1);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Check for specific key presses
        processInput(window);

        // Execute render commands
        // 
        // (Currently just clearing the previous frame, displaying the specified color)
        // 
        // glClearColor is a state-setting function, and glClear is a state-using function in that
        // it uses the current state to retrieve the clearing color from.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render 
        objectShader.useProgram();

        // Material Properties
        objectShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        objectShader.setFloat("material.shininess", 64.0f);

        // Light Properties
        float curTime = (float)glfwGetTime();
        float positionVariance = curTime / 2.0f;
        lightPosition.x = 2.0f * sinf(positionVariance);
        lightPosition.z = 2.0f * cosf(positionVariance);

        glm::vec3 lightColor;
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        // divide the current time by 4 to lengthen the sin period,
        // slowing down the color's rate of change
        // float colorVariance = curTime / 4.0f;
        // lightColor.x = sinf(colorVariance * 2.0f);
        // lightColor.y = sinf(colorVariance * 0.7f);
        // lightColor.z = sinf(colorVariance * 1.3f);

        glm::vec3 diffuseLight = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientLight = diffuseLight * glm::vec3(0.2f);
        objectShader.setVec3("light.position", lightPosition);
        objectShader.setVec3("light.ambient", ambientLight);
        objectShader.setVec3("light.diffuse", diffuseLight);
        objectShader.setVec3("light.specular", lightColor);

        objectShader.setVec3("viewPos", camera.Position);

        // Construct our object's transformation matrices
        // note that we're translating the scene in the reverse direction of where we want to move
        glm::mat4 view = camera.GetViewMatrix();
        objectShader.setMat4("view", view);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
        objectShader.setMat4("projection", projection);

        // Calculate the model matrix for each object and pass it to shader before drawing
        glm::mat4 objectModel = glm::mat4(1.0f);
        objectModel = glm::translate(objectModel, objectPosition);
        objectShader.setMat4("model", objectModel);

        // Prepare the normal matrix for the objects normal vectors (used to transform normals into world space
        // without suffering from non-uniform scaling distortions)
        glm::mat4 normalModel = glm::inverse(objectModel);
        normalModel = glm::transpose(normalModel);
        objectShader.setMat3("normalModel", glm::mat3(normalModel));

        // Bind Object Textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        glBindVertexArray(objectVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Render light
        lightShader.useProgram();
        lightShader.setVec3("lightColor", lightColor);

        // Set up the light's transform matrices
        lightShader.setMat4("view", view);
        lightShader.setMat4("projection", projection);

        glm::mat4 lightModel = glm::mat4(1.0f);
        // Because the lightModel will be acting upon object-space vertex coordinates we need to translate to the world space position
        // of the light
        lightModel = glm::translate(lightModel, lightPosition);
        lightModel = glm::scale(lightModel, glm::vec3(0.2f));
        lightShader.setMat4("model", lightModel);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Swap color buffer once the new frame is ready
        glfwSwapBuffers(window);

        // Poll IO events (updates the window state and calls corresponding callback functions)
        glfwPollEvents();
    }

    // Memory clean-up
    glDeleteVertexArrays(1, &objectVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    objectShader.deleteProgram();
    lightShader.deleteProgram();

    glfwTerminate();
    return 0;
}

/// <summary>
/// Ensures the viewport dimensions match those of the application window if/when it is resized
/// </summary>
/// <param name="window"></param>
/// <param name="width"></param>
/// <param name="height"></param>
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/// <summary>
/// Calculates yaw and pitch for creating a direction vector given the current and previous mouse positions
/// </summary>
/// <param name="window"></param>
/// <param name="xpos"></param>
/// <param name="ypos"></param>
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

/// <summary>
/// Adjusts the field of view based on user scroll-wheel input
/// </summary>
/// <param name="window"></param>
/// <param name="xoffset"></param>
/// <param name="yoffset"></param>
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

/// <summary>
/// Handle specific key presses
/// </summary>
/// <param name="window"></param>
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // What about hardware adjusted velocity?
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
}


unsigned int configureTexture(const char* texturePath)
{
    unsigned int texture;
    int width, height, nrChannels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

    if (data)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // TODO: Again, is this a proper initialization? We need a good default to avoid the unitialized memory issue
        GLenum format = 0;
        if (nrChannels == 1)
        {
            format = GL_RED;
        }
        else if (nrChannels == 3)
        {
            format = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            format = GL_RGBA;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return 0;
    }

    stbi_image_free(data);

    return texture;
}