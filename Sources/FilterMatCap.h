#pragma once

#include <glad/glad.h>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>


#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>

/***************************************/
//Pour utiliser l'objet ulisé :
//FilterMatCap FilterMat;
//std::vector<float> listVariance = FilterMat.GetVarianceForPoint(3.14 / 4, "filename", 20, 10, 10);
/**************************************/



class FilterMatCap {
public:
		bool begin = false;
		~FilterMatCap ();
		FilterMatCap();
		 glm::vec2 inverse_orthographic_projection(glm::vec2 xy);
		 glm::vec3 alpha_0;
		 void FindEnergy(const std::string & filename);
		 float  standardDeviation(std::vector<float> samples);
		 float Gaussian(float stdderivation, float t);
		 glm::vec3 Interpolation(float x, float y, const std::string & filename);
		 std::vector<float> sizeWindow;
		 std::vector<float> sizeWindowList(int nbrPoints);
		 std::vector<float> attrWindow(float thetha, int nbrPoints, float sizeWindow);
		 std::vector<std::vector<float>> listValueForThethaAndSizeWindow(float sizeWindow, float thetha, const std::string & filename,int nbrPointsSizeFort,int nbrPointsForphi);
		 float fromListToVariance(std::vector<std::vector<float>> List);
		 std::vector<float> GetVarianceForPoint(float thetha,  const std::string & filename, int nbrPointsSizeWindow, int nbrPointsFort, int nbrPointsForPhi);
		 std::vector<float> deriveList(std::vector<float> List,float dx);
		 void FilterMatCap::save_Data(const char *text);
		 glm::vec2 revelantWindowSize(std::vector<float> ListVariance);
		 std::pair<std::pair<bool, int>, std::pair<bool, int>> findInflexions(std::vector<float> ListVariance,int nbrPointsSizeWindow);
		 glm::vec2 cartesian_to_dual_parabloid(glm::vec3 xyz);
		 glm::vec3 dual_parabloid_to_cartesian(glm::vec2 xy);
		 glm::vec3 sphere_to_cartesian(glm::vec2 thethaphi);
		 glm::vec2 orthographic_projection(glm::vec2 thetaphi);
}
;
