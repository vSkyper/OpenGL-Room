#include "Shader_Loader.h"
#include "Texture.h"
#include "SOIL/SOIL.h"

#include "room_declarations.hpp"

const unsigned int SHADOW_WIDTH = 2024, SHADOW_HEIGHT = 2024;

int WIDTH = 900, HEIGHT = 900;

GLuint depthMapFBO;
GLuint depthMap;
GLuint depthMapShipFBO;
GLuint depthMapShip;

GLuint bloomTextureFBO;
unsigned int colorBuffers[2];
unsigned int blurFBO[2];
unsigned int blurBuffer[2];
bool horizontal = true;

GLuint program;
GLuint programRoomLamp;
GLuint programTest;
GLuint programDepth;
GLuint programSkybox;
GLuint programBlur;
GLuint programBloom;
GLuint programFlames;
GLuint programFireParticles;

Core::Shader_Loader shaderLoader;

glm::vec3 sunPos = glm::vec3(-4.740971f, 2.149999f, 0.369280f);
glm::vec3 sunDir = glm::vec3(-0.93633f, 0.351106, 0.003226f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f) * 5;

glm::vec3 cameraPos = glm::vec3(0.479490f, 1.250000f, -2.124680f);
glm::vec3 cameraDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);

glm::vec3 spaceshipPos = glm::vec3(-0.181308f, 0.791063f, -1.625526f);
glm::vec3 spaceshipDir = glm::vec3(0.028363f, 0.000000f, 0.999599f);

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 pointlightPos = glm::vec3(0, 1.89622, 0.0008);
glm::vec3 pointlightColor = glm::vec3(0.9, 0.6, 0.6);

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9) * 3;

glm::vec3 firePos = glm::vec3(1.20635f, 0.587633f, 1.027260f);

float lastTime = -1.f;
float deltaTime = 0.f;

glm::mat4 lightVP = glm::ortho(-3.f, 3.f, -3.f, 3.f, 1.0f, 20.0f) * glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0, 1, 0));

glm::mat4 lightShipVP;

int tv_interaction = 0;
int keypressed = 0;

struct Particle {
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec4 Color;
	float Life;

	Particle()
		: Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

namespace fire {
	unsigned int nr_particles = 1000;
	std::vector<Particle> particles;
	unsigned int lastUsedParticle = 0;
}

void initParticles() {
	for (unsigned int i = 0; i < fire::nr_particles; i++)
		fire::particles.push_back(Particle());
}

unsigned int firstUnusedParticle()
{
	for (unsigned int i = fire::lastUsedParticle; i < fire::nr_particles; i++) {
		if (fire::particles[i].Life <= 0.0f) {
			fire::lastUsedParticle = i;
			return i;
		}
	}

	for (unsigned int i = 0; i < fire::lastUsedParticle; i++) {
		if (fire::particles[i].Life <= 0.0f) {
			fire::lastUsedParticle = i;
			return i;
		}
	}

	fire::lastUsedParticle = 0;
	return 0;
}

void respawnParticle(Particle& particle, glm::vec3 offset)
{
	float randomX = -((rand() % 100)) / 5000.0;
	float randomY = ((rand() % 200) - 100) / 500.0;
	float randomZ = ((rand() % 200) - 100) / 660.0;

	float rColor = 0.8f + ((rand() % 100) / 1000.0f);
	float gColor = 0.10f + ((rand() % 100) / 1000.0f);

	particle.Position = glm::vec3(randomX, randomY, randomZ) + offset;
	particle.Color = glm::vec4(rColor, gColor, 0.0, 1.0f);
	particle.Life = 1.0f;
	particle.Velocity = glm::vec3(0.1f);
}

void updateParticles() {
	unsigned int nr_new_particles = 100;

	for (unsigned int i = 0; i < nr_new_particles; i++)
	{
		int unusedParticle = firstUnusedParticle();
		respawnParticle(fire::particles[unusedParticle], glm::vec3(0.0, 0.1, 0.0));
	}

	for (unsigned int i = 0; i < fire::nr_particles; i++)
	{
		Particle& p = fire::particles[i];
		p.Life -= deltaTime * 3.0;
		if (p.Life > 0.0f)
		{
			p.Position.y += p.Velocity.y * deltaTime;
			p.Color.a -= deltaTime * 1.5;

			if (p.Color.a <= 0.0)
				p.Color.a = 0.01;
		}
	}
}

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}

glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide, cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix(float scale = 1.0)
{
	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;

	float aspectRatioLocal = aspectRatio;

	if (scale != 1.0) {
		aspectRatioLocal = 1.0;
	}

	perspectiveMatrix = glm::mat4({
		1 * scale,0,0.,0.,
		0.,aspectRatioLocal * scale,0.,0.,
		0.,0.,(f + n) / (n - f),2 * f * n / (n - f),
		0.,0.,-1.,0.,
		});


	perspectiveMatrix = glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, GLuint texture = texture::defaultTexture, GLuint textureNormal = texture::defaultTextureNormal, GLuint textureArm = texture::defaultTextureArm) {
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program, "depthMap"), 0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniformMatrix4fv(glGetUniformLocation(program, "lightVP"), 1, GL_FALSE, (float*)&lightVP);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(program, "depthMapShip"), 1);
	glBindTexture(GL_TEXTURE_2D, depthMapShip);
	glUniformMatrix4fv(glGetUniformLocation(program, "lightShipVP"), 1, GL_FALSE, (float*)&lightShipVP);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunPos"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);

	glUniform3f(glGetUniformLocation(program, "firePos"), firePos.x, firePos.y, firePos.z);

	Core::SetActiveTexture(texture, "colorTexture", program, 2);
	Core::SetActiveTexture(textureNormal, "normalSampler", program, 3);
	Core::SetActiveTexture(textureArm, "armSampler", program, 4);
	Core::DrawContext(context);
}

void drawObjectDepth(Core::RenderContext& context, glm::mat4 viewProjection, glm::mat4 model) {
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glUniformMatrix4fv(glGetUniformLocation(programDepth, "viewProjectionMatrix"), 1, GL_FALSE, (float*)&viewProjection);
	glUniformMatrix4fv(glGetUniformLocation(programDepth, "modelMatrix"), 1, GL_FALSE, (float*)&model);
	Core::DrawContext(context);
	//glDisable(GL_CULL_FACE);
}

void renderShadowMap(GLuint depthFBO, glm::mat4 light) {
	float time = glfwGetTime();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(programDepth);

	drawObjectDepth(models::sphereContext, light, glm::translate(pointlightPos) * glm::scale(glm::vec3(0.115)));

	drawObjectDepth(models::lampContext, light, glm::mat4());

	drawObjectDepth(models::doorContext, light, glm::mat4());

	drawObjectDepth(models::planeContext, light, glm::mat4());

	drawObjectDepth(models::roomContext, light, glm::mat4());

	drawObjectDepth(models::windowContext, light, glm::mat4());

	drawObjectDepth(models::commodeContext, light, glm::mat4());

	drawObjectDepth(models::tvTableContext, light, glm::mat4());

	drawObjectDepth(models::tvContext, light, glm::mat4());

	drawObjectDepth(models::sofaContext, light, glm::mat4());

	drawObjectDepth(models::tableContext, light, glm::mat4());

	drawObjectDepth(models::guitarContext, light, glm::mat4());

	drawObjectDepth(models::plantContext, light, glm::mat4());

	drawObjectDepth(models::vaseLeftContext, light, glm::mat4());
	drawObjectDepth(models::vaseRightContext, light, glm::mat4());

	drawObjectDepth(models::cakeContext, light, glm::mat4());

	drawObjectDepth(models::deskContext, light, glm::mat4());

	drawObjectDepth(models::chairContext, light, glm::mat4());

	drawObjectDepth(models::paintingContext, light, glm::mat4());

	drawObjectDepth(models::kukriContext, light, glm::mat4());

	drawObjectDepth(models::fireplaceContext, light, glm::mat4());
	drawObjectDepth(models::fireContext, light, glm::translate(glm::vec3(0.03, 0.0, 0.0)));
	drawObjectDepth(models::fireContext, light, glm::mat4());


	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectDepth(models::shipContext,
		light,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f))
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}

void initDepthMap()
{
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initDepthMapShip()
{
	glGenFramebuffers(1, &depthMapShipFBO);
	glGenTextures(1, &depthMapShip);
	glBindTexture(GL_TEXTURE_2D, depthMapShip);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapShipFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapShip, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initBloom()
{
	glGenFramebuffers(1, &bloomTextureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, bloomTextureFBO);

	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
		);
		unsigned int rboDepth;

		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	}

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;


	glGenFramebuffers(2, blurFBO);
	glGenTextures(2, blurBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
		glBindTexture(GL_TEXTURE_2D, blurBuffer[i]);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA16F, WIDTH, HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurBuffer[i], 0
		);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void bluring()
{
	bool first_iteration = true;
	int amount = 20;
	glUseProgram(programBlur);

	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
		glUniform1i(glGetUniformLocation(programBlur, "horizontal"), horizontal);
		glBindTexture(
			GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : blurBuffer[!horizontal]
		);
		Core::SetActiveTexture(first_iteration ? colorBuffers[1] : blurBuffer[!horizontal], "image", programBlur, 5);
		Core::DrawContext(models::testContext);
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawSkybox(glm::mat4 modelMatrix) {
	glDisable(GL_DEPTH_TEST);

	glUseProgram(programSkybox);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;

	glUniformMatrix4fv(glGetUniformLocation(programSkybox, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::SetActiveTexture(texture::cubemapTexture, "skybox", programSkybox, 0);
	glUniform1f(glGetUniformLocation(programSkybox, "exposition"), exposition);

	Core::DrawContext(models::skyboxContext);

	glEnable(GL_DEPTH_TEST);
}

void renderScene(GLFWwindow* window)
{
	glClearColor(0.4f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float time = glfwGetTime();
	updateDeltaTime(time);
	renderShadowMap(depthMapFBO, lightVP);
	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;
	lightShipVP = createPerspectiveMatrix(0.5) * glm::lookAt(spotlightPos, spotlightPos + spotlightConeDir, glm::vec3(0, 1, 0));
	renderShadowMap(depthMapShipFBO, lightShipVP);

	glBindFramebuffer(GL_FRAMEBUFFER, bloomTextureFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, WIDTH, HEIGHT);

	drawSkybox(glm::translate(cameraPos));

	//room lamp
	glUseProgram(programRoomLamp);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(pointlightPos) * glm::scale(glm::vec3(0.115));
	glUniformMatrix4fv(glGetUniformLocation(programRoomLamp, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programRoomLamp, "color"), sunColor.x / 2, sunColor.y / 2, sunColor.z / 2);
	glUniform1f(glGetUniformLocation(programRoomLamp, "exposition"), exposition);
	Core::DrawContext(models::sphereContext);

	glUseProgram(program);

	drawObjectPBR(models::lampContext, glm::mat4(), texture::lampTexture, texture::lampTextureNormal, texture::lampTextureArm);

	drawObjectPBR(models::doorContext, glm::mat4(), texture::doorTexture, texture::doorTextureNormal, texture::doorTextureArm);

	drawObjectPBR(models::planeContext, glm::mat4(), texture::floorTexture, texture::floorTextureNormal, texture::floorTextureArm);

	drawObjectPBR(models::roomContext, glm::mat4(), texture::wallTexture, texture::wallTextureNormal, texture::wallTextureArm);

	drawObjectPBR(models::windowContext, glm::mat4(), texture::windowTexture, texture::windowTextureNormal, texture::windowTextureArm);

	drawObjectPBR(models::commodeContext, glm::mat4(), texture::commodeTexture, texture::commodeTextureNormal, texture::commodeTextureArm);

	drawObjectPBR(models::tvTableContext, glm::mat4(), texture::tvTableTexture, texture::tvTableTextureNormal, texture::tvTableTextureArm);

	drawObjectPBR(models::tvContext, glm::mat4(), texture::tvTexture, texture::tvTextureNormal, texture::tvTextureArm);

	drawObjectPBR(models::sofaContext, glm::mat4(), texture::sofaTexture, texture::sofaTextureNormal, texture::sofaTextureArm);

	drawObjectPBR(models::tableContext, glm::mat4(), texture::tableTexture, texture::tableTextureNormal, texture::tableTextureArm);

	drawObjectPBR(models::guitarContext, glm::mat4(), texture::guitarTexture, texture::guitarTextureNormal, texture::guitarTextureArm);

	drawObjectPBR(models::plantContext, glm::mat4(), texture::plantTexture, texture::plantTextureNormal, texture::plantTextureArm);

	drawObjectPBR(models::vaseLeftContext, glm::mat4(), texture::vaseTexture, texture::vaseTextureNormal, texture::vaseTextureArm);
	drawObjectPBR(models::vaseRightContext, glm::mat4(), texture::vaseTexture, texture::vaseTextureNormal, texture::vaseTextureArm);

	drawObjectPBR(models::cakeContext, glm::mat4(), texture::cakeTexture, texture::cakeTextureNormal, texture::cakeTextureArm);

	drawObjectPBR(models::deskContext, glm::mat4(), texture::deskTexture, texture::deskTextureNormal, texture::deskTextureArm);

	drawObjectPBR(models::chairContext, glm::mat4(), texture::chairTexture, texture::chairTextureNormal, texture::chairTextureArm);

	drawObjectPBR(models::paintingContext, glm::mat4(), texture::paintingTexture, texture::paintingTextureNormal, texture::paintingTextureArm);

	drawObjectPBR(models::kukriContext, glm::mat4(), texture::kukriTexture, texture::kukriTextureNormal, texture::kukriTextureArm);

	drawObjectPBR(models::fireplaceContext, glm::mat4(), texture::fireplaceTexture, texture::fireplaceTextureNormal);
	drawObjectPBR(models::fireContext, glm::translate(glm::vec3(0.03, 0.0, 0.0)), texture::fireplaceBackTexture, texture::fireplaceBackTextureNormal, texture::fireplaceBackTextureArm);

	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectPBR(models::shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f)), texture::spaceshipTexture, texture::spaceshipTextureNormal
	);


	//fire
	glUseProgram(programFlames);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	transformation = viewProjectionMatrix * glm::mat4();
	glUniformMatrix4fv(glGetUniformLocation(programFlames, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform1f(glGetUniformLocation(programFlames, "exposition"), exposition);
	switch ((int)(time * 10) % 4)
	{
	case 0:
		Core::SetActiveTexture(texture::flames1Texture, "colorTexture", programFlames, 10);
		break;
	case 1:
		Core::SetActiveTexture(texture::flames2Texture, "colorTexture", programFlames, 10);
		break;
	case 2:
		Core::SetActiveTexture(texture::flames3Texture, "colorTexture", programFlames, 10);
		break;
	case 3:
		Core::SetActiveTexture(texture::flames4Texture, "colorTexture", programFlames, 10);
		break;
	}
	Core::DrawContext(models::fireContext);

	glUseProgram(programFireParticles);
	updateParticles();
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	for (const Particle& particle : fire::particles)
	{
		if (particle.Life > 0.0f)
		{
			transformation = viewProjectionMatrix * glm::translate(particle.Position);
			glUniformMatrix4fv(glGetUniformLocation(programFireParticles, "transformation"), 1, GL_FALSE, (float*)&transformation);
			glUniform4f(glGetUniformLocation(programFireParticles, "color"), particle.Color.r, particle.Color.g, particle.Color.b, particle.Color.a);
			Core::DrawContext(models::fireParticleContext);
		}
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	bluring();

	glUseProgram(programBloom);
	Core::SetActiveTexture(colorBuffers[0], "scene", programBloom, 6);
	Core::SetActiveTexture(blurBuffer[!horizontal], "bloomBlur", programBloom, 7);
	glUniform1f(glGetUniformLocation(programBloom, "exposition"), exposition);
	Core::DrawContext(models::testContext);

	//test depth buffer
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glUseProgram(programTest);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, depthMapShip);
	//Core::DrawContext(models::testContext);

	glUseProgram(0);
	glfwSwapBuffers(window);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = width;
	HEIGHT = height;
	initBloom();
}

void loadCubemap(char* cubemap[])
{
	glGenTextures(1, &texture::cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture::cubemapTexture);

	int w, h;
	unsigned char* data;
	for (unsigned int i = 0; i < 6; i++)
	{
		data = SOIL_load_image(cubemap[i], &w, &h, 0, SOIL_LOAD_RGBA);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
		);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_main.vert", "shaders/shader_main.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programRoomLamp = shaderLoader.CreateProgram("shaders/shader_room_lamp.vert", "shaders/shader_room_lamp.frag");
	programDepth = shaderLoader.CreateProgram("shaders/shader_depth.vert", "shaders/shader_depth.frag");
	programBlur = shaderLoader.CreateProgram("shaders/shader_blur.vert", "shaders/shader_blur.frag");
	programBloom = shaderLoader.CreateProgram("shaders/shader_bloom.vert", "shaders/shader_bloom.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	programFlames = shaderLoader.CreateProgram("shaders/shader_flames.vert", "shaders/shader_flames.frag");
	programFireParticles = shaderLoader.CreateProgram("shaders/shader_fire_particles.vert", "shaders/shader_fire_particles.frag");

	initDeclarations();

	initDepthMap();
	initDepthMapShip();

	initBloom();

	initParticles();

	char* cubemap[] = { "textures/skybox/posx.jpg", "textures/skybox/negx.jpg", "textures/skybox/posy.jpg", "textures/skybox/negy.jpg", "textures/skybox/posz.jpg", "textures/skybox/negz.jpg" };
	loadCubemap(cubemap);
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.05f * deltaTime * 60;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && glm::dot(glm::vec3(-1.277735f, 0.622864f, -0.097097f), spaceshipDir) > 0.866 && glm::distance(glm::vec3(-1.277735f, 0.622864f, -0.097097f), spaceshipPos) < 0.7) {
		spaceshipPos = glm::vec3(-1.277735f, 0.622864f, -0.097097f);
		spaceshipDir = glm::vec3(1.0f, 0.0f, 0.0f);
	}


	cameraPos = spaceshipPos - 0.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.2f;
	cameraDir = spaceshipDir;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposition += 0.05;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", spaceshipPos.x, spaceshipPos.y, spaceshipPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", spaceshipDir.x, spaceshipDir.y, spaceshipDir.z);
		printf("\n");

	}

	if (glm::distance(glm::vec3(-1.277735f, 0.622864f, -0.097097f), spaceshipPos) < 0.4)
	{
		if (keypressed == 0 && glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		{
			keypressed = 1;
			switch (tv_interaction) {
			case 0:
				texture::tvTexture = Core::LoadTexture("textures/tv/tv_on.png");
				tv_interaction = 1;
				break;
			case 1:
				texture::tvTexture = Core::LoadTexture("textures/tv/tv_off.png");
				tv_interaction = 0;
				break;
			}
		}

		if (keypressed == 1 && glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE)
			keypressed = 0;

	}
}

void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}