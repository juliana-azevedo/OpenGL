#include <iostream>
#include <vector>
#include <cmath>
#include <GL/glut.h>

using namespace std;

// Definições básicas
#define MAX_VERTICES 100
#define INF 999999

// Estrutura para representar um vértice
struct Vertex {
    int dist;
    bool known;
    int path;
};

// Estrutura para representar uma posição 2D
struct Point2D {
    float x, y;
    Point2D(float x = 0, float y = 0) : x(x), y(y) {}
};

// Variáveis globais
int numVertices = 6;
int graph[MAX_VERTICES][MAX_VERTICES] = {0};
Vertex vertices[MAX_VERTICES];
vector<int> shortestPath; // Armazena o caminho mais curto para destacar
Point2D vertexPositions[MAX_VERTICES]; // Posições dos vértices na tela
int windowWidth = 800, windowHeight = 600;
int startVertex = 0;
int targetVertex = -1; // Vértice de destino para visualizar o caminho

// Variáveis para transformações
float translateX = 0.0f, translateY = 0.0f;
float scale = 1.0f;
float angle = 0.0f;
bool rotating = false;
bool scaling = false;
bool translating = false;

// Função para encontrar o vértice não processado com menor distância
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
    // Inicialização
    for (int i = 0; i < numVertices; ++i) {
        vertices[i].dist = INF;
        vertices[i].known = false;
        vertices[i].path = -1;
    }
    vertices[start].dist = 0;

    // Processamento
    for (int count = 0; count < numVertices - 1; ++count) {
        int v = findMinDistanceVertex(vertices, numVertices);
        if (v == -1) break;
        
        vertices[v].known = true;

        // Atualiza as distâncias dos vizinhos de v
        for (int w = 0; w < numVertices; ++w) {
            if (graph[v][w] != 0 && !vertices[w].known && 
                vertices[v].dist + graph[v][w] < vertices[w].dist) {
                vertices[w].dist = vertices[v].dist + graph[v][w];
                vertices[w].path = v;
            }
        }
    }

    // Imprime as distâncias mínimas
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

// Função para calcular as posições dos vértices em um círculo
void calculateVertexPositions() {
    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;
    float radius = min(windowWidth, windowHeight) * 0.4f;
    
    for (int i = 0; i < numVertices; ++i) {
        float angle = 2 * M_PI * i / numVertices;
        vertexPositions[i].x = centerX + radius * cos(angle);
        vertexPositions[i].y = centerY + radius * sin(angle);
    }
}

// Função para desenhar texto na tela
void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// Função para desenhar o grafo
void drawGraph() {
    // Configurações iniciais
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    // Aplica as transformações
    glTranslatef(translateX, translateY, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, 1.0f);
    
    // Desenha as arestas
    glColor3f(0.5f, 0.5f, 0.5f); // Cinza
    glLineWidth(1.0f);
    for (int i = 0; i < numVertices; ++i) {
        for (int j = 0; j < numVertices; ++j) {
            if (graph[i][j] != 0) {
                glBegin(GL_LINES);
                glVertex2f(vertexPositions[i].x, vertexPositions[i].y);
                glVertex2f(vertexPositions[j].x, vertexPositions[j].y);
                glEnd();
                
                // Desenha o peso da aresta
                float midX = (vertexPositions[i].x + vertexPositions[j].x) / 2;
                float midY = (vertexPositions[i].y + vertexPositions[j].y) / 2;
                char weightText[10];
                sprintf(weightText, "%d", graph[i][j]);
                drawText(midX, midY, weightText);
            }
        }
    }
    
    // Desenha o caminho mais curto em vermelho
    if (!shortestPath.empty()) {
        glColor3f(1.0f, 0.0f, 0.0f); // Vermelho
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
    
    // Desenha os vértices
    for (int i = 0; i < numVertices; ++i) {
        // Cor diferente para o vértice inicial
        if (i == startVertex) {
            glColor3f(0.0f, 1.0f, 0.0f); // Verde
        } else if (targetVertex != -1 && i == targetVertex) {
            glColor3f(0.0f, 0.0f, 1.0f); // Azul
        } else {
            glColor3f(0.8f, 0.8f, 0.0f); // Amarelo
        }
        
        glBegin(GL_TRIANGLE_FAN);
        for (int j = 0; j < 360; j += 30) {
            float angle = j * M_PI / 180.0f;
            float x = vertexPositions[i].x + 15 * cos(angle);
            float y = vertexPositions[i].y + 15 * sin(angle);
            glVertex2f(x, y);
        }
        glEnd();
        
        // Desenha o número do vértice
        glColor3f(0.0f, 0.0f, 0.0f); // Preto
        char vertexText[10];
        sprintf(vertexText, "%d", i);
        drawText(vertexPositions[i].x - 5, vertexPositions[i].y - 5, vertexText);
        
        // Desenha a distância mínima
        if (vertices[i].dist != INF) {
            char distText[20];
            sprintf(distText, "d=%d", vertices[i].dist);
            drawText(vertexPositions[i].x - 15, vertexPositions[i].y + 25, distText);
        }
    }
    
    glutSwapBuffers();
}

// Função para reconstruir o caminho mais curto
void reconstructPath(int target) {
    shortestPath.clear();
    if (vertices[target].dist == INF) return;
    
    int current = target;
    while (current != -1) {
        shortestPath.insert(shortestPath.begin(), current);
        current = vertices[current].path;
    }
}

// Função de tratamento de teclado
void keyboard(unsigned char key, int x, int y) {
    if (key >= '0' && key < '0' + numVertices) {
        targetVertex = key - '0';
        reconstructPath(targetVertex);
        glutPostRedisplay();
    } else if (key == 27) { // ESC
        exit(0);
    }
}

// Função para teclas especiais (setas, etc.)
void specialKeys(int key, int x, int y) {
    float step = 10.0f; // Passo para translação
    float scaleStep = 0.1f; // Passo para escala
    float rotationStep = 5.0f; // Passo para rotação
    
    switch (key) {
        case GLUT_KEY_LEFT:
            translateX -= step;
            break;
        case GLUT_KEY_RIGHT:
            translateX += step;
            break;
        case GLUT_KEY_UP:
            translateY += step;
            break;
        case GLUT_KEY_DOWN:
            translateY -= step;
            break;
        case GLUT_KEY_PAGE_UP:
            scale += scaleStep;
            break;
        case GLUT_KEY_PAGE_DOWN:
            scale -= scaleStep;
            if (scale < 0.1f) scale = 0.1f;
            break;
        case GLUT_KEY_HOME:
            angle += rotationStep;
            break;
        case GLUT_KEY_END:
            angle -= rotationStep;
            break;
    }
    
    glutPostRedisplay();
}

// Função para interação com o mouse
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        rotating = true;
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        scaling = true;
    } else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN) {
        translating = true;
    } else {
        rotating = false;
        scaling = false;
        translating = false;
    }
}

// Função para arrastar com o mouse
void motion(int x, int y) {
    static int prevX = x, prevY = y;
    int deltaX = x - prevX;
    int deltaY = y - prevY;
    
    if (rotating) {
        angle += deltaX * 0.5f;
    } else if (scaling) {
        scale += deltaY * 0.01f;
        if (scale < 0.1f) scale = 0.1f;
    } else if (translating) {
        translateX += deltaX;
        translateY -= deltaY; // Inverte Y porque o sistema de coordenadas do mouse é invertido
    }
    
    prevX = x;
    prevY = y;
    glutPostRedisplay();
}

// Função principal de inicialização do OpenGL
void initialize() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Fundo branco
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    
    calculateVertexPositions();
    
    // Habilita suporte a cores e blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
}

int main(int argc, char** argv) {
    // Configuração do grafo
    graph[0][1] = 2;
    graph[0][2] = 8;
    graph[1][2] = 5;
    graph[1][3] = 6;
    graph[2][3] = 3;
    graph[2][4] = 2;
    graph[3][4] = 1;
    graph[3][5] = 9;
    graph[4][5] = 3;
    
    // Executa Dijkstra
    dijkstra(graph, numVertices, startVertex);
    
    // Inicializa o OpenGL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Visualizacao do Algoritmo de Dijkstra com Transformacoes");
    
    initialize();
    glutDisplayFunc(drawGraph);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    
    cout << "Controles:" << endl;
    cout << "- Teclas 0-5: Visualizar caminho para o vértice" << endl;
    cout << "- Setas: Translação" << endl;
    cout << "- Page Up/Down: Escala" << endl;
    cout << "- Home/End: Rotação" << endl;
    cout << "- Botão esquerdo do mouse + arrastar: Rotação" << endl;
    cout << "- Botão direito do mouse + arrastar: Escala" << endl;
    cout << "- Botão do meio do mouse + arrastar: Translação" << endl;
    cout << "- ESC: Sair" << endl;
    
    glutMainLoop();
    
    return 0;
}