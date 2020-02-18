#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <glm/ext.hpp>

//ajout
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Material {
private:
  glm::vec3 albedo;
  float kd;

  //rugosité
  float alpha;

  glm::vec3 F0;


  //ajout d'un compteur pour compter à chaque fois que l'on fait "bindVertex"
  int compteur = 0;

public:
  Material() : albedo(glm::vec3(0.4,0.6,0.2)), kd(1.0), alpha(0.2), F0(glm::vec3(0.03)) {}
  ~Material() {}

  GLuint loadTextureFromFileToGPU(const std::string & filename) {
    int width, height, numComponents;
    // Loading the image in CPU memory using stbd_image
    unsigned char * data = stbi_load (filename.c_str (),
    &width,
    &height,
    &numComponents, // 1 for a 8 bit greyscale image, 3 for 24bits RGB image
    0);
    // Create a texture in GPU memory
    GLuint texID;
    glGenTextures (1, &texID);
    glBindTexture (GL_TEXTURE_2D, texID);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Uploading the image data to GPU memory
    glTexImage2D (GL_TEXTURE_2D,
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
      glBindTexture (GL_TEXTURE_2D, compteur);
      compteur++;

      return texID;
    }

    float getKd() {return kd;}
    glm::vec3 getAlbedo() {return albedo;}
    float getAlpha() {return alpha;}
    glm::vec3 getF0() {return F0;}

    void setKd(float kd) {this->kd = kd;}
    void setAlbedo(glm::vec3 albedo) {this->albedo = albedo;}
    void setAlpha(float alpha) {this->alpha = alpha;}
    void setF0(glm::vec3 F0) {this->F0 = F0;}
  };


  #endif //Material
