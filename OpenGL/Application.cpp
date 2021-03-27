#define STB_IMAGE_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "FreeCamera.h"
#include <iostream>
#include <vector>
#include <iterator>
#include <numeric>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "Model.h"
#include "Camera.h"

//DATA INITIALIZATION
#include "data.h"


// camera
FreeCamera camera(glm::vec3(0.0f, 5.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

Camera cam_track = Camera(carInitialPosition + glm::vec3(0.0f, 4.0f, 0.0f), carInitialPosition);
Camera cam_observator = Camera(carInitialPosition + glm::vec3(0.0f, 9.0f, 0.0f), carInitialPosition);
Camera cam_const = Camera(carInitialPosition + glm::vec3(-36.0f, 4.0f, 36.0f), carInitialPosition);
Camera cam_const2 = Camera(carInitialPosition + glm::vec3(-16.0f, 4.0f, 16.0f), carInitialPosition + glm::vec3(16.0f, 0.0f, 16.0f));
Camera cam_behind_car = Camera(carInitialPosition + glm::vec3(0.0, 4.0, 0.0) - glm::vec3(5.0) * carDirVersor, carInitialPosition);

AbstractCamera* currCam = &cam_const2;

// skybox texture
unsigned int cubemapTexture;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
bool isDirectional = false;
float directional = 0.0f;
float pointLight = 1.0f;
vector<PointLight> pointLights = vector<PointLight>();
vector<DirectionalLight> dirLights = vector<DirectionalLight>();
SpotLight spot1;
SpotLight spot2;
float spotHeight = 0;
bool isFog = false;
bool isGourand = false;
bool lighter = false;
bool isDeffered = false;
bool pointLightsOn = true;

// car circle
float radius = 13;
float angle = 0;
float xCir = carInitialPosition.x;
float zCir = carInitialPosition.z;

// vao, vbo, buffers
unsigned int skyboxVAO, skyboxVBO;
unsigned int VBO, cubeVAO;
unsigned int gBuffer;
unsigned int gPosition, gNormal, gAlbedoSpec;
unsigned int lightCubeVAO;

// current lighting shader pointer
Shader* currModelShader;

void renderQuad();
std::pair<glm::vec3, glm::vec3> defferedShading(Shader& defferedLightingShader, Shader& defferedShader, Model* models);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
std::pair<glm::vec3, glm::vec3> forwardShading(Shader& gourandShader, Shader& phongShader, Model* models);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(vector<std::string> faces);
void renderScene(Shader* currModelShader, Model& scene);
glm::vec3 renderCar(Shader* currModelShader, Model& car);
void updateCameras(glm::vec3 carCurrentPosition);
void turnLightsOn(Light lights[]);
void turnLightsOff(Light lights[]);
glm::vec3 renderModels(Shader& shader, Model* models);
void simulateDayNightCycle();
void renderSkybox(Shader& skyboxShader, glm::vec3 lightVal);
glm::vec3 updateLightColors(Shader& shader);
void copyDepthBufferToFrameBuffer(int buffer);
glm::vec3 lightingPass(Shader& defferedLightingShader);
glm::vec3 geometryPass(Shader& defferedShader, Shader& defferedLightingShader, Model* models);
void updateAngle();
void updateCarReflector(SpotLight* spot, glm::vec3 carCurrentPosition, glm::vec3 spotLightPosition, float diff);
glm::vec3 updateCarReflectors();
void setUpLights();
void setUpGBuffer();
void setLightsToShaders(Shader& shader);
void setUpVBOVAO();


int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Project", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    unsigned int loadTexture(const char* path);

    // keyborad input:
    glfwSetKeyCallback(window, keyCallback);


    /* Initialize GLEW library */
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Initializing GLEW libraray fail!\n";
    }

    // stbi_set_flip_vertically_on_load(true);

     // configure global opengl state
     // -----------------------------
     glEnable(GL_DEPTH_TEST);

    // configure g-buffer framebuffer
    // ------------------------------
     setUpGBuffer();
    

     // light creation
     setUpLights();
  
     // build and compile our shader zprogram
   // ------------------------------------
     Shader lightingShader("basic_lighting.vs", "basic_lighting.fs");
     Shader lightCubeShader("light_cube.vs", "light_cube.fs");
     // build and compile shaders
     // -------------------------
     Shader ourShader("model_loading.vs", "model_loading.fs");
     Shader gourandShader("model_loading_gourand.vs", "model_loading_gourand.fs");
     Shader skyboxShader("skybox.vs", "skybox.fs");
     Shader defferedShader("deffered_shading.vs", "deffered_shading.fs");
     Shader defferedLightingShader("deffered_lighting.vs", "deffered_lighting.fs");

     // shader configuration
// --------------------
     lightingShader.use();
     lightingShader.setInt("material.diffuse", 0);
     lightingShader.setInt("material.specular", 1);

     skyboxShader.use();
     skyboxShader.setInt("skybox", 0);

     //deffered shader configuration
    // --------------------
     defferedLightingShader.use();
     defferedLightingShader.setInt("gPosition", 0);
     defferedLightingShader.setInt("gNormal", 1);
     defferedLightingShader.setInt("gAlbedoSpec", 2);

     setLightsToShaders(ourShader);
     setLightsToShaders(gourandShader);
     setLightsToShaders(defferedLightingShader);


     // load models
// -----------
     Model scene("resources/objects/odniesienie/scene.obj");
     Model car("resources/objects/auto1/auto1.obj");
     Model models[2] = { scene, car };

    setUpVBOVAO();

    // loading skybox textures:
    cubemapTexture = loadCubemap(faces);

   
    // render loop
 // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        updateCameras(glm::vec3(0.0));
        //do deffered shading if enabled
        std::pair<glm::vec3, glm::vec3> res;
        if (isDeffered)
            res = defferedShading(defferedLightingShader, defferedShader, models);
        else 
            res = forwardShading(gourandShader, ourShader, models);
       
        glm::vec3 carCurrentPosition = res.first;
        glm::vec3 lightVal = res.second;

        updateAngle();

        renderSkybox(skyboxShader, lightVal);
              
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);
    //czy trzeba usun¹æ inne buffery?

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_F1))
        spotHeight += deltaTime;

    if (glfwGetKey(window, GLFW_KEY_F2))
        spotHeight -= deltaTime;

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        currCam = &camera;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        currCam = &cam_track;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        currCam = &cam_observator;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        currCam = &cam_const2;

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        currCam = &cam_const; 

}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_E:
        if (action == GLFW_PRESS)
        {
            isFog = !isFog;
        }
        break;
    case GLFW_KEY_Z:
        if (action == GLFW_PRESS && !isDeffered)
        {
            isGourand = !isGourand;
        }
        break;
    case GLFW_KEY_X:
        if (action == GLFW_PRESS && !isGourand)
        {
            isDeffered = !isDeffered;
        }
        break;
    default:
        break;
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        currCam = &camera;

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        currCam = &cam_track;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        currCam = &cam_observator;

    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        currCam = &cam_const2;

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        currCam = &cam_const;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        currCam = &cam_behind_car;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void turnLightsOn(Light lights[])
{
    for (int i = 0; i < 6; i++)
    {
        lights[i].TurnOn();
    }
    pointLightsOn = true;
}

void turnLightsOff(Light lights[])
{
    for (int i = 0; i < 6; i++)
    {
        lights[i].TurnOff();
    }
    pointLightsOn = false;
}

void renderScene(Shader* currModelShader, Model& scene)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 5.0f)); // translate it down so it's at the center of the scene
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
    currModelShader->setMat4("model", model);
    currModelShader->setMat4("normal", transpose(inverse(model)));
    scene.Draw(*currModelShader);
}

glm::vec3 renderCar(Shader* currModelShader, Model& car)
{
    glm::mat4 model2 = glm::mat4(1.0f);
    glm::vec3 carCurrentPosition = glm::vec3(carInitialPosition.x + -sin(angle) * radius, carInitialPosition.y + 0.0f, carInitialPosition.z - radius + (1 - cos(angle)) * radius);
    glm::mat4 model3 = glm::translate(model2, carCurrentPosition);
    model3 = glm::rotate(model3, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    currModelShader->setMat4("model", model3);
    currModelShader->setMat4("normal", transpose(inverse(model3)));
    car.Draw(*currModelShader);
    return carCurrentPosition;
}

void simulateDayNightCycle()
{
    float speed = 0.0094f;
    if (lighter)
    {
        if (dirLights[0].diffuse.x < 0.6)
           dirLights[0].diffuse += glm::vec3(speed) * deltaTime;
        else
            lighter = false;
    }
    else
    {
        if (dirLights[0].diffuse.x > 0.0)
           dirLights[0].diffuse -= glm::vec3(speed) * deltaTime;
        else
            lighter = true;
    }
}

void updateCameras(glm::vec3 carCurrentPosition)
{
    glm::vec3 pos = glm::vec3(carInitialPosition.x + -sin(angle) * radius, 0.0f, carInitialPosition.z - radius  + (1 - cos(angle)) * radius);
    cam_track.updateTarget(pos);
    cam_track.updatePosition(glm::vec3(-5.0 + pos.x, 40.0f, 5.0 + pos.z));
    
    cam_observator.updateTarget(pos);

    glm::mat4 mV = glm::mat4(1.0f);
    mV = glm::rotate(mV, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 move = mV * glm::vec4(carDirVersor, 1.0);
    glm::vec3 carDirVersor = glm::vec3(move.x / move.a, move.y / move.a, move.z / move.a);

    cam_behind_car.updatePosition(pos + glm::vec3(0.0, 4.0, 0.0) - 15.0f * carDirVersor);
    cam_behind_car.updateTarget(pos);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void renderSkybox(Shader& skyboxShader, glm::vec3 lightVal)
{ // draw skybox as last
    glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
    skyboxShader.use();
    skyboxShader.setVec3("skyColor", lightVal);
    glm::mat4 view = glm::mat4(glm::mat3((*currCam).GetViewMatrix())); // remove translation from the view matrix
    skyboxShader.setMat4("view", view);
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    skyboxShader.setMat4("projection", projection);
    // skybox cube
    glBindVertexArray(skyboxVAO);
    // glActiveTexture(GL_TEXTURE0);
     //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); // set depth function back to default

}

glm::vec3 renderModels(Shader& shader, Model* models)
{
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = currCam->GetViewMatrix();
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    renderScene(&shader, models[0]);
    return renderCar(&shader, models[1]);
}

glm::vec3 updateLightColors(Shader& shader)
{
    shader.use();
    shader.setBool("isFog", isFog);

    //day and night:
    simulateDayNightCycle(); //changes lights color

    glm::vec3 lightVal = std::accumulate(dirLights.begin(), dirLights.end(), glm::vec3(0.0f), [](glm::vec3 sum, DirectionalLight light) { return sum + light.diffuse;});
    shader.setVec3("fogColor", lightVal);
    shader.setLightsUniforms();
    return lightVal;
}

void updateAngle()
{
    angle += 1 * deltaTime;
    if (angle > 360)
        angle = 0;
}

std::pair<glm::vec3, glm::vec3> defferedShading(Shader& defferedLightingShader, Shader& defferedShader, Model* models)
{
    // 1. geometry pass: render all geometric/color data to g-buffer 
    glm::vec3 carCurrentPosition = geometryPass(defferedShader, defferedLightingShader, models);
    
    // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
    // -----------------------------------------------------------------------------------------------------------------------
    glm::vec3 lightVal = lightingPass(defferedLightingShader);

    // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer to enable futher forward rendering
    // ----------------------------------------------------------------------------------
    copyDepthBufferToFrameBuffer(gBuffer);
   
    return std::make_pair(carCurrentPosition, lightVal);
}

std::pair<glm::vec3, glm::vec3> forwardShading(Shader& gourandShader, Shader& phongShader, Model* models)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currModelShader = &phongShader;

    if (isGourand)
        currModelShader = &gourandShader;


    glm::vec3 carCurrentPosition = updateCarReflectors();
    glm::vec3 lightVal = updateLightColors(*currModelShader);


    currModelShader->use();
    currModelShader->setVec3("viewPos", currCam->Position);
    renderModels(*currModelShader, models);

    return std::make_pair(carCurrentPosition, lightVal);
}



glm::vec3 geometryPass(Shader& defferedShader, Shader& defferedLightingShader, Model* models)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    defferedLightingShader.use();
    defferedLightingShader.setVec3("viewPos", currCam->Position);

    glm::vec3 carCurrentPosition = renderModels(defferedShader, models);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return carCurrentPosition;
}

glm::vec3 lightingPass(Shader& defferedLightingShader)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    defferedLightingShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    updateCarReflectors();
    glm::vec3 lightVal = updateLightColors(defferedLightingShader);

    // finally render quad - two triangles accordingly to g-buffor data
    renderQuad();
    return lightVal;
}

void copyDepthBufferToFrameBuffer(int buffer)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
    // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
    // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
    glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 updateCarReflectors()
{
    glm::vec3 carCurrentPosition = glm::vec3(carInitialPosition.x -sin(angle) * radius, 0.0f, carInitialPosition.z - radius + (1 - cos(angle)) * radius);
   
    updateCarReflector(&spot1, carCurrentPosition, spotLightPositions[0], 0.1);
    updateCarReflector(&spot2, carCurrentPosition, spotLightPositions[1], -0.1);

    return carCurrentPosition;
}

void updateCarReflector(SpotLight* spot, glm::vec3 carCurrentPosition, glm::vec3 spotLightPosition, float diff)
{
    glm::mat4 mV = glm::mat4(1.0f);
    mV = glm::rotate(mV, angle + diff, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 move = mV * glm::vec4(carDirVersor, 1.0);
    glm::vec3 carDirVersor = glm::vec3(move.x / move.a, move.y / move.a, move.z / move.a);

    spot->direction = carDirVersor + glm::vec3(0.0, spotHeight, 0.0);

    glm::mat4 rM = glm::mat4(1.0f);
    rM = glm::rotate(rM, angle, glm::vec3(0.0, 1.0, 0.0));
    glm::vec4 move2 = rM * glm::vec4(spotLightPosition, 1.0);

    spot->position =  glm::vec3(move2) + carCurrentPosition;
    
}

void copyDepthBufferToFrameBuffer(int buffer);
glm::vec3 lightingPass(Shader& defferedLightingShader);
glm::vec3 geometryPass(Shader& defferedShader, Shader& defferedLightingShader, Model* models);

void setUpLights()
{
    //Light Creation:
    glm::vec3 dirA = glm::vec3(0.05f, 0.05f, 0.05f);
    glm::vec3 dirD = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 dirS = glm::vec3(0.02f, 0.04f, 0.05f);
    dirLights.push_back(DirectionalLight(dirA, dirD, dirS, glm::vec3(0.3f, -0.9f, 0.3f)));
    pointLights.push_back(PointLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), pointLightPositions[0], 1.0, 0.14, 0.07));
    pointLights.push_back(PointLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), pointLightPositions[1], 1.0, 0.14, 0.07));
    pointLights.push_back(PointLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), pointLightPositions[2], 1.0, 0.14, 0.07));
    pointLights.push_back(PointLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), pointLightPositions[3], 1.0, 0.14, 0.07));
    pointLights.push_back(PointLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), pointLightPositions[4], 1.0, 0.14, 0.07));
    pointLights.push_back(PointLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), pointLightPositions[5], 1.0, 0.14, 0.07));
    spot1 = SpotLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), carInitialPosition, 1.0f, 0.014, 0.0007, carDirVersor, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));
    spot2 = SpotLight(glm::vec3(0.05f, 0.05f, 0.05f), glm::vec3(1, 0.8f, 0), glm::vec3(1, 0.8f, 0), carInitialPosition, 1.0f, 0.014, 0.0007, carDirVersor, glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f)));

}

void setLightsToShaders(Shader& shader)
{
    for (auto it = dirLights.begin(); it != dirLights.end(); it++)
    {
        shader.AddDirectionalLight(&(*it));
    }


    for (auto it = pointLights.begin(); it != pointLights.end(); it++)
    {
        shader.AddPointLight(&(*it));
    }

    shader.AddSpotLight(&spot1);
    shader.AddSpotLight(&spot2);
}

void setUpGBuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void setUpVBOVAO()
{
    //configure the cube's VAO (and VBO)

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    // skybox VAO

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // lighting map
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}