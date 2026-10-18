#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shaders/shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cmath>


class Object {
public:
    virtual bool intersect(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float &t0, float &t1) const = 0;
};

class Sphere : public Object {
public:
    bool intersect(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float &t0, float &t1) const override {
        glm::vec3 oc = ray_origin - center;
        float a = 1.0f; 
        float b = 2.0f * glm::dot(oc, ray_direction); 
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;
        float sqrtd = std::sqrt(discriminant);
        t0 = (-b - sqrtd) / (2.0f * a);
        t1 = (-b + sqrtd) / (2.0f * a);
        if (t0 > t1) std::swap(t0, t1);
        return t1 >= 0;
    }
    float sigma_a{ 0.3f }; 
    glm::vec3 scatter{ 0.8f, 0.2f, 0.3f }; 
    glm::vec3 center{ 0.0f, 0.0f, -4.0f }; 
    float radius{ 1.0f }; 
};

glm::vec3 computeRay(float col, float row, int width, int height) {
    float x = (2.0f * (col + 0.5f) / (float)width - 1.0f) * ((float)width / (float)height);
    float y = (1.0f - 2.0f * (row + 0.5f) / (float)height);
    glm::vec3 dir = {x, y, -1.0f}; 
    return glm::normalize(dir); 
}

glm::vec3 traceScene(const glm::vec3& ray_origin, const glm::vec3& ray_direction, const Sphere *sphere) 
{ 
    float t0, t1; 
    glm::vec3 background_color { 0.572f, 0.772f, 0.921f }; 
    if (sphere->intersect(ray_origin, ray_direction, t0, t1)) { 
        if (t0 < 0) t0 = 0; 
        glm::vec3 p1 = ray_origin + ray_direction * t0; 
        glm::vec3 p2 = ray_origin + ray_direction * t1; 
        float distance = glm::length(p2 - p1);
        float transmission = std::exp(-distance * sphere->sigma_a); 
        return background_color * transmission + sphere->scatter * (1.0f - transmission); 
    } 
    else {
        return background_color; 
    }
} 

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);

int main() {
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    const int IMG_WIDTH = 400;
    const int IMG_HEIGHT = 300;

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    Shader shadertri("include/shaders/vert.sh", "include/shaders/frag.sh");

    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_WIDTH, IMG_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    Sphere *sphere = new Sphere();
    std::vector<unsigned char> image_buffer(IMG_WIDTH * IMG_HEIGHT * 3);
    glm::vec3 ray_origin{0.0f, 0.0f, 0.0f};

    while (!glfwWindowShouldClose(window)) {
        process_input(window);

        for (float row = 0; row < IMG_HEIGHT; ++row) {
            for (float col = 0; col < IMG_WIDTH; ++col) {
                glm::vec3 ray_dir = computeRay(col, row, IMG_WIDTH, IMG_HEIGHT);
                glm::vec3 pixel_color = traceScene(ray_origin, ray_dir, sphere);
                
                size_t index = (row * IMG_WIDTH + col) * 3;
                image_buffer[index + 0] = static_cast<unsigned char>(std::min(pixel_color.x * 255.0f, 255.0f));
                image_buffer[index + 1] = static_cast<unsigned char>(std::min(pixel_color.y * 255.0f, 255.0f));
                image_buffer[index + 2] = static_cast<unsigned char>(std::min(pixel_color.z * 255.0f, 255.0f));
            }
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, IMG_WIDTH, IMG_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, image_buffer.data());

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shadertri.use();

        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete sphere;
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}
