#include "../include/renderer.h"

Renderer::Renderer()
        : shaderProgram(0), positionAttrib(-1), texCoordAttrib(-1), textureUniform(-1) {}

Renderer::~Renderer() {
    cleanup();
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        SDL_Log("Shader compilation error (%s): %s",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", infoLog);
    }
    return shader;
}

bool Renderer::init() {
    const char* vertexShaderSource =
            "attribute vec2 aPosition;\n"
            "attribute vec2 aTexCoord;\n"
            "varying vec2 vTexCoord;\n"
            "void main() {\n"
            "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
            "    vTexCoord = aTexCoord;\n"
            "}\n";

    const char* fragmentShaderSource =
            "precision mediump float;\n"
            "varying vec2 vTexCoord;\n"
            "uniform sampler2D uTexture;\n"
            "void main() {\n"
            "    gl_FragColor = texture2D(uTexture, vTexCoord);\n"
            "}\n";

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        SDL_Log("Shader linking error: %s", infoLog);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    positionAttrib = glGetAttribLocation(shaderProgram, "aPosition");
    texCoordAttrib = glGetAttribLocation(shaderProgram, "aTexCoord");
    textureUniform = glGetUniformLocation(shaderProgram, "uTexture");

    // Habilitar transparencia (Blending) para los sprites
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

GLuint Renderer::loadTexture(const char* filePath) {
    SDL_Surface* surface = SDL_LoadBMP(filePath);
    if (!surface) {
        SDL_Log("Could not load texture: %s", SDL_GetError());
        return 0;
    }

    // Forzamos a que SDL convierta cualquier BMP a un formato que OpenGL ES entienda bien (RGBA)
    SDL_Surface* optimizedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);

    if (!optimizedSurface) return 0;

    GLuint tid;
    glGenTextures(1, &tid);
    glBindTexture(GL_TEXTURE_2D, tid);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Ahora enviamos optimizedSurface que es garantizado RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, optimizedSurface->w, optimizedSurface->h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, optimizedSurface->pixels);

    SDL_DestroySurface(optimizedSurface);
    loadedTextures.push_back(tid);
    return tid;
}

void Renderer::draw(const std::vector<Sprite>& sprites, int windowWidth, int windowHeight) {
    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    // Coordenadas UV: mapean las 4 esquinas de la textura al cuadrado
    float texCoords[] = {
            0.0f, 1.0f, // Inferior Izquierda
            1.0f, 1.0f, // Inferior Derecha
            0.0f, 0.0f, // Superior Izquierda
            1.0f, 0.0f  // Superior Derecha
    };

    glEnableVertexAttribArray(positionAttrib);
    glEnableVertexAttribArray(texCoordAttrib);

    // PASO IMPORTANTE: Enviar las coordenadas de textura
    // Si todos los sprites usan la textura completa, puedes ponerlo fuera del bucle
    glVertexAttribPointer(texCoordAttrib, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    for (const auto& sprite : sprites) {
        float halfW = sprite.width / 2.0f;
        float halfH = sprite.height / 2.0f;

        float vertices[] = {
                sprite.x - halfW, sprite.y - halfH,
                sprite.x + halfW, sprite.y - halfH,
                sprite.x - halfW, sprite.y + halfH,
                sprite.x + halfW, sprite.y + halfH
        };

        glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 0, vertices);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sprite.textureID);
        glUniform1i(textureUniform, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glDisableVertexAttribArray(positionAttrib);
    glDisableVertexAttribArray(texCoordAttrib);
}

void Renderer::cleanup() {
    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    if (!loadedTextures.empty()) {
        glDeleteTextures(loadedTextures.size(), loadedTextures.data());
        loadedTextures.clear();
    }
}