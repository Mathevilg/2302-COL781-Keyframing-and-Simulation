#include "camera.hpp"

#include <iostream>

#include <vector>

using namespace COL781;
namespace GL = COL781::OpenGL;
using namespace glm;

GL::Rasterizer r;
GL::ShaderProgram program;


GL::Object object;
GL::AttribBuf vertexBuf, normalBuf;

CameraControl camCtl;


struct Particle {
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 force;
	float mass;
	bool isFixed;
};

struct Spring {
	int particle1;
	int particle2;
	float restLength;
	float stiffness;
	float damping;
};



void initializeScene(vec3* vertices, vec3* normals, ivec3* triangles, std::vector<Particle> &particles, int numParticlesX, int numParticlesY) {
	int nv = numParticlesX*numParticlesY;
	int nt = 2*(numParticlesX-1)*(numParticlesY-1);
	object = r.createObject();
	
	// set vertices 
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			int p = i*numParticlesY + j;
			vertices[p] = particles[p].position;
		}
	}

	vertexBuf = r.createVertexAttribs(object, 0, nv, vertices);
	
	// set normals
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			int p = i*numParticlesY + j;
			normals[p] = vec3(0, 0, 1);
		}
	}

	normalBuf = r.createVertexAttribs(object, 1, nv, normals);

	// set triangles
	int t = 0;
	for (int i = 0; i < numParticlesX-1; i++) {
		for (int j = 0; j < numParticlesY-1; j++) {
			triangles[t++] = ivec3(i*numParticlesY + j, i*numParticlesY + j+1, (i+1)*numParticlesY + j);
			triangles[t++] = ivec3(i*numParticlesY + j+1, (i+1)*numParticlesY + j+1, (i+1)*numParticlesY + j);
		}
	}

	r.createTriangleIndices(object, nt, triangles);
}

void updateScene(float t, vec3* vertices, vec3* normals, std::vector<Particle>& particles, std::vector<Spring> springs, int numParticlesX, int numParticlesY){
	// 	apply Gravity and update particles
	for (int i = 0; i < particles.size(); i++) {
		if (!particles[i].isFixed) {
			particles[i].force += vec3(0, 0, -9.8) * particles[i].mass; // Apply gravity
		}
	}
	std::cout<<"Gravity applied!\n";
	// apply spring forces
	// for (const Spring& spring : springs) {
	// 	const Particle& particle1 = particles[spring.particle1];
	// 	const Particle& particle2 = particles[spring.particle2];
	// 	std::cout << "1\n";
	// 	// Calculate the direction and distance between the particles
	// 	glm::vec3 direction = particle2.position - particle1.position;
	// 	float distance = glm::length(direction);
	// 	std::cout << "2\n";

	// 	// Calculate the spring force
	// 	glm::vec3 force = -spring.stiffness * (distance - spring.restLength) * glm::normalize(direction);
	// 	std::cout << "3\n";

	// 	// Apply the spring force to both particles
	// 	particles[spring.particle1].force += force;
	// 	particles[spring.particle2].force -= force;
	// 	std::cout << "4\n";

	// 	// Apply damping force
	// 	glm::vec3 relativeVelocity = particle2.velocity - particle1.velocity;
	// 	glm::vec3 dampingForce = -spring.damping * glm::dot(relativeVelocity, direction) / distance * glm::normalize(direction);
	// 	std::cout << "5\n";

	// 	// Apply the damping force to both particles
	// 	particles[spring.particle1].force += dampingForce;
	// 	particles[spring.particle2].force -= dampingForce;
	// 	std::cout << "6\n";

	// }
	std::cout<<"Spring forces applied!\n";

	// update position and velocity (according to t) and set force = 0
	for (int i = 0; i < particles.size(); i++) {
		if (!particles[i].isFixed) {
			// Update position and velocity
			particles[i].position += particles[i].velocity * t;
			particles[i].velocity += (particles[i].force / particles[i].mass) * t;
			// Reset force to zero
			particles[i].force = glm::vec3(0.0f);
		}
		else std::cout << "Particle " << i << " is fixed\n";
	}
	std::cout<<"Positions and Velocities updated!\n";


	// update vertices and normals accordingly
	int nv = numParticlesX*numParticlesY;
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			int p = i * numParticlesY + j;
			vertices[p] = particles[p].position;
		}
	}
	vertexBuf = r.createVertexAttribs(object, 0, nv, vertices);
	r.updateVertexAttribs(vertexBuf, nv, vertices);

	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			int p = i * numParticlesY + j;
			normals[p] = vec3(0, 0, 1);
		}
	}
	normalBuf = r.createVertexAttribs(object, 1, nv, normals);
	r.updateVertexAttribs(normalBuf, nv, normals);
}
// {
// 	float freq = 2, amp = 1;
// 	float phase0 = 0, phase1 = 0.5;
// 	float theta0 = amp*cos(freq*t + phase0), theta1 = amp*cos(freq*t + phase1);
// 	vertices[0] = vec3(0, -cos(theta0), sin(theta0));
// 	vertices[1] = vec3(1, -cos(theta1), sin(theta1));
// 	r.updateVertexAttribs(vertexBuf, nv, vertices);
// 	normals[0] = glm::normalize(glm::cross(vertices[1]-vertices[0], vertices[3]-vertices[0]));
// 	normals[1] = glm::normalize(glm::cross(vertices[2]-vertices[1], vertices[0]-vertices[1]));
// 	normals[2] = glm::normalize(glm::cross(vertices[3]-vertices[2], vertices[1]-vertices[2]));
// 	normals[3] = glm::normalize(glm::cross(vertices[0]-vertices[3], vertices[2]-vertices[3]));
// 	r.updateVertexAttribs(normalBuf, nv, normals);
// }

std::vector<Spring> setSprings(std::vector<Particle> &particles, int numParticlesX, int numParticlesY, float width, float height) {
	std::vector<Spring> springs;
	float structuralStiffness = 100.0f;
	float shearStiffness = 50.0f;
	float bendingStiffness = 10.0f;
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			if (i < numParticlesX - 1) {
				Spring spring;
				spring.particle1 = i * numParticlesY + j;
				spring.particle2 = (i + 1) * numParticlesY + j;
				spring.restLength = width / (numParticlesX - 1);
				spring.stiffness = structuralStiffness;
				spring.damping = 0.1f * structuralStiffness;
				springs.push_back(spring);
			}
			if (j < numParticlesY - 1) {
				Spring spring;
				spring.particle1 = i * numParticlesY + j;
				spring.particle2 = i * numParticlesY + (j + 1);
				spring.restLength = height / (numParticlesY - 1);
				spring.stiffness = structuralStiffness;
				spring.damping = 0.1f * structuralStiffness;
				springs.push_back(spring);
			}
		}
	}
	std::cout<<"Structural springs set!\n";
	std::cout<<"Number of springs: "<<springs.size()<<std::endl;
	for (int i = 0; i < numParticlesX - 1; i++) {
		for (int j = 0; j < numParticlesY - 1; j++) {
			Spring spring1;
			spring1.particle1 = i * numParticlesY + j;
			spring1.particle2 = (i + 1) * numParticlesY + (j + 1);
			spring1.restLength = glm::length(glm::vec3(width / (numParticlesX - 1), 0, height / (numParticlesY - 1)));
			spring1.stiffness = shearStiffness;
			spring1.damping = 0.1f * shearStiffness;
			springs.push_back(spring1);

			Spring spring2;
			spring2.particle1 = (i + 1) * numParticlesY + j;
			spring2.particle2 = i * numParticlesY + (j + 1);
			spring2.restLength = glm::length(glm::vec3(width / (numParticlesX - 1), 0, height / (numParticlesY - 1)));
			spring2.stiffness = shearStiffness;
			spring2.damping = 0.1f * shearStiffness;
			springs.push_back(spring2);
		}
	}
	std::cout<<"Number of springs: "<<springs.size()<<std::endl;
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			if (i < numParticlesX - 2) {
				Spring spring;
				spring.particle1 = i * numParticlesY + j;
				spring.particle2 = (i + 2) * numParticlesY + j;
				spring.restLength = 2 * width / (numParticlesX - 1);
				spring.stiffness = bendingStiffness;
				spring.damping = 0.1f * bendingStiffness;
				springs.push_back(spring);
			}
			if (j < numParticlesY - 2) {
				Spring spring;
				spring.particle1 = i * numParticlesY + j;
				spring.particle2 = i * numParticlesY + (j + 2);
				spring.restLength = 2 * height / (numParticlesY - 1);
				spring.stiffness = bendingStiffness;
				spring.damping = 0.1f * bendingStiffness;
				springs.push_back(spring);
			}
		}
	}
	std::cout<<"Springs set!\n";
	std::cout<<"Number of springs: "<<springs.size()<<std::endl;
	

	return springs;
}

std::vector<Particle> setParticles(int numParticlesX, int numParticlesY, float massDensity, float clothWidth, float clothHeight) {
	std::vector<Particle> particles;
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			Particle p;
			p.position = vec3(i*clothWidth/(numParticlesX-1), j*clothHeight/(numParticlesY-1), 0);
			p.velocity = vec3(0, 0, 0);
			p.force = vec3(0, 0, 0);
			p.mass = massDensity*clothWidth*clothHeight/(numParticlesX*numParticlesY);
			p.isFixed = (i == 0 && j == 0) || (i == numParticlesX-1 && j == numParticlesY-1); // Fix two adjacent corners
			particles.push_back(p);
		}
	}
	return particles;
}

int main() {
	int width = 640, height = 480;
	if (!r.initialize("Animation", width, height)) {
		return EXIT_FAILURE;
	}
	camCtl.initialize(width, height);
	camCtl.camera.setCameraView(vec3(-10, -10, -10), vec3(0.5, -0.5, 0.0), vec3(0.0, 1.0, 0.0));
	program = r.createShaderProgram(
		r.vsBlinnPhong(),
		r.fsBlinnPhong()
	);

	float clothWidth = 5.0f;
	float clothHeight = 5.0f;
	int numParticlesX = 4;
	int numParticlesY = 3;
	float massDensity = 1.0f;
	float structuralStiffness = 100.0f;
	float shearStiffness = 50.0f;
	float bendingStiffness = 10.0f;

	int nv = numParticlesX*numParticlesY;
	int nt = 2*(numParticlesX-1)*(numParticlesY-1);
	vec3 vertices[nv];
	vec3 normals[nv];
	ivec3 triangles[nt];


	std::vector<Particle> particles = setParticles(numParticlesX, numParticlesY, massDensity, clothWidth, clothHeight);
	// print particle positions
	for (int i = 0; i < numParticlesX; i++) {
		for (int j = 0; j < numParticlesY; j++) {
			int p = i*numParticlesY + j;
			std::cout << "Particle " << p << " position: " << particles[p].position.x << " " << particles[p].position.y << " " << particles[p].position.z << std::endl;
		}
	}


	std::vector<Spring> springs = setSprings(particles, numParticlesX, numParticlesY, clothWidth, clothHeight);

	// set vertices, normals and triangles for the cloth according to the particles
	initializeScene(vertices, normals, triangles, particles, numParticlesX, numParticlesY);

	while (!r.shouldQuit()) {
        float t = 1e-3;
		updateScene(t, vertices, normals, particles, springs, numParticlesX, numParticlesY);
		// print vertices
		// for (int i = 0; i < nv; i++) {
		// 	std::cout << "Vertex " << i << " position: " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << std::endl;
		// }

		camCtl.update();
		Camera &camera = camCtl.camera;

		r.clear(vec4(1.0, 1.0, 1.0, 1.0));
		r.enableDepthTest();
		r.useShaderProgram(program);

		r.setUniform(program, "model", glm::mat4(1.0));
		r.setUniform(program, "view", camera.getViewMatrix());
		r.setUniform(program, "projection", camera.getProjectionMatrix());
		r.setUniform(program, "lightPos", camera.position);
		r.setUniform(program, "viewPos", camera.position);
		r.setUniform(program, "lightColor", vec3(1.0f, 1.0f, 1.0f));

		r.setupFilledFaces();
        glm::vec3 orange(1.0f, 0.6f, 0.2f);
        glm::vec3 white(1.0f, 1.0f, 1.0f);
        r.setUniform(program, "ambientColor", 0.4f*orange);
        r.setUniform(program, "diffuseColor", 0.9f*orange);
        r.setUniform(program, "specularColor", 0.8f*white);
        r.setUniform(program, "phongExponent", 100.f);
		r.drawObject(object);

		r.setupWireFrame();
        glm::vec3 black(0.0f, 0.0f, 0.0f);
        r.setUniform(program, "ambientColor", black);
        r.setUniform(program, "diffuseColor", black);
        r.setUniform(program, "specularColor", black);
        r.setUniform(program, "phongExponent", 0.f);
		r.drawObject(object);

		r.show();
	}
}
