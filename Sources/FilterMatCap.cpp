

#include "FilterMatCap.h"
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>
#include "stb_image.h"
#include "stb_image_write.h"
#include <cmath>
#include <algorithm>

using namespace std;

FilterMatCap::~FilterMatCap() {
	
}
//constructeur
FilterMatCap::FilterMatCap() {
	int i =100;

}


//effectue la transformation orthographique pour x,y incult dans [0,1]
glm::vec2 FilterMatCap::inverse_orthographic_projection(glm::vec2 xy) {

	xy[0] = 2*xy[0]-1;
	xy[1] = 2 * xy[1] - 1;
	float rho = pow(dot(xy, xy),0.5);
	if (rho > 1) {
		std::cout << "xy not in range : Transformation avorted" << std::endl;
	}
	float c = asin(rho);
	float phi = asin((xy[1]*sin(c)) / rho);
	float theta = atan((xy[0]*sin(c)) / (rho*cos(c)));
	return glm::vec2(theta, phi);

}


//converti le thetha et le phi en coordonée entre [0,1]
glm::vec2 FilterMatCap::orthographic_projection(glm::vec2 thetaphi) {
	if ((abs(thetaphi[0]) > (3.14/2))) {

		std::cout << "theta or phi not in range : Transformation avorted" << std::endl;
	}
	float x = cos(thetaphi[1])*sin(thetaphi[0]);
	float y = sin(thetaphi[0])*sin(thetaphi[1]);
	x = (x + 1) / 2;
	y = (y + 1) / 2;
	return (glm::vec2(x, y));
}

//from [0,1] to [-1,1] to [x,y,z]
glm::vec3 FilterMatCap::dual_parabloid_to_cartesian(glm::vec2 xy) { 
	float x = (xy[0] * 2) - 1;
	float y = (xy[1] * 2) - 1;
	float z_sum = (pow(x, 2) + pow(y, 2) + 1);
	float rx = (2 * x) / z_sum;
	float ry = (2 * y) / z_sum;
	float rz = (-1 + pow(x, 2) + pow(y, 2)) / z_sum;
	return glm::vec3(rx, ry, rz);
}

//from [x,y,z] to [-1,1] to [0,1]
glm::vec2 FilterMatCap::cartesian_to_dual_parabloid(glm::vec3 xyz) {
	float x = xyz[0] / (1 - xyz[2]);
	float y = xyz[1] / (1 - xyz[2]);
	x = (x + 1) / 2;
	y = (y + 1) / 2;
	return glm::vec2(x, y);
}

//from [theta,phi] to [x,y,z] 
glm::vec3 FilterMatCap::sphere_to_cartesian(glm::vec2 thethaphi) {
	float x = sin(thethaphi[0])*cos(thethaphi[1]);
	float y = sin(thethaphi[0])*sin(thethaphi[1]);
	float z = -cos(thethaphi[0]);
	return glm::vec3(x, y, z);
}

// trouve la basecolor
void FilterMatCap::FindEnergy(const std::string & filename) {
	int width, height, numComponents;
	unsigned char * data = stbi_load(filename.c_str(),
		&width,
		&height,
		&numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
		0);
	float theta;
	float phi_tmp;
	glm::vec2 nvT;
	float lumi = 0;
	float lumi_tmp = 0;
	glm::vec3 R_tmp;
	glm::vec3 R;
	unsigned char* pixelOffset;
	for (int i = 0; i < 600;i++) {
		for (int j = 0;j < 600;j++) {
			pixelOffset = data + (i + 600 * j) * 3;
			R_tmp = glm::vec3(pixelOffset[0], pixelOffset[1], pixelOffset[2]);		
			lumi_tmp = (0.2126*R_tmp[0] + 0.7152*R_tmp[1] + 0.0722*R_tmp[2]);
			if (lumi_tmp > lumi) {
				lumi = lumi_tmp;
				R = R_tmp;
			}
		}	
		}
	std::cout << " lumi : " << lumi << " avec " << R[0] << " " << R[1] << " " << R[2] << std::endl;
}


//return la standard deviation à partir d'une liste de float
float FilterMatCap::standardDeviation(std::vector<float> samples)
{
	int size = samples.size();

	float variance = 0;
	float t = samples[0];
	for (int i = 1; i < size; i++)
	{
		t += samples[i];
		float diff = ((i + 1) * samples[i]) - t;
		variance += (diff * diff) / ((i + 1.0) *i);
	}
	return pow(variance / (size - 1),0.5);
}


//return la valeur gausienne avec la troncature à std/3
float FilterMatCap::Gaussian(float stddeviation,float t){
	float std = stddeviation /3;
	if (abs(t) > stddeviation) {
		return 0;
	}
	else {
		return (1 / (std*pow(2 * 3.14, 0.5))*exp(-0.5*pow((t / std), 2)));
	}
}


// return la vraie valeur glm::vec3 provenant de l'image avec en entrée ses coordonnées en float
glm::vec3 FilterMatCap::Interpolation(float x, float y, const std::string & filename) {
	if (x > 600 || y > 600 ) {
		std::cout << "Error limits for interpolation" << std::endl;
	}
	int width, height, numComponents;
	unsigned char * data = stbi_load(filename.c_str(),
		&width,
		&height,
		&numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
		0);
	int x_ = int(x);
	int y_ = int(y);
	float dx = x - x_;
	float dy = y - y_;
	
	unsigned char* pixelOffset = data + (x_ + 600 * y_) * 3;
	glm::vec3 F1 = glm::vec3(pixelOffset[0], pixelOffset[1], pixelOffset[2]);
	pixelOffset = data + (x_+1  + 600 * y_) * 3;
	glm::vec3 F2 = glm::vec3(pixelOffset[0], pixelOffset[1], pixelOffset[2]);
	pixelOffset = data + (x_ + 600 * (y_+1)) * 3;
	glm::vec3 F3 = glm::vec3(pixelOffset[0], pixelOffset[1], pixelOffset[2]);
	pixelOffset = data + (x_+1 + 600 * (y_+1)) * 3;
	glm::vec3 F4 = glm::vec3(pixelOffset[0], pixelOffset[1], pixelOffset[2]);

	glm::vec3 ret=(((F2 - F1)*dx + (F3 - F1)*dy + F1 + (F1 + F2 - F3 - F2)*dx*dy));
	return ret;
}

//return liste des size de window que l'on test de [0,PI/2]
std::vector<float> FilterMatCap::sizeWindowList(int nbrPoints) {
	float delta = 3.14 / (2 * (nbrPoints-1));
	std::vector<float> List;
	
	char buffer[64];
	save_Data("Taille Window : \n ");
	for (int i = 1; i < nbrPoints; i++) {
		List.push_back(delta*i);
		sprintf(buffer, "%f", delta*i);
		save_Data(buffer);
		save_Data("\n");
	}
	return List;
}


// return liste de vlaurs valables pour theta la taille de la fenêtre 
std::vector<float> FilterMatCap::attrWindow(float thetha, int nbrPoints,float sizeWindow) {
	float borne1 = -(3.14/2) - thetha;
	float borne2 = (3.14/2) -  thetha;
	if (borne1 < borne2) {
		float tmp = borne2;
		borne2 = borne1;
		borne1 = tmp;
	}

	float delta =(2*sizeWindow)/ (nbrPoints - 1);
	std::vector<float> List;
	for (int i = 1; i < nbrPoints; i++) {
		if (borne2 < delta*i) {
			if (delta*i < borne1) {
				List.push_back(delta*i);
			}
		}
	}
	return List;
}


//return liste de R(theta + t) avec le size de la fenêtre fixé
std::vector<std::vector<float>> FilterMatCap::listValueForThethaAndSizeWindow(float sizeWindow, float thetha, const std::string & filename,int nbrPointsFort,int nbrPointsForPhi) {
	std::vector<float> Listx;
	std::vector<float> Listy;
	std::vector<float> Listz;
	float List_x=0;
	float List_y=0;
	float List_z=0;
	std::vector<float> attr = FilterMatCap::attrWindow(thetha, nbrPointsFort, sizeWindow);
	glm::vec2 nvT;
	glm::vec3 R_tmp;
	glm::vec3 R_t=glm::vec3(0,0,0);
	float phi_tmp;
	for (int i = 0; i < attr.size();i++) {
		for (int j = 1;j < nbrPointsForPhi;j++) {
			phi_tmp = (j * 2 * 3.139) / nbrPointsForPhi;
			//tranformation des coordonnées pour récupérer cela dans la texture
			nvT = cartesian_to_dual_parabloid(sphere_to_cartesian(glm::vec2(attr[i] + thetha, phi_tmp)));
			R_tmp = FilterMatCap::Interpolation(nvT[0] * 600, nvT[1] * 600, filename);
			R_tmp = R_tmp * FilterMatCap::Gaussian(sizeWindow, attr[i]);
			R_t = R_t + R_tmp;
		}
		List_x = List_x + pow(R_t[0] / (nbrPointsForPhi - 2), 2);
		List_y = List_y + pow(R_t[1] / (nbrPointsForPhi - 2), 2);
		List_z = List_z + pow(R_t[2] / (nbrPointsForPhi - 2), 2);
		
		Listx.push_back(0.2126*R_t[0] / (nbrPointsForPhi - 2) );
		Listy.push_back(0.7152*R_t[1]/ (nbrPointsForPhi - 2));
		Listz.push_back(0.0722*R_t[2] / (nbrPointsForPhi - 2));
	}
	for (int i = 0; i < Listx.size();i++) {
		Listx[i] = Listx[i] / List_x;
		Listy[i] = Listy[i] / List_y;
		Listz[i] = Listz[i] / List_z;
	}	
	std::vector<std::vector<float>> List;
	List.push_back(Listx);
	List.push_back(Listy);
	List.push_back(Listz);
	return List;
}

//return la variance pour une liste de trois liste de float
float FilterMatCap::fromListToVariance(std::vector<std::vector<float>> List) {
	if (List[0].size() == 0 || List[1].size() == 0 || List[2].size() == 0) {
		std::cout << "Error size List = null so not varaince" << std::endl;
		return 0;
	}
	float v1 = FilterMatCap::standardDeviation(List[0]);
	float v2 = FilterMatCap::standardDeviation(List[1]);
	float v3 = FilterMatCap::standardDeviation(List[2]); 
	return ((v1 + v2 + v3) / 3);
}


//prend en entré le theta d'un point sur la sphere et ecrit dans test.txt les résultats
std::vector<float> FilterMatCap::GetVarianceForPoint(float thetha, const std::string & filename, int nbrPointsSizeWindow,int nbrPointsFort, int nbrPointsForPhi) {
	std::vector<float> Listsize = FilterMatCap::sizeWindowList(nbrPointsSizeWindow);
	std::vector<std::vector<float>> tmp; 
	std::vector<float> listVariance;
	float Var_tmp;
	char buffer[64];
	save_Data("Variance: \n ");
	
	for (int i = 0; i < Listsize.size();i++) {
		
		tmp = FilterMatCap::listValueForThethaAndSizeWindow(Listsize[i], thetha, filename,nbrPointsFort, nbrPointsForPhi);
		Var_tmp = FilterMatCap::fromListToVariance(tmp);
		std::cout << Listsize[i] << std::endl;
		Var_tmp = (Var_tmp * pow((Listsize[i] / 3), 2)) / (Var_tmp - pow((Listsize[i] / 3), 2));
		listVariance.push_back(Var_tmp);
		sprintf(buffer, "%f", Var_tmp);
		save_Data(buffer);
		save_Data("\n");
	}
	return listVariance;
}

// dx est l'écart suivant x entre deux variables. Ici c'est bien constant puisque l'abscisse est la variation de la taille de la fenêtre de troncature
std::vector<float> FilterMatCap::deriveList(std::vector<float> List,float dx) {
	if (List.size() < 3) {
		std::cout << "Derivate fault. List toot small" << std::endl;
	}
		std::cout << "derivate : " << std::endl;
	std::vector<float> derivation;
	char buffer[64];
	save_Data("Derivate : \n");
	for (int i = 1; i < List.size() - 1;i++) {
		derivation.push_back((List[i - 1] + List[i] + List[i + 1]) / (2 * dx));
		std::cout << derivation[i-1] << std::endl;
		sprintf(buffer, "%f", derivation[i - 1]);
		save_Data(buffer);
		save_Data("\n");
	}
	return derivation;
}

//fonction qui permet d'enregistrer des données dans test.txt sans écrasement 
void FilterMatCap::save_Data(const char *text) {
	FILE* fichier;
	if (!begin) {
		fichier = fopen("test.txt", "w+");
		begin = true;
	}
	else {
		fichier = fopen("test.txt", "a+");
	}
	fputs(text, fichier);
	fclose(fichier);
}


// censer retourner la size du filtre idéale
glm::vec2 FilterMatCap::revelantWindowSize(std::vector<float> ListVariance) {
	return glm::vec2(0, 0);
}

//cherche les deux inlfexions recherchées pour trouver v 
std::pair<std::pair<bool, int>, std::pair<bool, int>> FilterMatCap::findInflexions(std::vector<float> ListVariance, int nbrPointsSizeWindow){
	float dx = 3.14 / nbrPointsSizeWindow;
	std::vector<float> derivate = deriveList(ListVariance, dx);
	std::vector<float> derivate2 = deriveList(derivate, dx);
	bool find1 = false;
	int infl1;
	int infl2;
	bool find2 = false;
	bool signe = true;
	if (derivate[0] < 0) {
		signe = false;
	}
	int i = 1;
	while ((!find1) && (i<(ListVariance.size()-2))){
		if (signe) {
			if (derivate[i] <= 0) {
				find1 = true;
				infl1 = i + 1;
			}	
		}
		else {
			if (derivate[i] >= 0) {
				find1 = true;
				infl1 = i + 1;
			}
		}
		i++;
	}
	signe = true;
	if (derivate2[0] < 0) {
		signe = false;
	}
    i = 1;
	while ((!find2) && (i < (ListVariance.size() - 4))) {
		if (signe) {
			if (derivate2[i] <= 0) {
				find2 = true;
				infl2 = i + 2;
			}
		}
		else {
			if (derivate2[i] >= 0) {
				find2 = true;
				infl2 = i + 2;
			}
		}
		i=i+1;
	}
	
	std::pair<std::pair<bool, float>, std::pair<bool, float>> result;
	result = make_pair(make_pair(find1, infl1), make_pair(find2, infl2));
	char buffer[150];
	std::cout << "1er inflexion : " << result.first.first << " 2e inflexion : " << result.second.first << std::endl;
	sprintf(buffer,"First inflexion = %f. \n Second inflexion = %f. ", result.first.first,result.second.first);
	save_Data(buffer);
	
	
	return result;
}




