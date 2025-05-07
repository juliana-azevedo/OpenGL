#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glut.h>

using namespace std;

#define MAX_VERTICES 100
#define INF 999999

struct Vertex {
    int dist;
    bool known;
    int path;
};

struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
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

// Controles de transformação
float translateX = 0.0f, translateY = 0.0f;
float scale = 1.0f;
float angle = 0.0f;
bool rotating = false;
bool scaling = false;
bool translating = false;

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

// Desenha texto na tela
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// Função principal de desenho
void drawGraph() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    // Aplica transformações
    glTranslatef(translateX, translateY, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, 1.0f);
    
    // Desenha arestas
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.0f);
    for (int i = 0; i < numVertices; ++i) {
        for (int j = 0; j < numVertices; ++j) {
            if (graph[i][j] != 0) {
                glBegin(GL_LINES);
                glVertex2f(vertexPositions[i].x, vertexPositions[i].y);
                glVertex2f(vertexPositions[j].x, vertexPositions[j].y);
                glEnd();
                
                float midX = (vertexPositions[i].x + vertexPositions[j].x) / 2;
                float midY = (vertexPositions[i].y + vertexPositions[j].y) / 2;
                char weightText[10];
                sprintf(weightText, "%d", graph[i][j]);
                drawText(midX, midY, weightText);
            }
        }
    }
    
    // Destaca caminho mais curto
    if (!shortestPath.empty()) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glLineWidth(3.0f);
        for (size_t i = 0; i < shortestPath.size() - 1; ++i) {
            int from = shortestPath[i];
            int to = shortestPath[i+1];
            glBegin(GL_LINES);
            glVertex2f(vertexPositions[from].x, vertexPositions[from].y);
            glVertex2f(vertexPositions[to].x, vertexPositions[to].y);
            glEnd();
        }
    }
    
    // Desenha vértices
    for (int i = 0; i < numVertices; ++i) {
        if (i == startVertex) {
            glColor3f(0.0f, 1.0f, 0.0f); // Verde para início
        } else if (targetVertex != -1 && i == targetVertex) {
            glColor3f(0.0f, 0.0f, 1.0f); // Azul para destino
        } else {
            glColor3f(0.8f, 0.8f, 0.0f); // Amarelo para outros
        }
        
        glBegin(GL_TRIANGLE_FAN);
        for (int j = 0; j < 360; j += 30) {
            float ang = j * M_PI / 180.0f;
            float x = vertexPositions[i].x + 15 * cos(ang);
            float y = vertexPositions[i].y + 15 * sin(ang);
            glVertex2f(x, y);
        }
        glEnd();
        
        // Rótulos
        glColor3f(0.0f, 0.0f, 0.0f);
        char vertexText[10];
        sprintf(vertexText, "%d", i);
        drawText(vertexPositions[i].x - 5, vertexPositions[i].y - 5, vertexText);
        
        if (vertices[i].dist != INF) {
            char distText[20];
            sprintf(distText, "d=%d", vertices[i].dist);
            drawText(vertexPositions[i].x - 15, vertexPositions[i].y + 25, distText);
        }
    }
    
    glutSwapBuffers();
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

// Controles de teclado
void keyboard(unsigned char key, int x, int y) {
    if (key >= '0' && key < '0' + numVertices) {
        targetVertex = key - '0';
        reconstructPath(targetVertex);
        glutPostRedisplay();
    } else if (key == 27) { // ESC
        exit(0);
    }
}

// Teclas especiais (setas, etc.)
void specialKeys(int key, int x, int y) {
    float step = 10.0f;
    float scaleStep = 0.1f;
    float rotationStep = 5.0f;
    
    switch (key) {
        case GLUT_KEY_LEFT: translateX -= step; break;
        case GLUT_KEY_RIGHT: translateX += step; break;
        case GLUT_KEY_UP: translateY += step; break;
        case GLUT_KEY_DOWN: translateY -= step; break;
        case GLUT_KEY_PAGE_UP: scale += scaleStep; break;
        case GLUT_KEY_PAGE_DOWN: scale = max(0.1f, scale - scaleStep); break;
        case GLUT_KEY_HOME: angle += rotationStep; break;
        case GLUT_KEY_END: angle -= rotationStep; break;
    }
    glutPostRedisplay();
}

// Controles do mouse
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) rotating = (state == GLUT_DOWN);
    else if (button == GLUT_RIGHT_BUTTON) scaling = (state == GLUT_DOWN);
    else if (button == GLUT_MIDDLE_BUTTON) translating = (state == GLUT_DOWN);
}

// Arrastar com mouse
void motion(int x, int y) {
    static int prevX = x, prevY = y;
    int deltaX = x - prevX;
    int deltaY = y - prevY;
    
    if (rotating) angle += deltaX * 0.5f;
    else if (scaling) scale = max(0.1f, scale + deltaY * 0.01f);
    else if (translating) { translateX += deltaX; translateY -= deltaY; }
    
    prevX = x; prevY = y;
    glutPostRedisplay();
}

// Inicialização do OpenGL
void initialize() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Configura projeção ortográfica
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)windowWidth / (float)windowHeight;
    float viewSize = 500.0f; // Tamanho da área visível
    if (aspect >= 1.0) {
        glOrtho(-viewSize/2, viewSize/2, -viewSize/2/aspect, viewSize/2/aspect, -1.0, 1.0);
    } else {
        glOrtho(-viewSize/2*aspect, viewSize/2*aspect, -viewSize/2, viewSize/2, -1.0, 1.0);
    }
    
    glMatrixMode(GL_MODELVIEW);
    calculateVertexPositions();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
}

int main(int argc, char** argv) {
    // Configuração do grafo
    graph[0][1] = 2; graph[0][2] = 8;
    graph[1][2] = 5; graph[1][3] = 6;
    graph[2][3] = 3; graph[2][4] = 2;
    graph[3][4] = 1; graph[3][5] = 9;
    graph[4][5] = 3;
    
    dijkstra(graph, numVertices, startVertex);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Visualizador Dijkstra 2D com Projecao Ortografica");
    
    initialize();
    glutDisplayFunc(drawGraph);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    
    cout << "Controles:\n"
         << "0-5: Selecionar vértice destino\n"
         << "Setas: Mover\n"
         << "Page Up/Down: Zoom\n"
         << "Home/End: Rotacionar\n"
         << "Mouse: Arrastar para rotacionar/zoom/mover\n"
         << "ESC: Sair\n";
    
    glutMainLoop();
    return 0;
}