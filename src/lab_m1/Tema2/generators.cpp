#include "lab_m1/Tema2/Tema2.h"
#include "transform.h"

#include <vector>
#include <iostream>
#include <cmath>

using namespace std;

// Create vertices for a cube mesh
pair<vector<VertexFormat>, vector<unsigned int>> create_cube_vertices(float scaleX, float scaleY, float scaleZ, glm::vec3 color) {
	float x = scaleX / 2;
	float y = scaleY / 2;
	float z = scaleZ / 2;
	vector<VertexFormat> vertices = {
		// Z+
		VertexFormat(glm::vec3(-x, -y, z), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(x, -y, z), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(x, y, z), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(-x, y, z), color, glm::vec3(0, 0, 1)),

		// Z-
		VertexFormat(glm::vec3(x, -y, -z), color, glm::vec3(0, 0, -1)),
		VertexFormat(glm::vec3(-x, -y, -z), color, glm::vec3(0, 0, -1)),
		VertexFormat(glm::vec3(-x, y, -z), color, glm::vec3(0, 0, -1)),
		VertexFormat(glm::vec3(x, y, -z), color, glm::vec3(0, 0, -1)),

		// X-
		VertexFormat(glm::vec3(-x, -y, -z), color, glm::vec3(-1, 0, 0)),
		VertexFormat(glm::vec3(-x, -y, z), color, glm::vec3(-1, 0, 0)),
		VertexFormat(glm::vec3(-x, y, z), color, glm::vec3(-1, 0, 0)),
		VertexFormat(glm::vec3(-x, y, -z), color, glm::vec3(-1, 0, 0)),

		// X+
		VertexFormat(glm::vec3(x, -y, z), color, glm::vec3(1, 0, 0)),
		VertexFormat(glm::vec3(x, -y, -z), color, glm::vec3(1, 0, 0)),
		VertexFormat(glm::vec3(x, y, -z), color, glm::vec3(1, 0, 0)),
		VertexFormat(glm::vec3(x, y, z), color, glm::vec3(1, 0, 0)),

		// Y+
		VertexFormat(glm::vec3(-x, y, z), color, glm::vec3(0, 1, 0)),
		VertexFormat(glm::vec3(x, y, z), color, glm::vec3(0, 1, 0)),
		VertexFormat(glm::vec3(x, y, -z), color, glm::vec3(0, 1, 0)),
		VertexFormat(glm::vec3(-x, y, -z), color, glm::vec3(0, 1, 0)),

		// Y-
		VertexFormat(glm::vec3(-x, -y, -z), color, glm::vec3(0, -1, 0)),
		VertexFormat(glm::vec3(x, -y, -z), color, glm::vec3(0, -1, 0)),
		VertexFormat(glm::vec3(x, -y, z), color, glm::vec3(0, -1, 0)),
		VertexFormat(glm::vec3(-x, -y, z), color, glm::vec3(0, -1, 0)),
	};

	vector<unsigned int> indices = {
		// Z+
		0, 1, 2,
		2, 3, 0,

		// Z-
		4, 5, 6,
		6, 7, 4,

		// X-
		8, 9, 10,
		10, 11, 8,

		// X+
		12, 13, 14,
		14, 15, 12,

		// Y+
		16, 17, 18,
		18, 19, 16,

		// Y-
		20, 21, 22,
		22, 23, 20
	};

	return { vertices, indices };
}

// Create vertices for a cylinder mesh
pair<vector<VertexFormat>, vector<unsigned int>> create_tube_vertices(float radius, float height, glm::vec3 color) {
	float y = height / 2;
	vector<VertexFormat> vertices;
	vector<unsigned int> indices;

	vertices.push_back(VertexFormat(glm::vec3(0, -y, 0), color, glm::vec3(0, -1, 0)));
	vertices.push_back(VertexFormat(glm::vec3(0, y, 0), color, glm::vec3(0, 1, 0)));

	float angle = 0;
	float step = AI_MATH_TWO_PI / 64;

	for (int i = 0; i <= 64; i++) {
		float x = cos(angle) * radius;
		float z = sin(angle) * radius;

		vertices.push_back(VertexFormat(glm::vec3(x, -y, z), color, glm::vec3(0, -1, 0)));
		vertices.push_back(VertexFormat(glm::vec3(x, y, z), color, glm::vec3(0, 1, 0)));
		vertices.push_back(VertexFormat(glm::vec3(x, -y, z), color, glm::normalize(glm::vec3(x, 0, z))));
		vertices.push_back(VertexFormat(glm::vec3(x, y, z), color, glm::normalize(glm::vec3(x, 0, z))));
		if (i) {
			indices.push_back(0);
			indices.push_back(4 * (i - 1) + 2);
			indices.push_back(4 * i + 2);

			indices.push_back(1);
			indices.push_back(4 * i + 3);
			indices.push_back(4 * (i - 1) + 3);

			indices.push_back(4 * i + 4);
			indices.push_back(4 * (i - 1) + 4);
			indices.push_back(4 * i + 5);

			indices.push_back(4 * (i - 1) + 4);
			indices.push_back(4 * (i - 1) + 5);
			indices.push_back(4 * i + 5);
		}

		angle += step;
	}

	return { vertices, indices };
}

// Create vertices for a pyramid mesh
pair<vector<VertexFormat>, vector<unsigned int>> create_pyramid_vertices(float height, float base, glm::vec3(base_color)) {
	glm::vec3 point_color = base_color + 0.2f;

	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(-base / 2, 0, -base / 2), base_color, glm::normalize(glm::vec3(-1, 0, -1))),
		VertexFormat(glm::vec3(-base / 2, 0, base / 2), base_color, glm::normalize(glm::vec3(-1, 0, 1))),
		VertexFormat(glm::vec3(base / 2, 0, -base / 2), base_color, glm::normalize(glm::vec3(1, 0, -1))),
		VertexFormat(glm::vec3(base / 2, 0, base / 2), base_color, glm::normalize(glm::vec3(1, 0, 1))),
		VertexFormat(glm::vec3(0, height, 0), point_color, glm::vec3(0, 1, 0))
	};

	vector<unsigned int> indices = {
		1, 0, 2,
		0, 2, 3,

		0, 1, 4,
		2, 0, 4,
		3, 2, 4,
		1, 3, 4
	};

	return { vertices, indices };
}

// Create the vertices for the coal texture, needs to be 3D to be rendered on top of the
// stations
pair<vector<VertexFormat>, vector<unsigned int>> create_coal_vertices() {
	glm::vec3 color1 = glm::vec3(0.271, 0.271, 0.239);
	glm::vec3 color2 = glm::vec3(0.09, 0.09, 0.082);

	float z = 0.1;
	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0, -z), color1, glm::vec3(0, 0, -1)),
		VertexFormat(glm::vec3(0, 0, z), color1, glm::vec3(0, 0, 1)),
	};
	vector<unsigned int> indices;

	float angle = 0;
	float step = AI_MATH_TWO_PI / 64;
	constexpr float theta = glm::radians(30.0f);

	for (int i = 0; i <= 64; i++) {
		float x = 0.4 * cos(angle) * cos(theta) - 0.6 * sin(angle) * sin(theta);
		float y = 0.4 * cos(angle) * sin(theta) + 0.6 * sin(angle) * cos(theta);

		vertices.push_back(VertexFormat(glm::vec3(x, y, -z), color2, glm::vec3(0, 0, -1)));
		vertices.push_back(VertexFormat(glm::vec3(x, y, z), color2, glm::vec3(0, 0, 1)));
		vertices.push_back(VertexFormat(glm::vec3(x, y, -z), color2, glm::normalize(glm::vec3(x, y, 0))));
		vertices.push_back(VertexFormat(glm::vec3(x, y, z), color2, glm::normalize(glm::vec3(x, y, 0))));
		if (i) {
			indices.push_back(0);
			indices.push_back(4 * i + 2);
			indices.push_back(4 * (i - 1) + 2);

			indices.push_back(1);
			indices.push_back(4 * (i - 1) + 3);
			indices.push_back(4 * i + 3);

			indices.push_back(4 * i + 4);
			indices.push_back(4 * i + 5);
			indices.push_back(4 * (i - 1) + 4);

			indices.push_back(4 * (i - 1) + 4);
			indices.push_back(4 * i + 5);
			indices.push_back(4 * (i - 1) + 5);
		}

		angle += step;
	}

	auto cube = create_cube_vertices(1, 1, 0.2, color1);
	glm::mat4 newPos = transform::Translate(1, 0, 0);
	add_vertices(cube, make_pair(vertices, indices), newPos);

	return cube;
}

// Create the vertices for the wood texture, needs to be 3D to be rendered on top of the
// stations
pair<vector<VertexFormat>, vector<unsigned int>> create_wood_vertices() {
	glm::vec3 color1 = glm::vec3(0.588, 0.322, 0.075);
	glm::vec3 color2 = glm::vec3(0.4, 0.204, 0.024);

	auto base_front = create_tube_vertices(0.25, 0.2, color1);
	auto log = create_cube_vertices(0.5, 0.195, 1, color2);
	auto base_back = create_tube_vertices(0.25, 0.19, color2);

	glm::mat4 newPos = transform::Translate(0, 0, 0.5);
	add_vertices(base_front, log, newPos);
	newPos = transform::Translate(0, 0, 1);
	add_vertices(base_front, base_back, newPos);

	return base_front;
}

// Create the vertices for the brick texture, needs to be 3D to be rendered on top of the
// stations
pair<vector<VertexFormat>, vector<unsigned int>> create_brick_vertices() {
	glm::vec3 color_red = glm::vec3(0.851, 0.322, 0.239);
	glm::vec3 color_gray = glm::vec3(0.769, 0.722, 0.714);

	auto middle_mortar = create_cube_vertices(1.0, 0.2, 0.2, color_gray);
	auto top_brick = create_cube_vertices(1.0, 0.4, 0.2, color_red);
	auto bottom_mortar = create_cube_vertices(0.2, 0.4, 0.2, color_gray);
	auto bottom_brick = create_cube_vertices(0.4, 0.4, 0.2, color_red);

	glm::mat4 newPos = transform::Translate(0, 0.3, 0);
	add_vertices(middle_mortar, top_brick, newPos);
	newPos = transform::Translate(0, -0.3, 0);
	add_vertices(middle_mortar, bottom_mortar, newPos);
	newPos = transform::Translate(-0.3, -0.3, 0);
	add_vertices(middle_mortar, bottom_brick, newPos);
	newPos = transform::Translate(0.3, -0.3, 0);
	add_vertices(middle_mortar, bottom_brick, newPos);

	return middle_mortar;
}

// Create the vertices for a square (2D)
pair<vector<VertexFormat>, vector<unsigned int>> create_square_vertices(float scaleX, float scaleY, glm::vec3 color) {
	float x = scaleX / 2;
	float y = scaleY / 2;

	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(-x, -y, 0), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(-x, y, 0), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(x, -y, 0), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(x, y, 0), color, glm::vec3(0, 0, 1)),
	};

	vector<unsigned int> indices = {
		0, 1, 2,
		1, 3, 2
	};

	return { vertices, indices };
}

// Create the vertices for a triangle (2D)
// Yes, I will make a mesh that is just a triangle, fight me
pair<vector<VertexFormat>, vector<unsigned int>> create_triangle_vertices(glm::vec3 color) {
	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0.5, 0), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(-0.5, -0.5, 0), color, glm::vec3(0, 0, 1)),
		VertexFormat(glm::vec3(0.5, -0.5, 0), color, glm::vec3(0, 0, 1)),
	};

	vector<unsigned int> indices = {
		0, 1, 2
	};

	return { vertices, indices };
}

// Function that allows adding the vertices of the addV mesh to those of the baseV mesh. Since the indices corresponding
// are added too, this effectively makes the first mesh more complex, by adding a second one to it. Allows creating meshes for trains,
// Stations and many more
void add_vertices(pair<vector<VertexFormat>, vector<unsigned int>>& baseV, pair<vector<VertexFormat>, vector<unsigned int>>& addV, glm::mat4 modelTransform) {
	int baseVertices = baseV.first.size();
	glm::mat4 invTransModel = glm::transpose(glm::inverse(modelTransform));

	for (int i = 0; i < addV.first.size(); i++) {
		glm::vec4 addPos = modelTransform * glm::vec4(addV.first[i].position, 1.0f);
		glm::vec4 addNormal = invTransModel * glm::vec4(addV.first[i].normal, 0.0f);
		baseV.first.push_back(VertexFormat(glm::vec3(addPos.x, addPos.y, addPos.z), addV.first[i].color, glm::vec3(addNormal.x, addNormal.y, addNormal.z)));
	}

	for (int i = 0; i < addV.second.size(); i++) {
		baseV.second.push_back(addV.second[i] + baseVertices);
	}
}