#ifndef SDL3PROJECT_RENDERER_H
#define SDL3PROJECT_RENDERER_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>
#include <GLES2/gl2.h>
#include <vector>

struct Sprite {
    float x, y;          // Posición en coordenadas NDC (-1.0 a 1.0)
    float width, height; // Tamaño relativo (1.0 es la mitad de la pantalla)
    GLuint textureID;    // ID de la textura cargada
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();

    GLuint loadTexture(const char* filePath);

    void draw(const std::vector<Sprite>& sprites, int windowWidth, int windowHeight);

    void cleanup();

private:
    GLuint compileShader(GLenum type, const char* source);

    GLuint shaderProgram;
    GLint positionAttrib;
    GLint texCoordAttrib;
    GLint textureUniform;

    std::vector<GLuint> loadedTextures;
};

#endif //SDL3PROJECT_RENDERER_H