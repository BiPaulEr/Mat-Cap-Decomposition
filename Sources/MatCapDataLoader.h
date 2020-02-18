#pragma once
#include <vector>
#include <fstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include "stb_image.h"

class MatCapDataLoader
{
public:
	std::string Path;
	std::string HighBack_Path;
	std::string HighFront_Path;
	std::string LowBack_Path;
	std::string LowFront_Path;
	std::vector<float> Parameters;
	std::vector<std::vector<float>> Parameters_Path;
	GLuint HighBack;
	GLuint HighFront;
	GLuint LowBack;
	GLuint LowFront;
	//GLuint inNormals;
	float HighFreqVariance;
	float LowFreqVariance;
	float  LowFreqTheta0;
	float  HighFreqTheta0;
	float LowFreqM1;
	float HighFreqM1;
	float LowFreqAlpha1;
	float HighFreqAlpha1;
	float rotLightTheta;
	float rotLightPhi;
	glm::vec3  LowFreqBaseColor;
	glm::vec3 HighFreqBaseColor;


	MatCapDataLoader(int i,std::string Path);
	~MatCapDataLoader();
	void TextureLoader();
	GLuint loadTextureFromFileToGPU(const std::string & filename);
	void LoadInitialAngleofElevation();
	void LoadSteepness();
	void LoadFinalAlphaValue();
	void LoadBaseColor();
	void LoadRotation();
	void LoadVariance();
	void LoadNormals();
	/*glm::vec2 CoordsCartToSphere(glm::vec3 xyz);
	glm::vec2 CoordsSpheresToDual(glm::vec2 thetatphi,bool Front);*/
	

};

