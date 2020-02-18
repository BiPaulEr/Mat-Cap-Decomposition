// ----------------------------------------------
// Base code for practical computer graphics
// assignments.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Error.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "FilterMatCap.h"
#include "MatCapDataLoader.h"


//ajout Material
#include "Material.h"

//ajout pour lecture fichiers .txt
#include <fstream>
using namespace std;


static const std::string SHADER_PATH ("./Resources/Shaders/");

//ajout

static const std::string MATCAP_PATH ("./Resources/MatCapData/");

static const std::string DEFAULT_MESH_FILENAME ("./Resources/Models/face.off");

//ajout
static unsigned int fbo = 0, texture = 0, test = 0;

using namespace std;

// Window parameters
static GLFWwindow * windowPtr = nullptr;

// Pointer to the current camera model
static std::shared_ptr<Camera> cameraPtr;

// Pointer to the displayed mesh
static std::shared_ptr<Mesh> meshPtr;

// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
static std::shared_ptr<ShaderProgram> shaderProgramPtr; // A GPU program contains at least a vertex shader and a fragment shader


static std::shared_ptr<ShaderProgram> MatCapShaderProgramPtr;

//test
FilterMatCap FilterMat;


// Camera control variables
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

void clear ();

void printHelp () {
	std::cout << "> Help:" << std::endl
			  << "    Mouse commands:" << std::endl
			  << "    * Left button: rotate camera" << std::endl
			  << "    * Middle button: zoom" << std::endl
			  << "    * Right button: pan camera" << std::endl
			  << "    Keyboard commands:" << std::endl
   			  << "    * H: print this help" << std::endl
   			  << "    * F1: toggle wireframe rendering" << std::endl
   			  << "    * ESC: quit the program" << std::endl;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height) {
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods)
{
	ImGui_ImplGlfw_KeyCallback(windowPtr, key, scancode, action, mods);
	// if typing text inside ImGui window, don't process keyboard callback
	if (ImGui::GetIO().WantCaptureKeyboard)
		return;

	if (action == GLFW_PRESS && key == GLFW_KEY_H) {
		printHelp ();
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		GLint mode[2];
		glGetIntegerv (GL_POLYGON_MODE, mode);
		glPolygonMode (GL_FRONT_AND_BACK, mode[1] == GL_FILL ? GL_LINE : GL_FILL);
	} else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose (windowPtr, true); // Closes the application if the escape key is pressed
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	// if mouse focus is on ImGui window, don't process mouse callback
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		cameraPtr->setRotation (baseRot + dRot);
	}
	else if (isPanning) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (dx, dy, 0.0));
	} else if (isZooming) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods)
{
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	// if mouse focus is on ImGui window, don't process mouse callback
	if (ImGui::GetIO().WantCaptureMouse)
		return;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	if (!isRotating) {
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = cameraPtr->getRotation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    	isRotating = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    	isPanning = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    	isZooming = false;
    }
}

void scrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void charCallback(GLFWwindow * window, unsigned int c)
{
	ImGui_ImplGlfw_CharCallback(window, c);
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	windowPtr = glfwCreateWindow (1024, 768, "Demo - MatCap Decomposition", nullptr, nullptr);
	if (!windowPtr) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	/// Connect the callbacks for interactive control
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCharCallback(windowPtr, charCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
	glfwSetScrollCallback(windowPtr, scrollCallback);
}

void initImGui()
{
	// Setup ImGui binding
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	ImGui_ImplGlfw_InitForOpenGL(windowPtr, /*install callbacks*/ false);
	const char* glsl_version = "#version 150";
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void exitOnCriticalError (const std::string & message) {
	std::cerr << "> [Critical error]" << message << std::endl;
	std::cerr << "> [Clearing resources]" << std::endl;
	clear ();
	std::cerr << "> [Exit]" << std::endl;
	std::exit (EXIT_FAILURE);
}


void initOpenGL () {
	// Load extensions for modern OpenGL
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress))
		exitOnCriticalError ("[Failed to initialize OpenGL context]");

	glEnable (GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
	glDebugMessageCallback (debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	 // disable debug notifications (only critical errors will be shown)
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);


	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f); // specify the background color, used any time the framebuffer is cleared
	// Loads and compile the programmable shader pipeline
	try {
		shaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "VertexShader.glsl", SHADER_PATH + "FragmentShader.glsl");
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}

	try {
		MatCapShaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "MatCapVertexShader.glsl", SHADER_PATH + "MatCapRender.glsl");
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}

}


int createFrameBufferTexture() {
	//création du frame buffer
	glGenFramebuffers(1,&fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_FLOAT, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D,0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	//glTexSubImage2D(texture, 0, 0, GL_RGB, 800, 600, GL_RGB, GL_FLOAT, (meshPtr->m_vertexNormals).data());

	//CHECK
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "ERROR: Failed to create Framebuffer" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	return texture;
//possibiliter d'utiliser un quad pour afficher la texture, pour voir
//si la texture s'affiche bien/si elle est cohérente
//(dans le main : glDrawArrays(GL_TRIANGLE_STRIP))
//ou par ex : glGetTextureIMage pour enregistrer une texture en image
//pour vérifier au moins si la texture s'affiche

}

void setMatCapComponents(int texture, Material material) {
	//initalisation
	MatCapDataLoader MatCapData = MatCapDataLoader::MatCapDataLoader(2, MATCAP_PATH);
	
	/*size_t vertexTextSize = sizeof(glm::vec3) * ((meshPtr->m_vertexNormals).size());
	glGenTextures(1, &test);
	glBindTexture(GL_TEXTURE_2D, test);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, vertexTextSize, 0, GL_RGB, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexSubImage2D(test, 0, 0, GL_RGB, 1, vertexTextSize, GL_RGB, GL_FLOAT, (meshPtr->m_vertexNormals).data());*/
	MatCapShaderProgramPtr->set("inNormals",0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, test);
	
	
	MatCapShaderProgramPtr->set("inLowFreqFront", 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, MatCapData.LowFront);

	MatCapShaderProgramPtr->set("inLowFreqBack", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, MatCapData.LowBack);

	
	glm::vec2 abs = glm::vec2(0.5, 0.5);
	glm::vec3 thethaphi = FilterMat.dual_parabloid_to_cartesian(abs);
	std::vector<float> resul = FilterMat.GetVarianceForPoint(0, MATCAP_PATH + "MatCap01_TEST_LOW.jpg",30,10,10);

	std::pair<std::pair<bool, int>, std::pair<bool, int>> result = FilterMat.findInflexions(resul, 20);

	

	MatCapShaderProgramPtr->set("inHighFreqFront", 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, MatCapData.HighFront);


	MatCapShaderProgramPtr->set("inHighFreqBack", 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, MatCapData.HighBack);
	
	MatCapShaderProgramPtr->set("LowFreqVariance", MatCapData.LowFreqVariance);
	MatCapShaderProgramPtr->set("LowFreqBaseColor",MatCapData.LowFreqBaseColor);
	MatCapShaderProgramPtr->set("HighFreqVariance", MatCapData.HighFreqVariance);
	MatCapShaderProgramPtr->set("HighFreqBaseColor", MatCapData.HighFreqBaseColor);
	MatCapShaderProgramPtr->set("LowFreqTheta0", MatCapData.LowFreqTheta0);
	MatCapShaderProgramPtr->set("LowFreqM1", MatCapData.LowFreqM1);
	MatCapShaderProgramPtr->set("LowFreqAlpha1", MatCapData.LowFreqAlpha1);
	MatCapShaderProgramPtr->set("HighFreqTheta0", MatCapData.HighFreqTheta0);
	MatCapShaderProgramPtr->set("HighFreqM1", MatCapData.HighFreqM1);
	MatCapShaderProgramPtr->set("HighFreqAlpha1", MatCapData.HighFreqAlpha1);
	MatCapShaderProgramPtr->set("rotLightTheta", MatCapData.rotLightTheta);
	MatCapShaderProgramPtr->set("rotLightPhi", MatCapData.rotLightPhi);


}

void initScene (const std::string & meshFilename) {
	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));

	// Mesh
	meshPtr = std::make_shared<Mesh> ();
	try {
		MeshLoader::loadOFF (meshFilename, meshPtr);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading mesh]") + e.what ());
	}
	meshPtr->init ();


	// Lighting
	glm::vec3 lightSourcePosition (0.0, 10.0, 10.0);
	glm::vec3 lightSourceColor (1.0, 1.0, 1.0);
	float lightSourceIntensity = 1.f;
	glm::vec4 vec4LightSource1 = cameraPtr->computeViewMatrix() * glm::vec4(lightSourcePosition, 1);

	shaderProgramPtr->set ("lightSource.position", glm::vec3(vec4LightSource1)/vec4LightSource1[3]);
	shaderProgramPtr->set("lightSource.position", lightSourcePosition);

	shaderProgramPtr->set ("lightSource.color", lightSourceColor);
	shaderProgramPtr->set ("lightSource.intensity", lightSourceIntensity);

	// Material
	Material material = Material();
	//glm::vec3 materialAlbedo (0.4, 0.6, 0.2);
	//shaderProgramPtr->set ("material.albedo", materialAlbedo);
	shaderProgramPtr->set("material.albedo",material.getAlbedo());
	shaderProgramPtr->set ("material.kd", material.getKd());

	// Adjust the camera to the actual mesh
	glm::vec3 center;
	meshPtr->computeBoundingSphere (center, meshScale);
	cameraPtr->setTranslation (center + glm::vec3 (0.0, 0.0, 3.0 * meshScale));
	cameraPtr->setNear (meshScale / 100.f);
	cameraPtr->setFar (6.f * meshScale);

	texture=createFrameBufferTexture();
	setMatCapComponents(texture,material);

	
}



void init (const std::string & meshFilename) {
	initGLFW (); // Windowing system
	initOpenGL (); // OpenGL Context and shader pipeline
	initImGui(); // User Interface
	initScene (meshFilename); // Actual scene to render
}

void clear () {
	cameraPtr.reset ();
	meshPtr.reset ();
	shaderProgramPtr.reset ();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}

// The main rendering call
void render () {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers
	shaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
	shaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
	glm::mat4 modelMatrix = meshPtr->computeTransformMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
	shaderProgramPtr->set ("modelViewMat", modelViewMatrix);
	shaderProgramPtr->set ("normalMat", normalMatrix);
	meshPtr->render();

	//NE PAS OUBLIER D'UNBIND LE FRAMEBUFFER
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	shaderProgramPtr->stop ();

	MatCapShaderProgramPtr->use();
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

	MatCapShaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
	MatCapShaderProgramPtr->set ("modelViewMat", modelViewMatrix);
	MatCapShaderProgramPtr->set ("normalMat", normalMatrix);

	meshPtr->render();
	MatCapShaderProgramPtr->stop();
}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->

/*
	glm::vec3 lightSourcePosition (0.0, 100.0, 200.0);

	glm::vec4 vec4LightSource1 = cameraPtr->computeViewMatrix() * glm::vec4(lightSourcePosition, 1);


	shaderProgramPtr->use();
	shaderProgramPtr->set("lightSource.position", glm::vec3(vec4LightSource1)/vec4LightSource1[3]);
	shaderProgramPtr->stop();
*/

}

void showUI()
{
	/* see documentation and demo at https://github.com/ocornut/imgui	*/

	// imgui pushes
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f/*0.6f*/); // alpha applied to widgets
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);

	// set current UI font
	ImGui::GetFont()->FontSize = 16;

	// create flags for UI windows
	static const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_AlwaysAutoResize
		| ImGuiWindowFlags_NoCollapse
		//| ImGuiWindowFlags_AlwaysHorizontalScrollbar
		//| ImGuiWindowFlags_NoScrollbar
		//| ImGuiWindowFlags_MenuBar
		//| ImGuiWindowFlags_NoTitleBar
		//| ImGuiWindowFlags_NoResize
		//| ImGuiWindowFlags_NoMove
		;

	// force next window position or size
	//ImVec2 anchor(0, 0);
	//ImGui::SetNextWindowPos(anchor);
	//ImVec2 size(500, 100);
	//ImGui::SetNextWindowSize(size);

	ImGui::Begin("Movable window", 0, window_flags);
	if (ImGui::Button("A button saying Hello!"))
	{
		std::cout << "Hello!" << std::endl;
	}
	static float val = 0, min_val = -10, max_val = 5;
	if (ImGui::SliderFloat("A slider changing a float percentage", &val, min_val, max_val, "%.2f %%"))
	{
		std::cout << "New percentage: " << val << std::endl;
	}
	ImGui::End();

	// imgui pops
	ImGui::PopStyleVar(3);
}

void usage (const char * command) {
	std::cerr << "Usage : " << command << " [<file.off>]" << std::endl;
	std::exit (EXIT_FAILURE);
}

int main (int argc, char ** argv) {

	if (argc > 2)
		usage (argv[0]);

	// Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
	init (argc == 1 ? DEFAULT_MESH_FILENAME : argv[1]);

	while (!glfwWindowShouldClose (windowPtr))
	{
		// begin frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// your computations and rendering stuff
		update (static_cast<float> (glfwGetTime ()));
		render ();
		// your GUI stuff
		showUI();

		// show gui on top of render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// end frame
		glfwSwapBuffers (windowPtr);
		glfwPollEvents();
	}

	clear ();

	std::cout << " > Quit" << std::endl;
	return EXIT_SUCCESS;

}
