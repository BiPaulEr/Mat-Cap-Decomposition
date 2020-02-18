#include "MatCapDataLoader.h"

#include <string>
#include <vector>
#include <fstream>



MatCapDataLoader::MatCapDataLoader(int i, std::string Path_)
{
	Path = Path_;
	HighBack_Path= Path+"MatCap0"+ std::to_string(i)+"_high_back.jpg";
	HighFront_Path=Path+"MatCap0" + std::to_string(i) + "_high_front.jpg";
	LowBack_Path= Path+"MatCap0" + std::to_string(i) + "_low_back.jpg";
	LowFront_Path= Path+"MatCap0" + std::to_string(i) + "_low_front.jpg";
	std::vector <float> Mat1 = { 0.491, 139, 92, 44,0.059 ,113,154,78};
	std::vector <float> Mat2 = { 0.38, 137, 131, 101,0.058 ,87,101,133 };
	std::vector <float> Mat3 = { 0.319, 116, 99, 108,0.094 ,103,134,105 };
	Parameters_Path.push_back(Mat1);
	Parameters_Path.push_back(Mat2);
	Parameters_Path.push_back(Mat3);
	Parameters = Parameters_Path[i - 1];
	MatCapDataLoader::LoadSteepness();
	MatCapDataLoader::LoadFinalAlphaValue();
	MatCapDataLoader::LoadInitialAngleofElevation();
	MatCapDataLoader::LoadVariance();
	MatCapDataLoader::LoadRotation();
	MatCapDataLoader::LoadBaseColor();
	MatCapDataLoader::TextureLoader();
}
void MatCapDataLoader::LoadSteepness() {
	
	LowFreqM1 = 0;
	HighFreqM1 = 10;
}
void MatCapDataLoader::LoadFinalAlphaValue() {
	
	LowFreqAlpha1 = 0.5;
	HighFreqAlpha1 = 0.75;
}
void MatCapDataLoader::LoadInitialAngleofElevation()
{
	LowFreqTheta0 = 0;
	HighFreqTheta0 = 1.56;
}
void MatCapDataLoader::LoadVariance() {
	
	LowFreqVariance = Parameters[0];
	HighFreqVariance = Parameters[4];
}
void MatCapDataLoader::LoadRotation() {
	rotLightTheta = 3.14;
	rotLightPhi = 3.14;
}


void MatCapDataLoader::LoadBaseColor()
{
    glm::vec3 r= glm::vec3(Parameters[1], Parameters[2], Parameters[3]);
	glm::vec3 s= glm::vec3(Parameters[5], Parameters[6], Parameters[7]);
	LowFreqBaseColor = r;
	HighFreqBaseColor = s;
}


MatCapDataLoader::~MatCapDataLoader()
{
	

}
void MatCapDataLoader::TextureLoader() {
	HighBack = loadTextureFromFileToGPU(HighBack_Path);
	HighFront = loadTextureFromFileToGPU(HighFront_Path);
	LowBack = loadTextureFromFileToGPU(LowBack_Path);
	LowFront = loadTextureFromFileToGPU(HighFront_Path);
}
GLuint MatCapDataLoader::loadTextureFromFileToGPU(const std::string & filename) {
	int width, height, numComponents;
	// Loading the image in CPU memory using stbd_image
	unsigned char * data = stbi_load(filename.c_str(),
		&width,
		&height,
		&numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
		0);
	// Create a texture in GPU memory
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Uploading the image data to GPU memory
	glTexImage2D(GL_TEXTURE_2D,
		0,
		(numComponents == 1 ? GL_RED : GL_RGB), // We assume only greyscale or RGB pixels
		width,
		height,
		0,
		(numComponents == 1 ? GL_RED : GL_RGB), // We assume only greyscale or RGB pixels
		GL_UNSIGNED_BYTE,
		data);
	// Generating mipmaps for filtered texture fetch
	glGenerateMipmap(GL_TEXTURE_2D);
	// Freeing the now useless CPU memory
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;
}
void MatCapDataLoader::LoadNormals() {

}
/* glm::vec2 MatCapDataLoader::CoordsCartToSphere(glm::vec3 xyz) {
	x = xyz[0];
	y = xyz[1];
	z = xyz[2];
	float theta = std::atan(sqrt(x*x + y * y) / z);
	float phi = std::atan(y / x);
	return glm::vec2(theta,phi)
}
glm::vec2 MatCapDataLoader::CoordsSphereToDual(glm::vec2 thetaphi,bool Front) {
	float rx = 2 * s(s*s + t * t + 1);
	float ry = 2 * t*(s*s + t * t + 1);
	
	if (!Front) {
		rx = -1 * rx;
		ry = -1 * ry;
	
	}
	return glm::vec2(rx, ry);
	
} */