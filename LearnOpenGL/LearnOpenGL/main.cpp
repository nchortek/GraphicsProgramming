#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

// Forward Declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// Settings constants
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// TODO: These should probably be in their own shader file
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";


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

    // Set callbacks after window is created but before we initiate the render loop
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD (loads all OpenGL function pointers)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set up vertex shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // glCreateShader returns 0 if an error occurred
    if (!vertexShader)
    {
        std::cout << "Failed to create vertex shader" << std::endl;
        return -1;
    }

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Set up fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    if (!fragmentShader)
    {
        std::cout << "Failed to create fragment shader" << std::endl;
        return -1;
    }

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    if (!shaderProgram)
    {
        std::cout << "Failed to create shader program" << std::endl;
        return -1;
    }

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader program errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }

    // Our shaders are now part of our shader program, so we delete the original shaders to avoid
    // allocating twice the memory for them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // TODO: Should these be placed in dedicated mesh files?
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };

    // Create a Vertex Array Object and Vertex Buffer Object
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind to VAO
    glBindVertexArray(VAO);

    // Bind to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Because VBO is the currently bound buffer, any buffer calls made to
    // GL_ARRAY_BUFFER will now configure VBO specifically.
    // 
    // The following glBufferData call copies our vertex data into the VBO's memory (on the GPU!)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Tell OpenGL how to connect our vertex data to the vertex shader
    // Params:
    // 0 - corresponds to the location of our attribute, aPos, in the vertex shader
    // 3 - the size of the attribute (aPos is a vec3)
    // GL_FLOAT - vec* is comprised of float values
    // GL_FALSE - whether we want our input data to be normalized (we don't)
    // 3 * sizeof(float) - this is our stride (the space between consecutive vertex attributes)
    // (void*)0 - the offset of where the position data begins in the buffer
    //
    // NOTE: Our vertex array is tightly packed, so in this case we could have passed 0 for the stride value.
    // OpenGL can determine the stride itself if the input is tightly packed (there is no space between the next vertex attribute value)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Input here is the location/index of the vertex attribute to be enabled
    glEnableVertexAttribArray(0);

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
        glClear(GL_COLOR_BUFFER_BIT);

        // Ensure that subsequent shader and rendering calls use our shader program
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Swap color buffer once the new frame is ready
        glfwSwapBuffers(window);

        // Poll IO events (updates the window state and calls corresponding callback functions)
        glfwPollEvents();
    }

    // Memory clean-up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

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
/// Handle specific key presses
/// </summary>
/// <param name="window"></param>
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}