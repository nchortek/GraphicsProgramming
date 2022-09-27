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

// Orange
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

// Yellow
const char* fragmentShaderSource2 = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
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

    // Set up fragment shader (orange)
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

    // Set up fragment shader 2 (yellow)
    unsigned int fragmentShader2;
    fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);

    if (!fragmentShader2)
    {
        std::cout << "Failed to create fragment shader" << std::endl;
        return -1;
    }

    glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, NULL);
    glCompileShader(fragmentShader2);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader2, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader2, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT-2::COMPILATION_FAILED\n" << infoLog << std::endl;
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

    unsigned int shaderProgram2;
    shaderProgram2 = glCreateProgram();

    if (!shaderProgram2)
    {
        std::cout << "Failed to create shader program" << std::endl;
        return -1;
    }

    glAttachShader(shaderProgram2, vertexShader);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);

    // Check for shader program errors
    glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
    }

    // Our shaders are now part of our shader program, so we delete the original shaders to avoid
    // allocating twice the memory for them
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShader2);

    // TODO: Should these be placed in dedicated mesh files?
    float vertices1[] = {
        0.1f,  0.6f, 0.0f,
        0.4f, 0.6f, 0.0f,
        0.4f, 0.1f, 0.0f
    };

    float vertices2[] = {
        -0.4f,  -0.1f, 0.0f,
        -0.4f, -0.6f, 0.0f,
        -0.1f, -0.6f, 0.0f
    };

    // (0-indexed values)
    // Note that for rendering two non-overlapping triangles (as opposed to a rectangle)
    // an EBO is unecessary, but I want to maintain a reference for later
    unsigned int indices[] = {
        0, 1, 2
    };

    // Create a VAOs, VBOs, and EBOs
    unsigned int VAO, VAO2, VBO, VBO2, EBO;
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &VBO2);
    glGenBuffers(1, &EBO);

    // Configure VAO for first triangle
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Configure VAO for second triangle
    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // Reusing the same EBO so no need to reset its data
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



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

        // Render orange triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Render yellow triangle
        glUseProgram(shaderProgram2);
        glBindVertexArray(VAO2);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Swap color buffer once the new frame is ready
        glfwSwapBuffers(window);

        // Poll IO events (updates the window state and calls corresponding callback functions)
        glfwPollEvents();
    }

    // Memory clean-up
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgram2);

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