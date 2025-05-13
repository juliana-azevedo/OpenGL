cmake_minimum_required(VERSION 3.10)
project(Trabalho)

# Definir padrão C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar pacotes necessários
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)
find_package(Freetype REQUIRED)

# Incluir diretórios
include_directories(${GLEW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

# Adicionar executável
add_executable(Trabalho main.cpp)

# Linkar bibliotecas
target_link_libraries(Trabalho
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
    ${FREETYPE_LIBRARIES}
)

# Copiar fonte para o diretório de build (opcional)
# Substitua pelo caminho correto da fonte
file(COPY /usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf DESTINATION ${CMAKE_BINARY_DIR})
