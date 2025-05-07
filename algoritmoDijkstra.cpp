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

// Função principal de inicialização do OpenGL
void initialize() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Fundo branco
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    
    calculateVertexPositions();
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
    glutCreateWindow("Visualizacao do Algoritmo de Dijkstra");
    
    initialize();
    glutDisplayFunc(drawGraph);
    glutKeyboardFunc(keyboard);
    
    cout << "Pressione um numero de 0 a " << numVertices-1 << " para visualizar o caminho mais curto" << endl;
    cout << "Pressione ESC para sair" << endl;
    
    glutMainLoop();
    
    return 0;
}