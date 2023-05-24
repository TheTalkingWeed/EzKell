#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include <vector>


using namespace std;

static int WIN_WIDTH = 600;
static int WIN_HEIGHT = 600;

float circle_x = 0.0f;
float circle_y = 0.0f;
float circle_speed = 0.04f;
float circle_r = 0.16f;

std::vector<glm::vec3> linePoints;
std::vector<glm::vec3> lineColors;
std::vector<glm::vec3> circlePoints;
std::vector<glm::vec3> circleColors;

#define numVBOs 2
#define numVAOs 2
GLuint VBO[numVBOs];
GLuint VAO[numVAOs];

GLuint renderingProgram;
GLuint circleShaderProgram;
GLuint lineShaderProgram;

GLuint circleVertexShader;
GLuint circleFragmentShader;
GLuint lineVertexShader;
GLuint lineFragmentShader;

GLuint Xoffset;
GLuint Yoffset;

float start_x;
float start_y;
float end_x;
float end_y;

bool checkOpenGLError() {
    bool foundError = false;
    int glErr = glGetError();
    while (glErr != GL_NO_ERROR) {
        cout << "glError: " << glErr << endl;
        foundError = true;
        glErr = glGetError();
    }
    return foundError;
}

void printShaderLog(GLuint shader) {
    int len = 0;
    int chWrittn = 0;
    char* log;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetShaderInfoLog(shader, len, &chWrittn, log);
        cout << "Shader Info Log: " << log << endl;
        free(log);
    }
}

void printProgramLog(int prog) {
    int len = 0;
    int chWrittn = 0;
    char* log;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
        log = (char*)malloc(len);
        glGetProgramInfoLog(prog, len, &chWrittn, log);
        cout << "Program Info Log: " << log << endl;
        free(log);
    }
}

string readShaderSource(const char* filePath) {
    string content;
    ifstream fileStream(filePath, ios::in);
    string line = "";

    while (!fileStream.eof()) {
        getline(fileStream, line);
        content.append(line + "\n");
    }
    fileStream.close();
    return content;
}

GLuint createShaderProgram(const char* vertexShaderPath, const char* fragmentShaderPath) {
    GLint vertCompiled;
    GLint fragCompiled;
    GLint linked;

    string vertShaderStr = readShaderSource(vertexShaderPath);
    string fragShaderStr = readShaderSource(fragmentShaderPath);

    const char* vertShaderSrc = vertShaderStr.c_str();
    const char* fragShaderSrc = fragShaderStr.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vShader, 1, &vertShaderSrc, NULL);
    glShaderSource(fShader, 1, &fragShaderSrc, NULL);

    glCompileShader(vShader);
    checkOpenGLError();
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
    if (vertCompiled != 1) {
        cout << "vertex compilation failed" << endl;
        printShaderLog(vShader);
    }

    glCompileShader(fShader);
    checkOpenGLError();
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
    if (fragCompiled != 1) {
        cout << "fragment compilation failed" << endl;
        printShaderLog(fShader);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);

    glLinkProgram(program);
    checkOpenGLError();
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != 1) {
        cout << "linking failed" << endl;
        printProgramLog(program);
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return program;
}

void cleanUpScene()
{
    glDeleteVertexArrays(numVAOs, VAO);
    glDeleteBuffers(numVBOs, VBO);

    glDeleteProgram(renderingProgram);
    glDeleteProgram(circleShaderProgram);
    glDeleteProgram(lineShaderProgram);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

}

void generateCirclePoints(glm::vec2 O, GLfloat r, GLint num_segment) {
    GLfloat x, y;
    GLfloat alpha = 0.0f;
    GLfloat full_circle = 2.0f * M_PI;
    circlePoints.push_back(glm::vec3(O.x, O.y, 0.0f));
    circleColors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));

    for (int i = 0; i < num_segment; i++) {
        x = O.x + r * cos(alpha);
        y = O.y + r * sin(alpha);

        circlePoints.push_back(glm::vec3(x, y, 0.0f));
        circleColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

        alpha += full_circle / num_segment;
    }
    circlePoints.push_back(glm::vec3(O.x + r, 0, 0.0f));
    circleColors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
}

void generateLinePoints(glm::vec2 start_point, glm::vec2 vector, GLint num_segment) {
    GLfloat x, y;
    GLfloat t = 0.0f;

    for (int i = 0; i <= num_segment; i++) {
        x = start_point.x + t * vector.x;
        y = start_point.y + t * vector.y;

        linePoints.push_back(glm::vec3(x, y, 0.0f));

        t += 1.0f / num_segment;
    }
}

void init(GLFWwindow* window) {
    circleShaderProgram = createShaderProgram("circleVertexShader.glsl", "circleFragmentShader.glsl");
    lineShaderProgram = createShaderProgram("lineVertexShader.glsl", "lineFragmentShader.glsl");

    generateCirclePoints(glm::vec2(circle_x, circle_y), circle_r, 64);
    generateLinePoints(glm::vec2(-1.0f, -1.0f), glm::vec2(2.0f, 2.0f), 64);

    glGenVertexArrays(numVAOs, VAO);
    glGenBuffers(numVBOs, VBO);

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, circlePoints.size() * sizeof(glm::vec3), &circlePoints[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, linePoints.size() * sizeof(glm::vec3), &linePoints[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glPointSize(10.0f);
    glLineWidth(3.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);

    circle_x += circle_speed;
    if (circle_x + circle_r > 1.0f)
        circle_speed *= -1;
    if (circle_x - circle_r < -1.0f)
        circle_speed *= -1;

    glUseProgram(lineShaderProgram);
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_LINE_STRIP, 0, linePoints.size());


    glUseProgram(circleShaderProgram);
    glBindVertexArray(VAO[0]);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circlePoints.size());


    glUseProgram(0);
    glBindVertexArray(0);
}

int main(void) {
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "OpenGL Window", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    init(window);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene();

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
