#include "lab_m1/Tema1/Tema1.h"

#include <iostream>
#include <cmath>

using namespace std;

pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_circle(glm::vec3 color, bool full) {
	double center_X = 0;
	double center_Y = 0;
	double start_angle = full ? 2 * AI_MATH_PI : AI_MATH_PI;
	double end_angle = 0;

	int divisions = full ? 64 : 32;
	double angle_step = (end_angle - start_angle) / divisions;

	vector<double> Xes;
	vector<double> Yes;

	vector<VertexFormat> vertices;
	vector<unsigned int> indexes;

	for (int i = 0; i <= divisions; i++) {
		Xes.push_back(cos(start_angle + i * angle_step));
		Yes.push_back(sin(start_angle + i * angle_step));
		if (i == 0) {
			vertices.push_back(VertexFormat(glm::vec3(Xes[i], Yes[i], 0), glm::vec3(0.2, 0.2, 0.2)));
		}
		else {
			vertices.push_back(VertexFormat(glm::vec3(Xes[i], Yes[i], 0), color));
		}	
		if (i > 0 && i < divisions) {
			indexes.push_back(i);
			indexes.push_back(i + 1);
			indexes.push_back(0);
		}
		
	}

	return { vertices, indexes };
}

pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_square(glm::vec3 color, glm::vec3 center_color, double scaleX, double scaleY) {
	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0, 0), color),
		VertexFormat(glm::vec3(0, scaleY, 0), color),
		VertexFormat(glm::vec3(scaleX, 0, 0), color),
		VertexFormat(glm::vec3(scaleX, scaleY, 0), color),
		VertexFormat(glm::vec3(0.5 * scaleX, 0.5 * scaleY, 0), center_color)
	};

	vector<unsigned int> indexes = {
		0, 1, 4,
		1, 3, 4,
		0, 2, 4,
		2, 3, 4
	};

	return { vertices, indexes };
}

pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_K(glm::vec3 color) {
	glm::vec3 center_color = glm::vec3(color.x - 0.2, color.y - 0.2, color.z - 0.2);
	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0, 0), color),
		VertexFormat(glm::vec3(0, 1, 0), color),
		VertexFormat(glm::vec3(1, 0, 0), color),
		VertexFormat(glm::vec3(1, 1, 0), color),
		VertexFormat(glm::vec3(0.5, 0.5, 0), center_color),
	};

	vector<unsigned int> indexes = {
		0, 1, 4,
		0, 2, 4,
		1, 3, 4,
	};

	return { vertices, indexes };
}

pair<vector<VertexFormat>, vector<unsigned int>> create_endpoints_fire(glm::vec3 color1, glm::vec3 color2) {
	vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0, 0), color1),
		VertexFormat(glm::vec3(-0.3f, -1.1f, 0), color2),
		VertexFormat(glm::vec3(0.05f, -0.7f, 0), color1),
		VertexFormat(glm::vec3(0.3f, -1.35f, 0), color2),
		VertexFormat(glm::vec3(0.6f, -0.8f, 0), color1),
		VertexFormat(glm::vec3(0.7f, -1.2f, 0), color2),
		VertexFormat(glm::vec3(1, -1, 0), color1),
		VertexFormat(glm::vec3(1.2f, -1.4f, 0), color2),
		VertexFormat(glm::vec3(1, 0, 0), color1),
	};

	vector<unsigned int> indexes = {
		0, 1, 2,
		0, 2, 8,
		2, 3, 4,
		2, 4, 8,
		4, 5, 6,
		4, 6, 8,
		6, 7, 8,
	};

	return { vertices, indexes };
}
