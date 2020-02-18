#version 450 core // Minimal GL version support expected from the GPU

struct LightSource {
	vec3 position;
	vec3 color;
	float intensity;
};

uniform LightSource lightSource;

struct Material {
	vec3 albedo;
	float kd;
};


float G1(vec3 n,vec3 w,float alpha) {
	float prodScar = clamp(dot(n,w), 0, 1);
	float prodScarCarre = pow(prodScar,2);
	float alphaCarre = pow(alpha,2);
	return 2 * prodScar/(prodScar + pow( alphaCarre + (1 - alphaCarre) * prodScarCarre, 0.5));

}

vec3 F(vec3 F0, vec3 wi, vec3 wh) {
	float prodScar = clamp(dot(wi,wh), 0, 1);
	float max = max(0, prodScar);
	return F0 + pow(1 - max, 5) * (vec3(1.0) - F0);
}

float D(float alpha, vec3 n, vec3 wh) {
	float alphaCarre = pow (alpha, 2);
	float prodScar = clamp( dot(n, wh), 0, 1);
	float prodScarCarre = pow( prodScar, 2);
	return alphaCarre/(3.14 * pow(1 + (alphaCarre - 1) * prodScarCarre , 2));
}

//retourne la partie spéculaire d'une lumière
vec3 fs(float alpha, vec3 n, vec3 wo, vec3 wi, vec3 whi, vec3 F0) {
	float D1 = D(alpha, n, whi);
	vec3 F1 = F(F0, wi, whi);
	float G11 = G1(n, wi, alpha) * G1(n, wo, alpha);
	vec3 fs1 = D1*F1*G11/(4 * clamp(dot(n,wi), 0, 1) * clamp(dot(n,wo), 0, 1));
	return fs1;
}

//retourne la lumière "finale" à partir d'une source de lumière
vec3 finalLight(float alpha, vec3 n, vec3 wo, vec3 wi, vec3 whi, vec3 F0, LightSource lightSource, vec3 fPosition) {
	float D1 = D(alpha, n, whi);
	vec3 F1 = F(F0, wi, whi);
	float G11 = G1(n, wi, alpha) * G1(n, wo, alpha);
	vec3 L1 = lightSource.color * lightSource.intensity;
	float d1 = distance(lightSource.position, fPosition);
	//L1 = (1/(lightSource.ac + lightSource.al * d1 + lightSource.aq * d1 * d1)) * L1;
	return L1;
}

uniform Material material;

in vec3 fPosition; // Shader input, linearly interpolated by default from the previous stage (here the vertex shader)
in vec3 fNormal;
in vec2 fTexCoord;

out vec4 colorResponse; // Shader output: the color response attached to this fragment

void main() {
/*
	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wo = normalize (-fPosition);
	vec3 fr = material.albedo;
	vec3 Li = lightSource.color * lightSource.intensity;
	vec3 radiance =  Li * fr;
  colorResponse = vec4 (radiance, 1.0); // Building an RGBA value from an RGB one.
*/

//test

	vec3 n = normalize (fNormal); // Linear barycentric interpolation does not preserve unit vectors

	//pour la suite, ne pas oublier de mettre
	// n entre 0 et 1
	vec3 wi = normalize (lightSource.position - fPosition);
	vec3 wo = normalize (-fPosition);
	vec3 wh = normalize(wi+wo);
	float alpha = 1.0;
	vec3 F0 = vec3(0.04);
	vec3 albedo = vec3(0.4,0.8,0.2);
	float occlusion = 1.0;
	vec3 fd = albedo;
	vec3 fs = fs( alpha, n, wo, wi, wh, F0);
	vec3 L1 = finalLight(alpha, n, wo, wi, wh, F0, lightSource, fPosition);


	vec3 radiance = L1 * (fs + fd) * clamp(dot(n,wi), 0, 1);

	colorResponse = vec4(radiance, 1.0);


}
