#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "Transform.h"

class LightSource : public Transform {

private:
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
  float coneAngle;
  //coeff d'attenuation radiale
  //float ar
  //coeff d'attenuation constante
  float ac;
  //coeff d'attenuation lineaire
  float al;
  //coeff d'attenuation quadratique
  float aq;

public:

  ~LightSource() {}
  LightSource() : Transform () {
    position = glm::vec3(10.0,10.0,10.0);
    color = glm::vec3(0.0,1.0,0.0);
    intensity = 1.0;
    coneAngle = 1.0;
    //ar = 0.0;
    ac = 1.0;
    al = 0.0;
    aq = 0.0;
  }

  LightSource(glm::vec3 position, glm::vec3 color, float intensity, float coneAngle, float ac, float al, float aq) : Transform(),
  position(position), color(color), intensity(intensity), coneAngle(coneAngle), ac(ac), al(al), aq(aq) {}

  glm::vec3 getPosition() {return position;}
  glm::vec3 getColor() {return color;}
  float getIntensity() {return intensity;}
  float getConeAngle() {return coneAngle;}
  float getAc() {return ac;}
  float getAl() {return al;}
  float getAq() {return aq;}

  void setPosition(glm::vec3 position) {this->position = position;}
  void setColor(glm::vec3 color) {this->color = color;}
  void setIntensity(float intensity) {this->intensity = intensity;}
  void setConeAngle(float coneAngle) {this->coneAngle = coneAngle;}
  void setAc(float ac) {this->ac = ac;}
  void setAl(float al) {this->al = al;}
  void setAq(float aq) {this->aq = aq;}
};








#endif //LIGHTSOURCE_H
