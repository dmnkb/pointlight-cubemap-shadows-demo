#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#ifdef __APPLE__
#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// ---------------------------------------------
// Shader utils
// ---------------------------------------------
static GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;

    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, log.data());
        std::cerr << "Shader compile error:\n" << log << std::endl;
        std::exit(1);
    }
    return s;
}
static GLuint linkProgram(const char* vs, const char* fs)
{
    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(p, len, nullptr, log.data());
        std::cerr << "Program link error:\n" << log << std::endl;
        std::exit(1);
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

// ---------------------------------------------
// Geometry: cube helpers
// ---------------------------------------------
struct Mesh
{
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei count = 0;
};
static Mesh makeUnitCube(bool withNormals)
{
    struct VN
    {
        float x, y, z, nx, ny, nz;
    };
    struct V
    {
        float x, y, z;
    };
    static const unsigned idx[] = {0,  1,  2,  2,  3,  0,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
                                   12, 13, 14, 14, 15, 12, 16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20};
    const float p = 0.5f;
    Mesh m;
    glGenVertexArrays(1, &m.vao);
    glBindVertexArray(m.vao);
    glGenBuffers(1, &m.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glGenBuffers(1, &m.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
    if (withNormals)
    {
        const VN verts[] = {{-p, -p, p, 0, 0, 1},   {p, -p, p, 0, 0, 1},   {p, p, p, 0, 0, 1},   {-p, p, p, 0, 0, 1},
                            {-p, -p, -p, 0, 0, -1}, {-p, p, -p, 0, 0, -1}, {p, p, -p, 0, 0, -1}, {p, -p, -p, 0, 0, -1},
                            {p, -p, -p, 1, 0, 0},   {p, p, -p, 1, 0, 0},   {p, p, p, 1, 0, 0},   {p, -p, p, 1, 0, 0},
                            {-p, -p, -p, -1, 0, 0}, {-p, -p, p, -1, 0, 0}, {-p, p, p, -1, 0, 0}, {-p, p, -p, -1, 0, 0},
                            {-p, p, -p, 0, 1, 0},   {-p, p, p, 0, 1, 0},   {p, p, p, 0, 1, 0},   {p, p, -p, 0, 1, 0},
                            {-p, -p, -p, 0, -1, 0}, {p, -p, -p, 0, -1, 0}, {p, -p, p, 0, -1, 0}, {-p, -p, p, 0, -1, 0}};
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    else
    {
        const V verts[] = {{-p, -p, p},  {p, -p, p},  {p, p, p},    {-p, p, p},  {-p, -p, -p}, {-p, p, -p},
                           {p, p, -p},   {p, -p, -p}, {p, -p, -p},  {p, p, -p},  {p, p, p},    {p, -p, p},
                           {-p, -p, -p}, {-p, -p, p}, {-p, p, p},   {-p, p, -p}, {-p, p, -p},  {-p, p, p},
                           {p, p, p},    {p, p, -p},  {-p, -p, -p}, {p, -p, -p}, {p, -p, p},   {-p, -p, p}};
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }
    glBindVertexArray(0);
    m.count = (GLsizei)(sizeof(idx) / sizeof(idx[0]));
    return m;
}
static void drawMesh(const Mesh& m)
{
    glBindVertexArray(m.vao);
    glDrawElements(GL_TRIANGLES, m.count, GL_UNSIGNED_INT, 0);
}

// ---------------------------------------------
// Shaders
// ---------------------------------------------
static const char* depthVS = R"GLSL(
#version 410 core
layout(location=0) in vec3 aPos;
uniform mat4 uModel;
uniform mat4 uVP;
out vec3 WorldPos;
void main(){
    vec4 world = uModel * vec4(aPos,1.0);
    WorldPos = world.xyz;
    gl_Position = uVP * world;
}
)GLSL";

static const char* depthFS = R"GLSL(
#version 410 core
in vec3 WorldPos;
uniform vec3 uLightPos;
uniform float uFarPlane;
void main(){
    float dist = length(WorldPos - uLightPos);
    gl_FragDepth = dist / uFarPlane;
}
)GLSL";

static const char* litVS = R"GLSL(
#version 410 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
uniform mat4 uModel,uView,uProj;
out vec3 WorldPos;
out vec3 Normal;
void main(){
    vec4 world=uModel*vec4(aPos,1.0);
    WorldPos=world.xyz;
    Normal=mat3(transpose(inverse(uModel)))*aNormal;
    gl_Position=uProj*uView*world;
}
)GLSL";

static const char* litFS = R"GLSL(
#version 410 core
in vec3 WorldPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 uAlbedo;
uniform int uInvertNormals;
uniform int uNumLights;
uniform vec3 uLightPos[16];
uniform vec3 uLightColor[16];
uniform float uFarPlane[16];
uniform samplerCubeArray uDepthCubeArray;

float shadowFactor(vec3 worldPos, int li)
{
    vec3 L = worldPos - uLightPos[li];
    float current = length(L) / uFarPlane[li];
    float closest = texture(uDepthCubeArray, vec4(normalize(L), li)).r;
    float bias=0.003;
    return (current - bias > closest)?0.0:1.0;
}

void main(){
    vec3 N=normalize(Normal);
    if(uInvertNormals==1) N=-N;
    vec3 color=vec3(0.0);
    for(int i=0;i<uNumLights;i++){
        vec3 L=normalize(uLightPos[i]-WorldPos);
        float ndotl=max(dot(N,L),0.0);
        float s=shadowFactor(WorldPos,i);
        color+=uLightColor[i]*(0.06+ndotl*s);
    }
    FragColor=vec4(uAlbedo*color,1.0);
}
)GLSL";

// ---------------------------------------------
// Main
// ---------------------------------------------
int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* win = glfwCreateWindow(1280, 720, "Pointlight Cubemap Shadows Demo", nullptr, nullptr);
    if (!win)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
#if USE_GLAD2
    if (gladLoadGL() == 0)
    {
        std::cerr << "GLAD load fail\n";
        return 1;
    }
#else
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD v1 load fail\n";
        return 1;
    }
#endif
    std::cout << "GL: " << glGetString(GL_VERSION) << "\n";

    GLuint progDepth = linkProgram(depthVS, depthFS);
    GLuint progLit = linkProgram(litVS, litFS);
    Mesh cubePosOnly = makeUnitCube(false);
    Mesh cubeLit = makeUnitCube(true);

    glm::vec3 camPos = {2, 4.6, 5};
    const int NUM_LIGHTS = 3;
    glm::vec3 lightPos[NUM_LIGHTS] = {{0, 2.8, 0}, {2, 1, 2}, {-2, 1, -2}};
    glm::vec3 lightColor[NUM_LIGHTS] = {{.5, 0.97, 0.5}, {0.9, 0.6, 0.6}, {0.6, 0.8, 1.0}};
    float farPlane[NUM_LIGHTS] = {18.0f, 18.0f, 18.0f};

    const int SHADOW_RES = 512;
    const float nearPlane = 0.1f;

    GLuint depthFBO;
    glGenFramebuffers(1, &depthFBO);
    GLuint depthCubeArray;
    glGenTextures(1, &depthCubeArray);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubeArray);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT24, SHADOW_RES, SHADOW_RES, 6 * NUM_LIGHTS, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    auto cubeProj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, 18.0f);
    auto faceView = [&](glm::vec3 lp, int face)
    {
        switch (face)
        {
        case 0:
            return glm::lookAt(lp, lp + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
        case 1:
            return glm::lookAt(lp, lp + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
        case 2:
            return glm::lookAt(lp, lp + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
        case 3:
            return glm::lookAt(lp, lp + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
        case 4:
            return glm::lookAt(lp, lp + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
        default:
            return glm::lookAt(lp, lp + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
        }
    };
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1280.f / 720.f, 0.1f, 100.f);

    glEnable(GL_DEPTH_TEST);
    float t = 0.f;
    while (!glfwWindowShouldClose(win))
    {
        glfwPollEvents();
        t += 0.01f;
        // animate lights
        lightPos[0].x = std::sin(t) * 1.5f;
        lightPos[0].z = std::cos(t) * 1.5f;

        lightPos[1].x = std::cos(t * 0.7f) * 2.0f;
        lightPos[1].y = 1.5f + std::sin(t * 1.3f) * 0.5f;
        lightPos[1].z = std::sin(t * 0.7f) * 2.0f;

        lightPos[2].x = std::sin(t * 0.5f) * 2.5f;
        lightPos[2].y = 1.0f + std::cos(t * 0.9f) * 0.5f;
        lightPos[2].z = std::cos(t * 0.5f) * 2.5f;

        // 1) Depth cubemap array
        glViewport(0, 0, SHADOW_RES, SHADOW_RES);
        glUseProgram(progDepth);
        GLint locModel = glGetUniformLocation(progDepth, "uModel");
        GLint locVP = glGetUniformLocation(progDepth, "uVP");
        for (int li = 0; li < NUM_LIGHTS; li++)
        {
            glUniform3fv(glGetUniformLocation(progDepth, "uLightPos"), 1, glm::value_ptr(lightPos[li]));
            glUniform1f(glGetUniformLocation(progDepth, "uFarPlane"), farPlane[li]);
            for (int face = 0; face < 6; face++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeArray, 0, li * 6 + face);
                glClear(GL_DEPTH_BUFFER_BIT);
                glm::mat4 vp = cubeProj * faceView(lightPos[li], face);
                glUniformMatrix4fv(locVP, 1, GL_FALSE, glm::value_ptr(vp));
                glDisable(GL_CULL_FACE);
                glm::mat4 modelRoom = glm::scale(glm::mat4(1), glm::vec3(10.f));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(modelRoom));
                drawMesh(cubePosOnly);
                glm::mat4 m1 = glm::translate(glm::mat4(1), glm::vec3(-1, 0.5f, -1));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(m1));
                drawMesh(cubePosOnly);
                glm::mat4 m2 =
                    glm::translate(glm::mat4(1), glm::vec3(2, 1, 0)) * glm::scale(glm::mat4(1), glm::vec3(1.5f));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(m2));
                drawMesh(cubePosOnly);
                glm::mat4 m3 =
                    glm::translate(glm::mat4(1), glm::vec3(0, 0.25f, 2)) * glm::scale(glm::mat4(1), glm::vec3(0.5f));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(m3));
                drawMesh(cubePosOnly);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
        }

        // 2) Lighting pass
        int fbw, fbh;
        glfwGetFramebufferSize(win, &fbw, &fbh);
        glViewport(0, 0, fbw, fbh);
        glClearColor(0.1f, 0.12f, 0.14f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(progLit);
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));
        glUniformMatrix4fv(glGetUniformLocation(progLit, "uView"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(progLit, "uProj"), 1, GL_FALSE, glm::value_ptr(proj));
        glUniform1i(glGetUniformLocation(progLit, "uNumLights"), NUM_LIGHTS);
        glUniform3fv(glGetUniformLocation(progLit, "uLightPos"), NUM_LIGHTS, glm::value_ptr(lightPos[0]));
        glUniform3fv(glGetUniformLocation(progLit, "uLightColor"), NUM_LIGHTS, glm::value_ptr(lightColor[0]));
        glUniform1fv(glGetUniformLocation(progLit, "uFarPlane"), NUM_LIGHTS, farPlane);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, depthCubeArray);
        glUniform1i(glGetUniformLocation(progLit, "uDepthCubeArray"), 0);

        // Draw room
        GLint uModel = glGetUniformLocation(progLit, "uModel");
        glDisable(GL_CULL_FACE);
        glm::mat4 modelRoom = glm::scale(glm::mat4(1), glm::vec3(10.f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(modelRoom));
        glUniform1i(glGetUniformLocation(progLit, "uInvertNormals"), 1);
        glUniform3f(glGetUniformLocation(progLit, "uAlbedo"), 0.35f, 0.34f, 0.36f);
        drawMesh(cubeLit);

        // Objects
        glUniform1i(glGetUniformLocation(progLit, "uInvertNormals"), 0);
        glUniform3f(glGetUniformLocation(progLit, "uAlbedo"), 0.85f, 0.85f, 0.9f);
        glm::mat4 m1 = glm::translate(glm::mat4(1), glm::vec3(-1, 0.5f, -1));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m1));
        drawMesh(cubeLit);
        glm::mat4 m2 = glm::translate(glm::mat4(1), glm::vec3(2, 1, 0)) * glm::scale(glm::mat4(1), glm::vec3(1.5f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m2));
        drawMesh(cubeLit);
        glm::mat4 m3 = glm::translate(glm::mat4(1), glm::vec3(0, 0.25f, 2)) * glm::scale(glm::mat4(1), glm::vec3(0.5f));
        glUniformMatrix4fv(uModel, 1, GL_FALSE, glm::value_ptr(m3));
        drawMesh(cubeLit);

        glfwSwapBuffers(win);
    }
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
