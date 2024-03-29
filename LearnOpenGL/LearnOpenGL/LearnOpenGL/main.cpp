#include <Camera/camera.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ModelLoading/model.h>
#include <Shaders/shader.h>
#include <string>
#include <Textures/stb_image.h>

// Forward Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

unsigned int configureTexture(const char* texturePath);
void setSpotLight(Shader shader, Camera camera);
void setDirectionalLight(Shader shader);
void setPointLights(Shader shader);


// Screen setting constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* assimpVertShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/LearnOpenGL/assimpShader.vs";
const char* assimpFragShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/LearnOpenGL/assimpShader.fs";

const char* backpackObjectPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/Resources/Objects/backpack/backpack.obj";

const char* objVertShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/LearnOpenGL/objectShader.vs";
const char* objFragShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/LearnOpenGL/objectShader.fs";
const char* lightVertShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/LearnOpenGL/lightShader.vs";
const char* lightFragShaderPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/LearnOpenGL/lightShader.fs";
const char* diffuseMapPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/Resources/Textures/container2.png";
const char* specularMapPath = "C:/GraphicsProgramming/LearnOpenGL/LearnOpenGL/Resources/Textures/container2_specular.png";

// Time between current frame and last frame
float deltaTime = 0.0f;
// Time of last frame
float lastFrame = 0.0f;

// Camera / Mouse Positions
glm::vec3 cameraStartPos = glm::vec3(0.0f, 0.0f, 3.0f);
Camera camera(cameraStartPos);
float lastX = SCR_WIDTH / 2,
    lastY = SCR_HEIGHT / 2;
bool firstMouse = true;


glm::vec3 objectColor = glm::vec3(1.0f, 0.5f, 0.31f);
glm::vec3 cubePositions[] =
{
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

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

glm::vec3 pointLightColor = glm::vec3(1.0f, 0.72f, 0.11f);
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

// Toggle this to switch between using assimp model loading vs manually defined geometry
const bool useAssimp = true;

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
        cout << "Failed to create GLFW window" << endl;
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
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // Enable depth testing via the z-buffer
    glEnable(GL_DEPTH_TEST);

    // Set stbi to flip loaded textures on the y-axis before we load any models.
    stbi_set_flip_vertically_on_load(true);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if (useAssimp)
    {
        // --------------------- Assimp Model Rendering ---------------------

        Shader assimpShader(assimpVertShaderPath, assimpFragShaderPath);
        Model guitarModel(backpackObjectPath);

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
            // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Enable shader before setting uniforms
            assimpShader.useProgram();

            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            assimpShader.setMat4("projection", projection);
            assimpShader.setMat4("view", view);

            glm::mat4 model = glm::mat4(1.0f);
            // translate it down so it's at the center of the scene
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            // it's a bit too big for our scene, so scale it down
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            assimpShader.setMat4("model", model);

            // TODO: Set normal model here

            // Render!
            guitarModel.draw(assimpShader);


            // Swap color buffer once the new frame is ready
            glfwSwapBuffers(window);

            // Poll IO events (updates the window state and calls corresponding callback functions)
            glfwPollEvents();
        }

        guitarModel.freeResources();
        assimpShader.deleteProgram();
    }
    else 
    {
        // --------------------- Container & Lighting Rendering ---------------------

        // Create shader program
        Shader objectShader(objVertShaderPath, objFragShaderPath);
        Shader lightShader(lightVertShaderPath, lightFragShaderPath);

        // Create and load textures
        unsigned int diffuseMap = configureTexture(diffuseMapPath),
            specularMap = configureTexture(specularMapPath);

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
            // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render Cubes
            objectShader.useProgram();
            glBindVertexArray(objectVAO);

            // Bind Object Textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuseMap);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, specularMap);

            // Material Properties
            objectShader.setFloat("material.shininess", 64.0f);

            // Light Properties
            setSpotLight(objectShader, camera);
            setDirectionalLight(objectShader);
            setPointLights(objectShader);

            objectShader.setVec3("viewPos", camera.Position);

            // Construct our object's transformation matrices
            // note that we're translating the scene in the reverse direction of where we want to move
            glm::mat4 view = camera.GetViewMatrix();
            objectShader.setMat4("view", view);

            glm::mat4 projection;
            projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
            objectShader.setMat4("projection", projection);

            for (unsigned int i = 0; i < 10; i++)
            {
                // Calculate the model matrix for each object and pass it to shader before drawing
                glm::mat4 objectModel = glm::mat4(1.0f);
                objectModel = glm::translate(objectModel, cubePositions[i]);
                float angle = 20.0f * i;
                objectModel = glm::rotate(objectModel, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                objectShader.setMat4("model", objectModel);

                // Prepare the normal matrix for the objects normal vectors (used to transform normals into world space
                // without suffering from non-uniform scaling distortions)
                glm::mat4 normalModel = glm::inverse(objectModel);
                normalModel = glm::transpose(normalModel);
                objectShader.setMat3("normalModel", glm::mat3(normalModel));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

            // Render pointlights
            lightShader.useProgram();
            glBindVertexArray(lightVAO);

            lightShader.setVec3("lightColor", pointLightColor);

            // Set up the light's transform matrices
            lightShader.setMat4("view", view);
            lightShader.setMat4("projection", projection);

            for (unsigned int i = 0; i < 4; i++)
            {
                glm::mat4 lightModel = glm::mat4(1.0f);
                // Because the lightModel will be acting upon object-space vertex coordinates we need to translate to the world space position
                // of the light
                lightModel = glm::translate(lightModel, glm::vec3(pointLightPositions[i]));
                lightModel = glm::scale(lightModel, glm::vec3(0.2f));
                lightShader.setMat4("model", lightModel);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }

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
    }

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

        stbi_image_free(data);
        return texture;
    }
    else
    {
        cout << "Failed to load texture" << endl;

        stbi_image_free(data);
        return 0;
    }
}


void setSpotLight(Shader shader, Camera camera)
{
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float outerCutOff = 10.0f;
    float innerCutOff = 8.0f;
    glm::vec3 diffuseLight = lightColor * glm::vec3(0.8f);
    glm::vec3 ambientLight = diffuseLight * glm::vec3(0.05f);

    shader.setVec3("spotLight.position", camera.Position);
    shader.setVec3("spotLight.direction", camera.Front);
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(outerCutOff)));
    shader.setFloat("spotLight.innerCutOff", glm::cos(glm::radians(innerCutOff)));
    shader.setVec3("spotLight.ambient", ambientLight);
    shader.setVec3("spotLight.diffuse", diffuseLight);
    shader.setVec3("spotLight.specular", lightColor);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);
}


void setPointLights(Shader shader)
{
    glm::vec3 lightColor = pointLightColor;
    glm::vec3 diffuseLight = lightColor * glm::vec3(0.2f);
    glm::vec3 ambientLight = diffuseLight * glm::vec3(0.05f);

    for (unsigned int i = 0; i < 4; i++)
    {
        string prefix = "pointLights[" + to_string(i) + "].";
        shader.setVec3(prefix + "position", pointLightPositions[i]);
        shader.setVec3(prefix + "ambient", ambientLight);
        shader.setVec3(prefix + "diffuse", diffuseLight);
        shader.setVec3(prefix + "specular", lightColor);
        shader.setFloat(prefix + "constant", 1.0f);
        shader.setFloat(prefix + "linear", 0.09f);
        shader.setFloat(prefix + "quadratic", 0.032f);
    }
}


void setDirectionalLight(Shader shader)
{
    shader.setVec3("directionalLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader.setVec3("directionalLight.ambient", glm::vec3(0.02f));
    shader.setVec3("directionalLight.diffuse", glm::vec3(0.06f));
    shader.setVec3("directionalLight.specular", glm::vec3(0.2f));
}