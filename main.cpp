#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

// Declaração antecipada da função compileShader
GLuint compileShader(GLenum type, const char* source);

// Shaders para geometria
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 position;
    uniform mat4 model;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * model * vec4(position, 0.0, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec3 color;
    void main() {
        FragColor = vec4(color, 1.0);
    }
)glsl";

// Shaders para texto
const char* textVertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec4 vertex; // vec2 pos, vec2 tex
    out vec2 TexCoords;
    uniform mat4 projection;
    void main() {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
        TexCoords = vertex.zw;
    }
)glsl";

const char* textFragmentShaderSource = R"glsl(
    #version 330 core
    in vec2 TexCoords;
    out vec4 color;
    uniform sampler2D text;
    uniform vec3 textColor;
    void main() {    
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
        color = vec4(textColor, 1.0) * sampled;
    }
)glsl";

#define MAX_VERTICES 100
#define INF 999999
#define SEGMENTS 60

struct Vertex {
    int dist;
    bool known;
    int path;
};

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Character {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

class TextRenderer {
public:
    std::map<GLchar, Character> characters;
    GLuint VAO, VBO, shaderProgram;

    TextRenderer(GLuint width, GLuint height) {
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, textVertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource);
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        FT_Library ft;
        if (FT_Init_FreeType(&ft))
            std::cerr << "Erro ao inicializar FreeType" << std::endl;

        FT_Face face;
        // Substitua pelo caminho de uma fonte disponível
        if (FT_New_Face(ft, "DejaVuSans-Bold.ttf", 0, &face)) ///usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
            std::cerr << "Erro ao carregar fonte" << std::endl;

        FT_Set_Pixel_Sizes(face, 0, 24); //24

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "Erro ao carregar caractere " << c << std::endl;
                continue;
            }

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows,
                         0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<GLuint>(face->glyph->advance.x)
            };
            characters.insert(std::pair<GLchar, Character>(c, character));
        }
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, "textColor"), 1, &color[0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        for (auto c : text) {
            Character ch = characters[c];

            float xpos = x + ch.bearing.x * scale;
            float ypos = y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };

            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    glm::mat4 projection;
};

// Variáveis globais
int numVertices = 6;
int graph[MAX_VERTICES][MAX_VERTICES] = {0};
Vertex vertices[MAX_VERTICES];
vector<int> shortestPath;
Point2D vertexPositions[MAX_VERTICES];
int windowWidth = 800, windowHeight = 600;
int startVertex = 0;
int targetVertex = -1;
glm::vec2 translate = glm::vec2(0.0f);
float scale = 1.0f;
float angle = 0.0f;
bool rotating = false;
bool scaling = false;
bool translating = false;
float viewSize = 500.0f;
GLuint shaderProgram;
GLuint lineVAO, lineVBO;
GLuint pathVAO, pathVBO;
GLuint circleVAO, circleVBO;
TextRenderer* textRenderer;

// Função para compilar shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cerr << "Erro de compilação do shader:\n" << infoLog << endl;
    }
    return shader;
}

// Função para encontrar o vértice com menor distância
int findMinDistanceVertex(Vertex vertices[], int numVertices) {
    int minDist = INF;
    int minIndex = -1;
    
    for (int i = 0; i < numVertices; ++i) {
        if (!vertices[i].known && vertices[i].dist < minDist) {
            minDist = vertices[i].dist;
            minIndex = i;
        }
    }
    return minIndex;
}

// Algoritmo de Dijkstra
void dijkstra(int graph[MAX_VERTICES][MAX_VERTICES], int numVertices, int start) {
    for (int i = 0; i < numVertices; ++i) {
        vertices[i].dist = INF;
        vertices[i].known = false;
        vertices[i].path = -1;
    }
    vertices[start].dist = 0;

    for (int count = 0; count < numVertices - 1; ++count) {
        int v = findMinDistanceVertex(vertices, numVertices);
        if (v == -1) break;
        
        vertices[v].known = true;

        for (int w = 0; w < numVertices; ++w) {
            if (graph[v][w] != 0 && !vertices[w].known && 
                vertices[v].dist + graph[v][w] < vertices[w].dist) {
                vertices[w].dist = vertices[v].dist + graph[v][w];
                vertices[w].path = v;
            }
        }
    }

    cout << "Vertice\tDistancia\tCaminho" << endl;
    for (int i = 0; i < numVertices; ++i) {
        cout << i << "\t" << vertices[i].dist << "\t\t";
        int current = i;
        while (current != -1) {
            cout << current;
            current = vertices[current].path;
            if (current != -1) cout << " <- ";
        }
        cout << endl;
    }
}

// Calcula posições dos vértices em um círculo
void calculateVertexPositions() {
    float radius = min(windowWidth, windowHeight) * 0.3f;
    for (int i = 0; i < numVertices; ++i) {
        float ang = 2 * M_PI * i / numVertices;
        vertexPositions[i].x = radius * cos(ang);
        vertexPositions[i].y = radius * sin(ang);
    }
}

// Reconstrói o caminho mais curto
void reconstructPath(int target) {
    shortestPath.clear();
    if (vertices[target].dist == INF) return;
    
    int current = target;
    while (current != -1) {
        shortestPath.insert(shortestPath.begin(), current);
        current = vertices[current].path;
    }
}

// Inicializa buffers OpenGL
void initBuffers() {
    vector<float> circleVertices;
    const int segments = SEGMENTS;
    const float radius = 25.0f; //15.0f;
    
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * i / segments;
        circleVertices.push_back(radius * cos(angle));
        circleVertices.push_back(radius * sin(angle));
    }
    
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    vector<float> lineVertices;
    for (int i = 0; i < numVertices; ++i) {
        for (int j = 0; j < numVertices; ++j) {
            if (graph[i][j] != 0) {
                lineVertices.push_back(vertexPositions[i].x);
                lineVertices.push_back(vertexPositions[i].y);
                lineVertices.push_back(vertexPositions[j].x);
                lineVertices.push_back(vertexPositions[j].y);
            }
        }
    }
    
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glGenVertexArrays(1, &pathVAO);
    glGenBuffers(1, &pathVBO);
    glBindVertexArray(pathVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pathVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// Atualiza o buffer do caminho mais curto
void updatePathBuffer() {
    vector<float> pathVertices;
    for (size_t i = 0; i < shortestPath.size() - 1; ++i) {
        int from = shortestPath[i];
        int to = shortestPath[i+1];
        pathVertices.push_back(vertexPositions[from].x);
        pathVertices.push_back(vertexPositions[from].y);
        pathVertices.push_back(vertexPositions[to].x);
        pathVertices.push_back(vertexPositions[to].y);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, pathVBO);
    glBufferData(GL_ARRAY_BUFFER, pathVertices.size() * sizeof(float), pathVertices.data(), GL_DYNAMIC_DRAW);
}

// Função para resetar a visualização
void resetView() {
    translate = glm::vec2(0.0f);
    scale = 1.0f;
    angle = 0.0f;
}

// Função de renderização
void render(GLFWwindow* window) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    float aspect = (float)width / (float)height;
    
    glm::mat4 projection;
    if (aspect >= 1.0) {
        projection = glm::ortho(
            -viewSize/2.0f * aspect, viewSize/2.0f * aspect,
            -viewSize/2.0f, viewSize/2.0f,
            -1.0f, 1.0f);
    } else {
        projection = glm::ortho(
            -viewSize/2.0f, viewSize/2.0f,
            -viewSize/2.0f / aspect, viewSize/2.0f / aspect,
            -1.0f, 1.0f);
    }
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(translate, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, scale, 1.0f));
    
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.5f, 0.5f, 0.5f);
    glLineWidth(2.0f); // Aumento da largura da linha
    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINES, 0, numVertices * numVertices * 2);
    glLineWidth(1.0f); // Resetando a largura da linha
    
    if (!shortestPath.empty()) {
        updatePathBuffer();
        glUniform3f(glGetUniformLocation(shaderProgram, "color"), 1.0f, 0.0f, 0.0f);
        glLineWidth(5.0f); // 3.0f
        glBindVertexArray(pathVAO);
        glDrawArrays(GL_LINES, 0, shortestPath.size() * 2);
        glLineWidth(1.0f);
    }
    
    glBindVertexArray(circleVAO);
    for (int i = 0; i < numVertices; ++i) {
        glm::mat4 vertexModel = glm::translate(model, glm::vec3(vertexPositions[i].x, vertexPositions[i].y, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(vertexModel));
        
        if (i == startVertex) {
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 1.0f, 0.0f);
        } else if (targetVertex != -1 && i == targetVertex) {
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f);
        } else {
            glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.8f, 0.8f, 0.0f);
        }
        
        glDrawArrays(GL_TRIANGLE_FAN, 0, SEGMENTS + 2);
    }
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < numVertices; ++i) {
        glm::vec4 pos(vertexPositions[i].x, vertexPositions[i].y, 0.0f, 1.0f);
        pos = projection * model * pos;
        float x = (pos.x / pos.w + 1.0f) * width / 2.0f;
        float y = (pos.y / pos.w + 1.0f) * height / 2.0f;
        textRenderer->RenderText(std::to_string(i), x - 8.0f, y - 8.0f, 0.8f, glm::vec3(0.0f, 0.0f, 0.0f)); // textRenderer->RenderText(std::to_string(i), x - 5.0f, y - 5.0f, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
    }

    for (int i = 0; i < numVertices; ++i) {
        for (int j = i + 1; j < numVertices; ++j) {
            if (graph[i][j] != 0) {
                glm::vec2 mid = glm::vec2(
                    (vertexPositions[i].x + vertexPositions[j].x) / 2.0f,
                    (vertexPositions[i].y + vertexPositions[j].y) / 2.0f
                );
                glm::vec4 pos(mid.x, mid.y, 0.0f, 1.0f);
                pos = projection * model * pos;
                float x = (pos.x / pos.w + 1.0f) * width / 2.0f;
                float y = (pos.y / pos.w + 1.0f) * height / 2.0f;
                textRenderer->RenderText(std::to_string(graph[i][j]), x - 10.0f, y - 10.0f, 0.8f, glm::vec3(0.0f, 0.0f, 0.0f)); //textRenderer->RenderText(std::to_string(graph[i][j]), x - 10.0f, y - 10.0f, 0.5f, glm::vec3(0.0f, 0.0f, 0.0f));
            }
        }
    }

    glfwSwapBuffers(window);
}

// Callbacks de teclado
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float step = 10.0f;
        float scaleStep = 0.1f;
        float rotationStep = 5.0f;
        
        switch (key) {
            case GLFW_KEY_LEFT: translate.x -= step; break;
            case GLFW_KEY_RIGHT: translate.x += step; break;
            case GLFW_KEY_UP: translate.y += step; break;
            case GLFW_KEY_DOWN: translate.y -= step; break;
            case GLFW_KEY_PAGE_UP: scale += scaleStep; break;
            case GLFW_KEY_PAGE_DOWN: scale = max(0.1f, scale - scaleStep); break;
            case GLFW_KEY_HOME: angle += rotationStep; break;
            case GLFW_KEY_END: angle -= rotationStep; break;
            case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
            case GLFW_KEY_R: resetView(); break;
        }
        
        if (key >= GLFW_KEY_0 && key <= GLFW_KEY_5) {
            targetVertex = key - GLFW_KEY_0;
            if (targetVertex == startVertex) {
                shortestPath.clear(); // Limpa o caminho se o destino é o nó inicial
            } else {
                reconstructPath(targetVertex); // Reconstrói o caminho para outros nós
            }
        }
    }
}

// Callbacks de mouse
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) rotating = true;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) scaling = true;
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) translating = true;
    } else if (action == GLFW_RELEASE) {
        rotating = false;
        scaling = false;
        translating = false;
    }
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    static double prevX = xpos, prevY = ypos;
    double deltaX = xpos - prevX;
    double deltaY = ypos - prevY;
    
    if (rotating) angle += deltaX * 0.5f;
    else if (scaling) scale = std::max(0.1f, scale + static_cast<float>(deltaY * 0.01f));
    else if (translating) { 
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        translate.x += deltaX * (viewSize / width);
        translate.y -= deltaY * (viewSize / height);
    }
    
    prevX = xpos;
    prevY = ypos;
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    delete textRenderer;
    textRenderer = new TextRenderer(width, height);
}

int main() {
    if (!glfwInit()) {
        cerr << "Falha ao inicializar GLFW" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Dijkstra com OpenGL", NULL, NULL);
    if (!window) {
        cerr << "Falha ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        cerr << "Falha ao inicializar GLEW" << endl;
        return -1;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Configuração do grafo (não direcionado)
    graph[0][1] = 2; graph[1][0] = 2;
    graph[0][2] = 8; graph[2][0] = 8;
    graph[1][2] = 5; graph[2][1] = 5;
    graph[1][3] = 6; graph[3][1] = 6;
    graph[2][3] = 3; graph[3][2] = 3;
    graph[2][4] = 2; graph[4][2] = 2;
    graph[3][4] = 1; graph[4][3] = 1;
    graph[3][5] = 9; graph[5][3] = 9;
    graph[4][5] = 3; graph[5][4] = 3;

    dijkstra(graph, numVertices, startVertex);
    calculateVertexPositions();

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cerr << "Erro de linkagem do shader program:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    initBuffers();
    textRenderer = new TextRenderer(windowWidth, windowHeight);

    cout << "Controles:\n"
         << "0-5: Selecionar vértice destino\n"
         << "Setas: Mover\n"
         << "Page Up/Down: Zoom\n"
         << "Home/End: Rotacionar\n"
         << "R: Resetar visualização\n"
         << "Mouse: Arrastar para rotacionar/zoom/mover\n"
         << "ESC: Sair\n";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        render(window);
    }

    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &pathVAO);
    glDeleteBuffers(1, &pathVBO);
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteProgram(shaderProgram);
    delete textRenderer;

    glfwTerminate();
    return 0;
}