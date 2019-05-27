#include "terrain.h"

#include <math.h>
#include "gl/shaders/ShaderAttribLocations.h"

Terrain::Terrain() : m_numRows(100), m_numCols(m_numRows)
{
}


/**
 * Returns a pseudo-random value between -1.0 and 1.0 for the given row and column.
 */
float Terrain::randValue(int row, int col) {
    return -1.0 + 2.0 * glm::fract(sin(row * 127.1f + col * 311.7f) * 43758.5453123f);
}

//static float NoiseFunction(int row, int col, float freq, float amplitude) {
//    int sRow = row/freq;
//    int sCol = col/freq;

//    float fractRow = glm::fract(row/freq);
//    float fractCol = glm::fract(col/freq);

//    float a = randValue(sRow, sCol);
//    float b = randValue(sRow, sCol + 1);
//    float c = randValue(sRow + 1, sCol);
//    float d = randValue(sRow + 1, sCol + 1);

//    float x = fractCol*fractCol*(3-2*fractCol);
//    float y = fractRow*fractRow*(3-2*fractRow);
//    return glm::mix(glm::mix(a, b, x), glm::mix(c, d, x), y) * amplitude;
//}

float Terrain::noiseFunction(int row, int col, float freq, float amplitude)
{
    int sRow = row/freq;
    int sCol = col/freq;

    float fractRow = glm::fract(row/freq);
    float fractCol = glm::fract(col/freq);

    float a = randValue(sRow, sCol);
    float b = randValue(sRow, sCol + 1);
    float c = randValue(sRow + 1, sCol);
    float d = randValue(sRow + 1, sCol + 1);

    float x = fractCol*fractCol*(3-2*fractCol);
    float y = fractRow*fractRow*(3-2*fractRow);
    return (float)glm::mix(glm::mix(a, b, x), glm::mix(c, d, x), y) * amplitude;
}

/**
 * Returns the object-space position for the terrain vertex at the given row and column.
 */
glm::vec3 Terrain::getPosition(int row, int col) {
    glm::vec3 position;
    position.x = 10 * row/m_numRows - 5;
    position.z = 10 * col/m_numCols - 5;

    // TODO: Adjust position.y using value noise.
    position.y = noiseFunction(row, col, 10.0, 0.5) + noiseFunction(row, col, 20.0, 1) + noiseFunction(row, col, 5.0, 0.25);
    return position;
}


/**
 * Returns the normal vector for the terrain vertex at the given row and column.
 */
glm::vec3 Terrain::getNormal(int row, int col) {
    // TODO: Compute the normal at the given row and column using the positions of the
    //       neighboring vertices.

    glm::vec3 pos = getPosition(row, col);
    glm::vec3 n0 = getPosition(row, col + 1);
    glm::vec3 n1 = getPosition(row - 1, col + 1);
    glm::vec3 n2 = getPosition(row - 1, col);
    glm::vec3 n3 = getPosition(row - 1, col - 1);
    glm::vec3 n4 = getPosition(row, col - 1);
    glm::vec3 n5 = getPosition(row + 1, col - 1);
    glm::vec3 n6 = getPosition(row + 1, col);
    glm::vec3 n7 = getPosition(row + 1, col + 1);
    glm::vec3 solution = glm::cross(n1 - pos, n0 - pos) + glm::cross(n2 - pos, n1 - pos)
            + glm::cross(n3 - pos, n2 - pos) + glm::cross(n4 - pos, n3 - pos) + glm::cross(n5 - pos, n4 - pos) + glm::cross(n6 - pos, n5 - pos)
            + glm::cross(n7 - pos, n6 - pos) + glm::cross(n0 - pos, n7 - pos);
    solution /= 8;
    return glm::normalize(solution);
}


/**
 * Initializes the terrain by storing positions and normals in a vertex buffer.
 */
void Terrain::init() {
    // TODO: Change from GL_LINE to GL_FILL in order to render full triangles instead of wireframe.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    // Initializes a grid of vertices using triangle strips.
    int numVertices = (m_numRows - 1) * (2 * m_numCols + 2);
    std::vector<glm::vec3> data(2 * numVertices);
    int index = 0;
    for (int row = 0; row < m_numRows - 1; row++) {
        for (int col = m_numCols - 1; col >= 0; col--) {
            data[index++] = getPosition(row, col);
            data[index++] = getNormal  (row, col);
            data[index++] = getPosition(row + 1, col);
            data[index++] = getNormal  (row + 1, col);
        }
        data[index++] = getPosition(row + 1, 0);
        data[index++] = getNormal  (row + 1, 0);
        data[index++] = getPosition(row + 1, m_numCols - 1);
        data[index++] = getNormal  (row + 1, m_numCols - 1);
    }

    // Initialize OpenGLShape.
    m_shape = std::make_unique<OpenGLShape>();
    m_shape->setVertexData(&data[0][0], data.size() * 3, VBO::GEOMETRY_LAYOUT::LAYOUT_TRIANGLE_STRIP, numVertices);
    m_shape->setAttribute(ShaderAttrib::POSITION, 3, 0, VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_shape->setAttribute(ShaderAttrib::NORMAL, 3, sizeof(glm::vec3), VBOAttribMarker::DATA_TYPE::FLOAT, false);
    m_shape->buildVAO();
}


/**
 * Draws the terrain.
 */
void Terrain::draw()
{
    m_shape->draw();
}
