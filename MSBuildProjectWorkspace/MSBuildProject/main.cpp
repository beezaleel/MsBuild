#include "GlobalOpenGL.h"
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream> 
#include "GlobalObjects.h"
#include "VAOManager.h"
#include "ShaderManager.h"
#include "LightHelper.h"

const unsigned int NUMBER_OF_BUNNIES = 10;
static bool seeded = false;

//glm::vec3 gCameraEye = glm::vec3(-47.0, 42.0, -93.7f);
glm::vec3 gCameraEye = glm::vec3(-28.16, 479.0, -1452.7f);
//glm::vec3 gCameraEye = glm::vec3(441.16, 78, -183.7f);
glm::vec3 gCameraTarget = glm::vec3(0.0f, 10.0f, 0.0f);

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

float RandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;

    return a + r;
}


bool LoadThePLYFile(std::string filename, ModelDrawInfo& modelDrawInfo)
{
    struct sVertex_XYZ_N_RGBA_UV
    {
        float x, y, z;
        float nx, ny, nz;
        float red, green, blue, alpha;
        float texture_u, texture_v;
    };

    struct sTrianglePLY
    {
        unsigned int vertexIndices[3];
    };

    sVertex_XYZ_N_RGBA_UV* pTheModelArray = NULL;   // NULL, 0, nullptr
    sTrianglePLY* pTheModelTriangleArray = NULL;

    std::ifstream theFile(filename);
    if (!theFile.is_open())
    {
        std::cout << "Unable to open file!" << std::endl;
        return false;
    }

    // Read an entire line
    char buffer[10000];
    theFile.getline(buffer, 10000);

    std::string theNextToken;

    // Scan for the word "vertex"
    while (theFile >> theNextToken)
    {
        if (theNextToken == "vertex")
        {
            break;
        }
    }
    // 
    theFile >> modelDrawInfo.numberOfVertices;

    // Scan for the word "face"
    while (theFile >> theNextToken)
    {
        if (theNextToken == "face")
        {
            break;
        }
    }
    // 
    theFile >> modelDrawInfo.numberOfTriangles;

    // Scan for the word "end_header"
    while (theFile >> theNextToken)
    {
        if (theNextToken == "end_header")
        {
            break;
        }
    }

    pTheModelArray = new sVertex_XYZ_N_RGBA_UV[modelDrawInfo.numberOfVertices];

    std::cout << "Loading";
    for (unsigned int count = 0; count != modelDrawInfo.numberOfVertices; count++)
    {
        theFile >> pTheModelArray[count].x;
        theFile >> pTheModelArray[count].y;
        theFile >> pTheModelArray[count].z;

        theFile >> pTheModelArray[count].nx;
        theFile >> pTheModelArray[count].ny;
        theFile >> pTheModelArray[count].nz;

        theFile >> pTheModelArray[count].red;
        theFile >> pTheModelArray[count].green;
        theFile >> pTheModelArray[count].blue;
        theFile >> pTheModelArray[count].alpha;

        theFile >> pTheModelArray[count].texture_u;
        theFile >> pTheModelArray[count].texture_v;

        if (count % 10000 == 0)
        {
            std::cout << ".";
        }
    }
    std::cout << "done" << std::endl;


    // HACK: "Transform" the model here
    for (unsigned int count = 0; count != modelDrawInfo.numberOfVertices; count++)
    {
        //        pTheModelArray[count].x *= 0.1f;
        //        pTheModelArray[count].y *= 0.1f;
        //        pTheModelArray[count].z *= 0.1f;

        //        pTheModelArray[count].x += 20.0f;
    }


    // Load the faces (or triangles)
    pTheModelTriangleArray = new sTrianglePLY[modelDrawInfo.numberOfTriangles];

    for (unsigned int count = 0; count != modelDrawInfo.numberOfTriangles; count++)
    {
        // 3 15393 15394 15395 
        unsigned int discard = 0;
        theFile >> discard;

        theFile >> pTheModelTriangleArray[count].vertexIndices[0];
        theFile >> pTheModelTriangleArray[count].vertexIndices[1];
        theFile >> pTheModelTriangleArray[count].vertexIndices[2];
    }

    theFile.close();

    modelDrawInfo.pVertices = new Vertex[modelDrawInfo.numberOfVertices];

    // Now copy the information from the PLY infomation to the model draw info structure
    for (unsigned int index = 0; index != modelDrawInfo.numberOfVertices; index++)
    {
        // To The Shader                        From the file
        modelDrawInfo.pVertices[index].x = pTheModelArray[index].x;
        modelDrawInfo.pVertices[index].y = pTheModelArray[index].y;
        modelDrawInfo.pVertices[index].z = pTheModelArray[index].z;

        modelDrawInfo.pVertices[index].r = pTheModelArray[index].red;
        modelDrawInfo.pVertices[index].g = pTheModelArray[index].green;
        modelDrawInfo.pVertices[index].b = pTheModelArray[index].blue;

        // Copy the normal information we loaded, too! :)
        modelDrawInfo.pVertices[index].nx = pTheModelArray[index].nx;
        modelDrawInfo.pVertices[index].ny = pTheModelArray[index].ny;
        modelDrawInfo.pVertices[index].nz = pTheModelArray[index].nz;

    }

    modelDrawInfo.numberOfIndices = modelDrawInfo.numberOfTriangles * 3;

    // This is the "index" (or element) buffer
    modelDrawInfo.pIndices = new unsigned int[modelDrawInfo.numberOfIndices];

    unsigned int vertex_element_index_index = 0;

    for (unsigned int triangleIndex = 0; triangleIndex != modelDrawInfo.numberOfTriangles; triangleIndex++)
    {
        modelDrawInfo.pIndices[vertex_element_index_index + 0] = pTheModelTriangleArray[triangleIndex].vertexIndices[0];
        modelDrawInfo.pIndices[vertex_element_index_index + 1] = pTheModelTriangleArray[triangleIndex].vertexIndices[1];
        modelDrawInfo.pIndices[vertex_element_index_index + 2] = pTheModelTriangleArray[triangleIndex].vertexIndices[2];

        // Each +1 of the triangle index moves the "vertex element index" by 3
        // (3 vertices per triangle)
        vertex_element_index_index += 3;
    }

    delete[] pTheModelArray;
    delete[] pTheModelTriangleArray;


    return true;
}


bool LoadModelTypesIntoVAO(std::string fileTypesToLoadName,
    VAOManager* pVAOManager,
    GLuint shaderID)
{
    std::ifstream modelTypeFile(fileTypesToLoadName.c_str());
    if (!modelTypeFile.is_open())
    {
        // Can't find that file
        return false;
    }

    std::string PLYFileNameToLoad;
    std::string friendlyName;

    bool bKeepReadingFile = true;

    const unsigned int BUFFER_SIZE = 1000;
    char textBuffer[BUFFER_SIZE];
    // Clear that array to all zeros
    memset(textBuffer, 0, BUFFER_SIZE);

    while (bKeepReadingFile)
    {
        modelTypeFile.getline(textBuffer, BUFFER_SIZE);

        PLYFileNameToLoad.clear(); 
        PLYFileNameToLoad.append(textBuffer);

        if (PLYFileNameToLoad == "EOF")
        {
            // All done
            bKeepReadingFile = false;
            // Skip to the end of the while loop
            continue;
        }

        memset(textBuffer, 0, BUFFER_SIZE);
        modelTypeFile.getline(textBuffer, BUFFER_SIZE);
        friendlyName.clear();
        friendlyName.append(textBuffer);

        ModelDrawInfo motoDrawInfo;

        if (LoadThePLYFile(PLYFileNameToLoad, motoDrawInfo))
        {
            std::cout << "Loaded the file OK" << std::endl;
        }
        if (pVAOManager->LoadModelIntoVAO(friendlyName, motoDrawInfo, shaderID))
        {
            std::cout << "Loaded the MOTO model" << std::endl;
        }

    }

    return true;
}

int main(void)
{
    std::cout << "starting up..." << std::endl;

    GLFWwindow* window;
    GLuint vertex_buffer = 0;
    GLuint shaderID = 0;
    

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Basic3DScene", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    std::cout << "Window created." << std::endl;

    glfwSetKeyCallback(window, KeyCallback);

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    ShaderManager* shaderManager = new ShaderManager();

    Shader vertexShader01;
    Shader fragmentShader01;

    vertexShader01.fileName = "assets/shaders/vertexShader01.glsl";
    fragmentShader01.fileName = "assets/shaders/fragmentShader01.glsl";

    if (!shaderManager->CreateProgramFromFile("Shader_1", vertexShader01, fragmentShader01))
    {
        std::cout << "Didn't compile shaders" << std::endl;
        std::string theLastError = shaderManager->GetLastError();
        std::cout << "Because: " << theLastError << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Compiled shader OK." << std::endl;
    }

    shaderManager->UseShaderProgram("Shader_1");

    shaderID = shaderManager->GetIDFromFriendlyName("Shader_1");

    glUseProgram(shaderID);

    ::gLightManager = new LightManager();
    LightHelper* pLightHelper = new LightHelper();
    ::gLightManager->LoadLightUniformLocations(shaderID);
    

    //::gLightManager->vLights[0].position = glm::vec4(-28.3f, 954.9f, -24.4f, 1.0f);
    ::gLightManager->vLights[0].position = glm::vec4(500.0f, 370.0f, 1100.0f, 1.0f);
    ::gLightManager->vLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::gLightManager->vLights[0].atten = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::gLightManager->vLights[0].TurnOn();

    ::gLightManager->vLights[1].position = glm::vec4(-28.3f, 1200.9f, -24.4f, 1.0f);
    ::gLightManager->vLights[1].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    ::gLightManager->vLights[1].atten = glm::vec4(0.01f, 0.0001f, 0.000001f, 1.0f);
    ::gLightManager->vLights[1].TurnOn();


    // Load the models
    VAOManager* pVAOManager = new VAOManager();

    if (!LoadModelTypesIntoVAO("assets/PLYFilesToLoadIntoVAO.txt", pVAOManager, shaderID))
    {
        std::cout << "Error: Unable to load list of models to load into VAO file" << std::endl;
    }
    std::vector< MeshObject* > vec_pMeshObjects;

    MeshObject* pTerrain = new MeshObject();
    pTerrain->meshName = "WestTown";
    pTerrain->scale = 100.0f;
    vec_pMeshObjects.push_back(pTerrain);

    MeshObject* pTRex = new MeshObject();
    pTRex->meshName = "TRex";
    pTRex->position = glm::vec3(450.0f, 0.0f, -240.0f);
    pTRex->scale = 0.8f;
    vec_pMeshObjects.push_back(pTRex);

    MeshObject* pSoldier = new MeshObject();
    pSoldier->meshName = "Soldier";
    pSoldier->position = glm::vec3(300, 0, -110);
    pSoldier->bUseRGBAColour = true;
    pSoldier->RGBAColour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    pSoldier->rotation = glm::vec3(0.0f, 200.0f, 0.0f);
    pSoldier->scale = 3.0f;
    vec_pMeshObjects.push_back(pSoldier);

    MeshObject* pGiraffatitan = new MeshObject();
    pGiraffatitan->meshName = "Giraffatitan";
    pGiraffatitan->position = glm::vec3(-200, 0, -400);
    pGiraffatitan->scale = 10.8f;
    vec_pMeshObjects.push_back(pGiraffatitan);

    MeshObject* pSun = new MeshObject();
    pSun->meshName = "Sun";
    pSun->bUseRGBAColour = true;
    pSun->RGBAColour = glm::vec4(0.99f, 0.72f, 0.07f, 1.0f);
    pSun->position = glm::vec3(-500, 150, 1100);
    pSun->scale = 0.5f;
    vec_pMeshObjects.push_back(pSun);

    MeshObject* pTieFighter = new MeshObject();
    pTieFighter->meshName = "TieFighter";
    pTieFighter->position = glm::vec3(0.0f, 300.0f, -600.0f);
    pTieFighter->scale = 20.8f;
    vec_pMeshObjects.push_back(pTieFighter);

    MeshObject* pDeathStar = new MeshObject();
    pDeathStar->meshName = "DeathStar";
    //pDeathStar->bUseRGBAColour = true;
    //pDeathStar->RGBAColour = glm::vec4(0.99f, 0.72f, 0.07f, 1.0f);
    pDeathStar->position = glm::vec3(500, 180, 1100);
    pDeathStar->scale = 0.09f;
    vec_pMeshObjects.push_back(pDeathStar);


    MeshObject* pDebugSphere_1 = new MeshObject();
    pDebugSphere_1->meshName = "ISO_Sphere_1";
    pDebugSphere_1->friendlyName = "Debug Sphere 1";
    pDebugSphere_1->bUseRGBAColour = true;
    pDebugSphere_1->RGBAColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    pDebugSphere_1->isWireframe = true;
    pDebugSphere_1->scale = 1.0f;

    pDebugSphere_1->bDoNotLight = true;

    vec_pMeshObjects.push_back(pDebugSphere_1);

    MeshObject* pDebugSphere_2 = new MeshObject();
    pDebugSphere_2->meshName = "ISO_Sphere_1";
    pDebugSphere_2->friendlyName = "Debug Sphere 2";
    pDebugSphere_2->bUseRGBAColour = true;
    pDebugSphere_2->RGBAColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    pDebugSphere_2->isWireframe = true;
    pDebugSphere_2->scale = 1.0f;
    pDebugSphere_2->position = ::gLightManager->vLights[1].position;
    pDebugSphere_2->bDoNotLight = true;
    vec_pMeshObjects.push_back(pDebugSphere_2);

    GLint mvp_location = glGetUniformLocation(shaderID, "MVP");
    GLint mModel_location = glGetUniformLocation(shaderID, "mModel");
    GLint mView_location = glGetUniformLocation(shaderID, "mView");
    GLint mProjection_location = glGetUniformLocation(shaderID, "mProjection");
    // Need this for lighting
    GLint mModelInverseTransform_location = glGetUniformLocation(shaderID, "mModelInverseTranspose");

    while (!glfwWindowShouldClose(window))
    {
        pDebugSphere_1->position = glm::vec3(::gLightManager->vLights[0].position);

        float distance10percent = pLightHelper->CalcApproxDistFromAtten(
            0.1f,  // 10%
            0.001f,
            100000.0f,
            ::gLightManager->vLights[0].atten.x,
            ::gLightManager->vLights[0].atten.y,
            ::gLightManager->vLights[0].atten.z);

        ::gLightManager->CopyLightInformationToShader(shaderID);

        pTieFighter->position.z += 0.80f;
        //pDeathStar->rotation.y -= 0.009f;

        float ratio;
        int width, height;

        glm::mat4x4 matModel;
        glm::mat4x4 matProjection;
        glm::mat4x4 matView;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);

        // note the binary OR (not the usual boolean "or" comparison)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        matView = glm::lookAt(::gCameraEye,
            ::gCameraTarget,
            upVector);

        GLint eyeLocation_UniLoc = glGetUniformLocation(shaderID, "eyeLocation");

        glUniform4f(eyeLocation_UniLoc,
            ::gCameraEye.x, ::gCameraEye.y, ::gCameraEye.z, 1.0f);

        matProjection = glm::perspective(
            0.6f,
            ratio,
            0.1f,
            10000.0f);

        for (std::vector< MeshObject* >::iterator itCurrentMesh = vec_pMeshObjects.begin();
            itCurrentMesh != vec_pMeshObjects.end();
            itCurrentMesh++)
        {
            MeshObject* pCurrentMeshObject = *itCurrentMesh;

            glCullFace(GL_BACK);
            glEnable(GL_DEPTH_TEST);

            matModel = glm::mat4x4(1.0f);
            
            glm::mat4 matTranslation = glm::translate(glm::mat4(1.0f),
                pCurrentMeshObject->position);

            glm::mat4 matRoationZ = glm::rotate(glm::mat4(1.0f),
                pCurrentMeshObject->rotation.z,
                glm::vec3(0.0f, 0.0f, 1.0f));

            glm::mat4 matRoationY = glm::rotate(glm::mat4(1.0f),
                pCurrentMeshObject->rotation.y,
                glm::vec3(0.0f, 1.0f, 0.0f));

            glm::mat4 matRoationX = glm::rotate(glm::mat4(1.0f),
                pCurrentMeshObject->rotation.x,
                glm::vec3(1.0f, 0.0f, 0.0f));


            // Scale the object
            float uniformScale = pCurrentMeshObject->scale;
            glm::mat4 matScale = glm::scale(glm::mat4(1.0f),
                glm::vec3(uniformScale, uniformScale, uniformScale));

            matModel = matModel * matTranslation;

            matModel = matModel * matRoationX;
            matModel = matModel * matRoationY;
            matModel = matModel * matRoationZ;

            matModel = matModel * matScale;

            glUniformMatrix4fv(mModel_location, 1, GL_FALSE, glm::value_ptr(matModel));
            glUniformMatrix4fv(mView_location, 1, GL_FALSE, glm::value_ptr(matView));
            glUniformMatrix4fv(mProjection_location, 1, GL_FALSE, glm::value_ptr(matProjection));

            glm::mat4 mModelInverseTransform = glm::inverse(glm::transpose(matModel));
            glUniformMatrix4fv(mModelInverseTransform_location, 1, GL_FALSE, glm::value_ptr(mModelInverseTransform));

            if (pCurrentMeshObject->isWireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            GLint RGBA_Colour_ULocID = glGetUniformLocation(shaderID, "RGBA_Colour");

            glUniform4f(RGBA_Colour_ULocID,
                pCurrentMeshObject->RGBAColour.r,
                pCurrentMeshObject->RGBAColour.g,
                pCurrentMeshObject->RGBAColour.b,
                pCurrentMeshObject->RGBAColour.w);


            GLint bUseRGBA_Colour_ULocID = glGetUniformLocation(shaderID, "bUseRGBA_Colour");

            if (pCurrentMeshObject->bUseRGBAColour)
            {
                glUniform1f(bUseRGBA_Colour_ULocID, (GLfloat)GL_TRUE);
            }
            else
            {
                glUniform1f(bUseRGBA_Colour_ULocID, (GLfloat)GL_FALSE);
            }

            //uniform bool bDoNotLight;	
            GLint bDoNotLight_Colour_ULocID = glGetUniformLocation(shaderID, "bDoNotLight");

            if (pCurrentMeshObject->bDoNotLight)
            {
                glUniform1f(bDoNotLight_Colour_ULocID, (GLfloat)GL_TRUE);
            }
            else
            {
                glUniform1f(bDoNotLight_Colour_ULocID, (GLfloat)GL_FALSE);
            }


            // Choose the VAO that has the model we want to draw...
            ModelDrawInfo drawingInformation;
            if (pVAOManager->FindDrawInfoByModelName(pCurrentMeshObject->meshName, drawingInformation))
            {
                glBindVertexArray(drawingInformation.VAOId);

                glDrawElements(GL_TRIANGLES,
                    drawingInformation.numberOfIndices,
                    GL_UNSIGNED_INT,
                    (void*)0);

                glBindVertexArray(0);

            }
            else
            {
                std::cout << "Error: didn't find model to draw." << std::endl;

            }

        }

        glfwSwapBuffers(window);
        glfwPollEvents();;

        std::stringstream ssTitle;
        ssTitle << "Camera (x,y,z): "
            << ::gCameraEye.x << ", "
            << ::gCameraEye.y << ", "
            << ::gCameraEye.z
            << "  Light #0 (xyz): "
            << ::gLightManager->vLights[0].position.x << ", "
            << ::gLightManager->vLights[0].position.y << ", "
            << ::gLightManager->vLights[0].position.z
            << " linear: " << ::gLightManager->vLights[0].atten.y
            << " quad: " << ::gLightManager->vLights[0].atten.z;

        std::string theText = ssTitle.str();

        glfwSetWindowTitle(window, ssTitle.str().c_str());
    }

    // Get rid of stuff
    delete shaderManager;
    delete ::gLightManager;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}