#include "camera.hpp"

#include <iostream>

using namespace COL781;
namespace GL = COL781::OpenGL;
using namespace glm;


// void initializeScene() {
// 	object = r.createObject();
// 	vertices[0] = vec3(0, 0, 1);
// 	vertices[1] = vec3(1, 0, 1);
// 	vertices[2] = vec3(1, 0, 0);
// 	vertices[3] = vec3(0, 0, 0);
//	vertexBuf = r.createVertexAttribs(object, 0, nv, vertices);
// 	normals[0] = vec3(0, 0, 1);
// 	normals[1] = vec3(0, 0, 1);
// 	normals[2] = vec3(0, 0, 1);
// 	normals[3] = vec3(0, 0, 1);
// 	normalBuf = r.createVertexAttribs(object, 1, nv, normals);
// 	triangles[0] = ivec3(0, 1, 2);
// 	triangles[1] = ivec3(0, 2, 3);
// 	r.createTriangleIndices(object, nt, triangles);
// }

// void updateScene(float t) {
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

// int main() {
// 	int width = 640, height = 480;
// 	if (!r.initialize("Animation", width, height)) {
// 		return EXIT_FAILURE;
// 	}
// 	camCtl.initialize(width, height);
// 	camCtl.camera.setCameraView(vec3(0.5, -0.5, 1.5), vec3(0.5, -0.5, 0.0), vec3(0.0, 1.0, 0.0));
// 	program = r.createShaderProgram(
// 		r.vsBlinnPhong(),
// 		r.fsBlinnPhong()
// 	);

// 	initializeScene();

// 	while (!r.shouldQuit()) {
//         float t = SDL_GetTicks64()*1e-3;
// 		updateScene(t);

// 		camCtl.update();
// 		Camera &camera = camCtl.camera;

// 		r.clear(vec4(1.0, 1.0, 1.0, 1.0));
// 		r.enableDepthTest();
// 		r.useShaderProgram(program);

// 		r.setUniform(program, "model", glm::mat4(1.0));
// 		r.setUniform(program, "view", camera.getViewMatrix());
// 		r.setUniform(program, "projection", camera.getProjectionMatrix());
// 		r.setUniform(program, "lightPos", camera.position);
// 		r.setUniform(program, "viewPos", camera.position);
// 		r.setUniform(program, "lightColor", vec3(1.0f, 1.0f, 1.0f));

// 		r.setupFilledFaces();
//         glm::vec3 orange(1.0f, 0.6f, 0.2f);
//         glm::vec3 white(1.0f, 1.0f, 1.0f);
//         r.setUniform(program, "ambientColor", 0.4f*orange);
//         r.setUniform(program, "diffuseColor", 0.9f*orange);
//         r.setUniform(program, "specularColor", 0.8f*white);
//         r.setUniform(program, "phongExponent", 100.f);
// 		r.drawObject(object);

// 		r.setupWireFrame();
//         glm::vec3 black(0.0f, 0.0f, 0.0f);
//         r.setUniform(program, "ambientColor", black);
//         r.setUniform(program, "diffuseColor", black);
//         r.setUniform(program, "specularColor", black);
//         r.setUniform(program, "phongExponent", 0.f);
// 		r.drawObject(object);

// 		r.show();
// 	}
// }


#include <iostream>
#include <vector>
#include <glm/glm.hpp>

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

class ClothSimulation {
public:
	ClothSimulation(float width, float height, int numParticlesX, int numParticlesY, float massDensity, float structuralStiffness, float shearStiffness, float bendingStiffness) {
		this->width = width;
		this->height = height;
		this->numParticlesX = numParticlesX;
		this->numParticlesY = numParticlesY;
		this->massDensity = massDensity;
		this->structuralStiffness = structuralStiffness;
		this->shearStiffness = shearStiffness;
		this->bendingStiffness = bendingStiffness;

		// Calculate particle mass based on mass density and cloth dimensions
		float totalMass = massDensity * width * height;
		float particleMass = totalMass / (numParticlesX * numParticlesY);

		// Create particles
		particles.resize(numParticlesX * numParticlesY);
		for (int i = 0; i < numParticlesX; i++) {
			for (int j = 0; j < numParticlesY; j++) {
				Particle& particle = particles[i * numParticlesY + j];
				particle.position = glm::vec3((i / (float)(numParticlesX - 1)) * width, 0, (j / (float)(numParticlesY - 1)) * height);
				particle.velocity = glm::vec3(0);
				particle.force = glm::vec3(0);
				particle.mass = particleMass;
				particle.isFixed = (i == 0 && j == 0) || (i == numParticlesX - 1 && j == 0); // Fix two adjacent corners
			}
		}

		// Create springs
		createStructuralSprings();
		createShearSprings();
		createBendingSprings();
	}

	void simulate(float deltaTime) {
		applyGravity();
		applySprings();
		integrate(deltaTime);
	}

	const std::vector<Particle>& getParticles() const {
		return particles;
	}

private:
	float width;
	float height;
	int numParticlesX;
	int numParticlesY;
	float massDensity;
	float structuralStiffness;
	float shearStiffness;
	float bendingStiffness;
	std::vector<Particle> particles;
	std::vector<Spring> structuralSprings;
	std::vector<Spring> shearSprings;
	std::vector<Spring> bendingSprings;

	void createStructuralSprings() {
		for (int i = 0; i < numParticlesX; i++) {
			for (int j = 0; j < numParticlesY; j++) {
				if (i < numParticlesX - 1) {
					Spring spring;
					spring.particle1 = i * numParticlesY + j;
					spring.particle2 = (i + 1) * numParticlesY + j;
					spring.restLength = width / (numParticlesX - 1);
					spring.stiffness = structuralStiffness;
					spring.damping = 0.1f * structuralStiffness;
					structuralSprings.push_back(spring);
				}
				if (j < numParticlesY - 1) {
					Spring spring;
					spring.particle1 = i * numParticlesY + j;
					spring.particle2 = i * numParticlesY + (j + 1);
					spring.restLength = height / (numParticlesY - 1);
					spring.stiffness = structuralStiffness;
					spring.damping = 0.1f * structuralStiffness;
					structuralSprings.push_back(spring);
				}
			}
		}
	}

	void createShearSprings() {
		for (int i = 0; i < numParticlesX - 1; i++) {
			for (int j = 0; j < numParticlesY - 1; j++) {
				Spring spring1;
				spring1.particle1 = i * numParticlesY + j;
				spring1.particle2 = (i + 1) * numParticlesY + (j + 1);
				spring1.restLength = glm::length(glm::vec3(width / (numParticlesX - 1), 0, height / (numParticlesY - 1)));
				spring1.stiffness = shearStiffness;
				spring1.damping = 0.1f * shearStiffness;
				shearSprings.push_back(spring1);

				Spring spring2;
				spring2.particle1 = (i + 1) * numParticlesY + j;
				spring2.particle2 = i * numParticlesY + (j + 1);
				spring2.restLength = glm::length(glm::vec3(width / (numParticlesX - 1), 0, height / (numParticlesY - 1)));
				spring2.stiffness = shearStiffness;
				spring2.damping = 0.1f * shearStiffness;
				shearSprings.push_back(spring2);
			}
		}
	}

	void createBendingSprings() {
		for (int i = 0; i < numParticlesX - 2; i++) {
			for (int j = 0; j < numParticlesY - 2; j++) {
				Spring spring1;
				spring1.particle1 = i * numParticlesY + j;
				spring1.particle2 = (i + 2) * numParticlesY + j;
				spring1.restLength = 2 * width / (numParticlesX - 1);
				spring1.stiffness = bendingStiffness;
				spring1.damping = 0.1f * bendingStiffness;
				bendingSprings.push_back(spring1);

				Spring spring2;
				spring2.particle1 = i * numParticlesY + j;
				spring2.particle2 = i * numParticlesY + (j + 2);
				spring2.restLength = 2 * height / (numParticlesY - 1);
				spring2.stiffness = bendingStiffness;
				spring2.damping = 0.1f * bendingStiffness;
				bendingSprings.push_back(spring2);
			}
		}
	}

	void applyGravity() {
		glm::vec3 gravity(0, -9.8f, 0);
		for (Particle& particle : particles) {
			if (!particle.isFixed) {
				particle.force += particle.mass * gravity;
			}
		}
	}

	void applySprings() {
		for (Spring& spring : structuralSprings) {
			Particle& particle1 = particles[spring.particle1];
			Particle& particle2 = particles[spring.particle2];
			glm::vec3 deltaPosition = particle2.position - particle1.position;
			glm::vec3 deltaVelocity = particle2.velocity - particle1.velocity;
			glm::vec3 force = -spring.stiffness * (glm::length(deltaPosition) - spring.restLength) * glm::normalize(deltaPosition) - spring.damping * deltaVelocity;
			particle1.force += force;
			particle2.force -= force;
		}

		for (Spring& spring : shearSprings) {
			Particle& particle1 = particles[spring.particle1];
			Particle& particle2 = particles[spring.particle2];
			glm::vec3 deltaPosition = particle2.position - particle1.position;
			glm::vec3 deltaVelocity = particle2.velocity - particle1.velocity;
			glm::vec3 force = -spring.stiffness * (glm::length(deltaPosition) - spring.restLength) * glm::normalize(deltaPosition) - spring.damping * deltaVelocity;
			particle1.force += force;
			particle2.force -= force;
		}

		for (Spring& spring : bendingSprings) {
			Particle& particle1 = particles[spring.particle1];
			Particle& particle2 = particles[spring.particle2];
			glm::vec3 deltaPosition = particle2.position - particle1.position;
			glm::vec3 deltaVelocity = particle2.velocity - particle1.velocity;
			glm::vec3 force = -spring.stiffness * (glm::length(deltaPosition) - spring.restLength) * glm::normalize(deltaPosition) - spring.damping * deltaVelocity;
			particle1.force += force;
			particle2.force -= force;
		}
	}

	void integrate(float deltaTime) {
		for (Particle& particle : particles) {
			if (!particle.isFixed) {
				glm::vec3 acceleration = particle.force / particle.mass;
				particle.velocity += acceleration * deltaTime;
				particle.position += particle.velocity * deltaTime;
				particle.force = glm::vec3(0);
			}
		}
	}
};



int main() {
	float clothWidth = 5.0f;
	float clothHeight = 5.0f;
	int numParticlesX = 2;
	int numParticlesY = 2;
	float massDensity = 1.0f;
	float structuralStiffness = 100.0f;
	float shearStiffness = 50.0f;
	float bendingStiffness = 10.0f;

	ClothSimulation simulation(clothWidth, clothHeight, numParticlesX, numParticlesY, massDensity, structuralStiffness, shearStiffness, bendingStiffness);

	float deltaTime = 0.0001f;
	float totalTime = 2.0f;
	int numSteps = totalTime / deltaTime;

	// save frames for animation
	std::vector<std::vector<Particle>> frames;
	for (int i = 0; i < numSteps; i++) {
		simulation.simulate(deltaTime);
		// Render the cloth or perform other visualization
		// For example, you can print the positions of the particles
		for (const Particle& particle : simulation.getParticles()) {
			std::cout << "Particle position: (" << particle.position.x << ", " << particle.position.y << ", " << particle.position.z << ")" << std::endl;
		}
		frames.push_back(simulation.getParticles());
		std::cout << std::endl;
	}



	GL::Rasterizer r;
	GL::ShaderProgram program;
	const int nv = numParticlesX * numParticlesY;
	const int nt = (numParticlesX - 1) * (numParticlesY - 1) * 2;
	vec3 vertices[nv];
	vec3 normals[nv];
	ivec3 triangles[nt];

	GL::Object object;
	GL::AttribBuf vertexBuf, normalBuf;

	CameraControl camCtl;

	std::cout << "Rendering" << std::endl;

	int width = 640, height = 480;
	if (!r.initialize("Animation", width, height)) {
		return EXIT_FAILURE;
	}
	camCtl.initialize(width, height);
	camCtl.camera.setCameraView(vec3(-20, -20, -20), vec3(0.5, -0.5, 0.0), vec3(0.0, 1.0, 0.0));
	program = r.createShaderProgram(
		r.vsBlinnPhong(),
		r.fsBlinnPhong()
	);

	object = r.createObject();

	std::cout << "Rendering!" << std::endl;

// 	object = r.createObject();
// 	vertices[0] = vec3(0, 0, 1);
// 	vertices[1] = vec3(1, 0, 1);
// 	vertices[2] = vec3(1, 0, 0);
// 	vertices[3] = vec3(0, 0, 0);
// 	vertexBuf = r.createVertexAttribs(object, 0, nv, vertices);
// 	normals[0] = vec3(0, 0, 1);
// 	normals[1] = vec3(0, 0, 1);
// 	normals[2] = vec3(0, 0, 1);
// 	normals[3] = vec3(0, 0, 1);
// 	normalBuf = r.createVertexAttribs(object, 1, nv, normals);
// 	triangles[0] = ivec3(0, 1, 2);
// 	triangles[1] = ivec3(0, 2, 3);
// 	r.createTriangleIndices(object, nt, triangles);

	int i=0;
	while (!r.shouldQuit()) {
		std::cout<<"2\n";

		for (int j = 0; j < nv; j++) {
			vertices[j] = frames[i][j].position;
		}

		std::cout<<"3\n";
		vertexBuf = r.createVertexAttribs(object, 0, nv, vertices);
		std::cout<<"3.5\n";
		for (int j = 0; j < nv; j++) {
			normals[j] = glm::normalize(glm::cross(vertices[(j + 1) % nv] - vertices[j], vertices[(j + numParticlesY) % nv] - vertices[j]));
		}

		// // print vertex buffer
		// for (int j = 0; j < nv; j++) {
		// 	std::cout << "Vertex " << j << ": (" << vertices[j].x << ", " << vertices[j].y << ", " << vertices[j].z << ")" << std::endl;
		// }
		std::cout<<"4\n";


		normalBuf = r.createVertexAttribs(object, 1, nv, normals);
		for (int j = 0; j < nt; j++) {
			triangles[j] = ivec3(j, j + 1, j + 2);
		}
		std::cout<<"5\n";
		r.createTriangleIndices(object, nt, triangles);

		std::cout << "Rendering!!!!" << std::endl;


		r.updateVertexAttribs(vertexBuf, nv, vertices);
		r.updateVertexAttribs(normalBuf, nv, normals);
		
		// print vertexBuf
		for (int j = 0; j < nv; j++) {
			std::cout << "Vertex " << j << ": (" << vertices[j].x << ", " << vertices[j].y << ", " << vertices[j].z << ")" << std::endl;
		}


		// float t = SDL_GetTicks64() * 1e-3;
		// update the scene according to ith frame
		// no function call, just update the vertices and normals

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
		i++;
		i%=numSteps;
	}
	


	return 0;
}

