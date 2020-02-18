#define _USE_MATH_DEFINES

#include "Mesh.h"

#include <cmath>
#include <algorithm>

using namespace std;

Mesh::~Mesh () {
	clear ();
}

void Mesh::computeBoundingSphere (glm::vec3 & center, float & radius) const {
	center = glm::vec3 (0.0);
	radius = 0.f;
	for (const auto & p : m_vertexPositions)
		center += p;
	center /= m_vertexPositions.size ();
	for (const auto & p : m_vertexPositions)
		radius = std::max (radius, distance (center, p));
}

void Mesh::recomputePerVertexNormals (bool angleBased) {

		m_vertexNormals.clear ();
		int i0;
		int i1;
		int i2;
		glm::vec3 p0, p1, p2;
		float angle0, angle1, angle2;
		glm::vec3 faceNormal;
		//tableau des moyennes de chaque vertex
		std::vector<int> avgVector;
		//tableau des angles
		std::vector<float> anglesVector;
		// Change the following code to compute a proper per-vertex normal
		m_vertexNormals.resize (m_vertexPositions.size (), glm::vec3 (0.0, 0.0, 0.0));

		avgVector.clear();
		avgVector.resize(m_vertexPositions.size(), 0);

		anglesVector.clear();
		anglesVector.resize(m_vertexPositions.size(), 0);

		if (angleBased == false) {
			for (int i = 0; i < m_triangleIndices.size(); i++) {
				i0 = m_triangleIndices[i][0];
				i1 = m_triangleIndices[i][1];
				i2 = m_triangleIndices[i][2];
				avgVector[i0]++;
				avgVector[i1]++;
				avgVector[i2]++;
				p0 = m_vertexPositions[i0];
				p1 = m_vertexPositions[i1];
				p2 = m_vertexPositions[i2];
				faceNormal = normalize(cross(p1-p0,p2-p0));
				m_vertexNormals[i0] = m_vertexNormals[i0] + faceNormal;
				m_vertexNormals[i1] = m_vertexNormals[i1] + faceNormal;
				m_vertexNormals[i2] = m_vertexNormals[i2] + faceNormal;
			}
			for (int i = 0; i < m_vertexNormals.size(); i++) {
				m_vertexNormals[i] = (1.f/avgVector[i]) * m_vertexNormals[i] ;
				m_vertexNormals[i] = normalize(m_vertexNormals[i]);
			}
		}
		else  {
			for (int i = 0; i < m_triangleIndices.size(); i++) {
				i0 = m_triangleIndices[i][0];
				i1 = m_triangleIndices[i][1];
				i2 = m_triangleIndices[i][2];

				p0 = m_vertexPositions[i0];
				p1 = m_vertexPositions[i1];
				p2 = m_vertexPositions[i2];

				angle0 = acos(dot(normalize(p1-p0),normalize(p2-p0)));
				angle1 = acos(dot(normalize(p0-p1),normalize(p2-p1)));
				angle2 = acos(dot(normalize(p0-p2),normalize(p1-p2)));

				anglesVector[i0] = anglesVector[i0] + angle0;
				anglesVector[i1] = anglesVector[i1] + angle1;
				anglesVector[i2] = anglesVector[i2] + angle2;

				faceNormal = normalize(cross(p1-p0,p2-p0));
				m_vertexNormals[i0] = m_vertexNormals[i0] + faceNormal * angle0;
				m_vertexNormals[i1] = m_vertexNormals[i1] + faceNormal * angle1;
				m_vertexNormals[i2] = m_vertexNormals[i2] + faceNormal * angle2;
			}
			for (int i = 0; i < m_vertexNormals.size(); i++) {
				m_vertexNormals[i] = (1.f/anglesVector[i]) * m_vertexNormals[i];
				m_vertexNormals[i] = normalize(m_vertexNormals[i]);
			}


		}

}

void Mesh::init () {
	glCreateBuffers (1, &m_posVbo); // Generate a GPU buffer to store the positions of the vertices
	size_t vertexBufferSize = sizeof (glm::vec3) * m_vertexPositions.size (); // Gather the size of the buffer from the CPU-side vector
	glNamedBufferStorage (m_posVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT); // Create a data store on the GPU
	glNamedBufferSubData (m_posVbo, 0, vertexBufferSize, m_vertexPositions.data ()); // Fill the data store from a CPU array

	glCreateBuffers (1, &m_normalVbo); // Same for normal
	glNamedBufferStorage (m_normalVbo, vertexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_normalVbo, 0, vertexBufferSize, m_vertexNormals.data ());

	glCreateBuffers (1, &m_texCoordVbo); // Same for texture coordinates
	size_t texCoordBufferSize = sizeof (glm::vec2) * m_vertexTexCoords.size ();
	glNamedBufferStorage (m_texCoordVbo, texCoordBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_texCoordVbo, 0, texCoordBufferSize, m_vertexTexCoords.data ());

	glCreateBuffers (1, &m_ibo); // Same for the index buffer, that stores the list of indices of the triangles forming the mesh
	size_t indexBufferSize = sizeof (glm::uvec3) * m_triangleIndices.size ();
	glNamedBufferStorage (m_ibo, indexBufferSize, NULL, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferSubData (m_ibo, 0, indexBufferSize, m_triangleIndices.data ());

	glCreateVertexArrays (1, &m_vao); // Create a single handle that joins together attributes (vertex positions, normals) and connectivity (triangles indices)
	glBindVertexArray (m_vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, m_posVbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (1);
	glBindBuffer (GL_ARRAY_BUFFER, m_normalVbo);
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), 0);
	glEnableVertexAttribArray (2);
	glBindBuffer (GL_ARRAY_BUFFER, m_texCoordVbo);
	glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_ibo);
	glBindVertexArray (0); // Desactive the VAO just created. Will be activated at rendering time.
}

void Mesh::render () {
	glBindVertexArray (m_vao); // Activate the VAO storing geometry data
	glDrawElements (GL_TRIANGLES, static_cast<GLsizei> (m_triangleIndices.size () * 3), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

void Mesh::clear () {
	m_vertexPositions.clear ();
	m_vertexNormals.clear ();
	m_vertexTexCoords.clear ();
	m_triangleIndices.clear ();
	if (m_vao) {
		glDeleteVertexArrays (1, &m_vao);
		m_vao = 0;
	}
	if(m_posVbo) {
		glDeleteBuffers (1, &m_posVbo);
		m_posVbo = 0;
	}
	if (m_normalVbo) {
		glDeleteBuffers (1, &m_normalVbo);
		m_normalVbo = 0;
	}
	if (m_texCoordVbo) {
		glDeleteBuffers (1, &m_texCoordVbo);
		m_texCoordVbo = 0;
	}
	if (m_ibo) {
		glDeleteBuffers (1, &m_ibo);
		m_ibo = 0;
	}
}
