#include <SDL3/SDL.h>

//#if defined(__APPLE__)
//    #include <TargetConditionals.h>
//#endif

//TARGET_OS_SIMULATOR

#if defined(PLATFORM_IOS) || defined(ANDROID)
    #include <SDL3/SDL_opengles2.h>
    #include <SDL3/SDL_main.h>
    #define USE_GLES 1
#else
    // On macOS, we need to include gl3.h for shader functions
    #if defined(PLATFORM_MACOS)
        #include <OpenGL/gl3.h>
    #else
        #define GLEW_STATIC
        #include <GL/glew.h>
        #include <SDL3/SDL_opengl.h>
    #endif
    #define USE_GLES 0
#endif

#include <stdio.h>

// Rectangle position (normalized coordinates -1 to 1)
float rectX = 0.0f;
float rectY = 0.0f;
float rectSize = 0.2f;

// Shader sources - different for desktop vs mobile
#if USE_GLES
const char* vertexShaderSource =
    "attribute vec2 aPosition;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
    "}\n";

const char* fragmentShaderSource =
    "precision mediump float;\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(uColor, 1.0);\n"
    "}\n";
#else
const char* vertexShaderSource =
    "#version 120\n"
    "attribute vec2 aPosition;\n"
    "void main() {\n"
    "    gl_Position = vec4(aPosition, 0.0, 1.0);\n"
    "}\n";

const char* fragmentShaderSource =
    "#version 120\n"
    "uniform vec3 uColor;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(uColor, 1.0);\n"
    "}\n";
#endif

GLuint shaderProgram = 0;
GLint positionAttrib = 0;
GLint colorUniform = 0;

// Compile a shader
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check compilation status
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error (%s): %s\n",
               type == GL_VERTEX_SHADER ? "vertex" : "fragment", infoLog);
        SDL_Log("Shader compilation error (%s): %s",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment", infoLog);
    }
    return shader;
}

// Create and link shader program
bool initShaders() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check linking status
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader linking error: %s\n", infoLog);
        SDL_Log("Shader linking error: %s", infoLog);
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Get attribute and uniform locations
    positionAttrib = glGetAttribLocation(shaderProgram, "aPosition");
    colorUniform = glGetUniformLocation(shaderProgram, "uColor");
    
    if (positionAttrib == -1) {
        SDL_Log("ERROR: Could not get aPosition attribute location");
        return false;
    }
    if (colorUniform == -1) {
        SDL_Log("ERROR: Could not get uColor uniform location");
        return false;
    }
    
    SDL_Log("Shaders initialized successfully. Position attrib: %d, Color uniform: %d",
            positionAttrib, colorUniform);
    
    return true;
}

void drawRectangle(int windowWidth, int windowHeight) {
    // Set viewport to match window size
    glViewport(0, 0, windowWidth, windowHeight);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use shader program
    glUseProgram(shaderProgram);
    
    // Set color uniform (orange)
    glUniform3f(colorUniform, 1.0f, 0.5f, 0.0f);
    
    // Rectangle vertices (using triangle strip: 4 vertices)
    float vertices[] = {
        rectX - rectSize, rectY - rectSize,  // Bottom-left
        rectX + rectSize, rectY - rectSize,  // Bottom-right
        rectX - rectSize, rectY + rectSize,  // Top-left
        rectX + rectSize, rectY + rectSize   // Top-right
    };
    
    // Set vertex data
    glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionAttrib);
    
    // Draw rectangle as triangle strip
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glDisableVertexAttribArray(positionAttrib);
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    // Platform-specific GL context setup
#if defined(__IPHONEOS__) || defined(__ANDROID__)
    // Mobile: OpenGL ES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(PLATFORM_WINDOWS)
    // Windows: OpenGL 2.1 compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
    // Desktop (macOS, Linux): OpenGL 2.1 compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow(
        "SDL3 OpenGL ES 2.0 Rectangle",
        800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        printf("Window creation error: %s\n", SDL_GetError());
        SDL_Log("Window creation error: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("OpenGL context error: %s\n", SDL_GetError());
        SDL_Log("OpenGL context error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_GL_MakeCurrent(window, glContext);
    SDL_GL_SetSwapInterval(1);

#if defined(PLATFORM_WINDOWS)
    // CRITICAL: Check OpenGL version BEFORE glewInit
    SDL_Log("OpenGL Info:\n");
    SDL_Log("  Vendor:   %s\n", glGetString(GL_VENDOR));
    SDL_Log("  Renderer: %s\n", glGetString(GL_RENDERER));
    SDL_Log("  Version:  %s\n", glGetString(GL_VERSION));

    // Initialize GLEW - MUST be after context creation
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
      SDL_Log("GLEW failed to initialize: %s\n", glewGetErrorString(glewStatus));
      SDL_GL_DestroyContext(glContext);
      SDL_DestroyWindow(window);
      SDL_Quit();
      return -1;
    }

    SDL_Log("GLEW initialized successfully\n");
    SDL_Log("GLEW version: %s\n", glewGetString(GLEW_VERSION));
#endif
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Initialize shaders
    if (!initShaders()) {
        SDL_Log("Failed to initialize shaders");
        SDL_GL_DestroyContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                     event.type == SDL_EVENT_FINGER_DOWN) {
                int windowWidth, windowHeight;
                SDL_GetWindowSize(window, &windowWidth, &windowHeight);
                
                float x, y;
                if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                    x = event.button.x;
                    y = event.button.y;
                } else {
                    // Touch event - convert normalized coords to pixels
                    x = event.tfinger.x * windowWidth;
                    y = event.tfinger.y * windowHeight;
                }
                
                rectX = (x / (float)windowWidth) * 2.0f - 1.0f;
                rectY = -((y / (float)windowHeight) * 2.0f - 1.0f);
                
                SDL_Log("Click at: %.2f, %.2f (NDC: %.2f, %.2f)", x, y, rectX, rectY);
            }
        }
        
        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        drawRectangle(windowWidth, windowHeight);
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    glDeleteProgram(shaderProgram);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
