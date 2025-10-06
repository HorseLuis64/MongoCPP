#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <string>


void onSuccess(emscripten_fetch_t *fetch) {
    std::cout << "Servidor respondió: " << std::string(fetch->data, fetch->numBytes) << std::endl;
    emscripten_fetch_close(fetch);
}

void onError(emscripten_fetch_t *fetch) {
    std::cout << "Error al enviar datos." << std::endl;
    emscripten_fetch_close(fetch);
}

void enviarDatos() {
    std::string jsonData = R"({"nombre":"Jorge","edad":21})";

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "POST");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = onSuccess;
    attr.onerror = onError;
    attr.requestData = jsonData.c_str();
    attr.requestDataSize = jsonData.size();
    const char* headers[] = {
        "Content-Type", "application/json",
        nullptr
    };
    attr.requestHeaders = headers;

    emscripten_fetch(&attr, "http://localhost:9080/insert");
}

int bWidth, bHeight;

void printViewportSize() {
    
    emscripten_get_screen_size(&bWidth, &bHeight);
    
    std::cout << "📏 Tamaño total de pantalla: " << bWidth << "x" << bHeight << std::endl;

    double availW, availH;
    emscripten_get_element_css_size("window", &availW, &availH);
    std::cout << "📺 Espacio disponible (sin barra de tareas): " << availW << "x" << availH << std::endl;

    double winW, winH;
    emscripten_get_element_css_size("window", &winW, &winH);
    std::cout << "🪟 Tamaño visible del navegador (viewport): " << winW << "x" << winH << std::endl;
}



// Variables globales para WebGL
GLuint program;
GLuint vbo;

// Coordenadas del botón (en espacio NDC)

bool buttonPressed = false;

struct Boton {
    float x, y, w, h;
    float nx, ny, nw, nh;
} button = {300, 600, 600, 800,
          -0.3f, -0.1f, 0.6f, 0.2f};

int canvasWidth = 800;
int canvasHeight = 600;

struct vec2
{
    public:
    int x;
    int y;
    vec2(int x, int y) : x(x), y(y){}
};

vec2 ndcToPx(float x, float y)
{
    vec2 stride = vec2(bWidth /2, bHeight / 2);
    vec2 r(1, 1);
    
    r.x = (stride.x * x) + stride.x;

    r.y = (stride.y * y) + stride.y;
    
    return r;
}

bool dentroDelBoton(float mouseX, float mouseY) {
    return mouseX >= button.x && mouseX <= button.x + button.w;
           
}

// Shaders simples
const char* vertexShaderSource = R"(
attribute vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
precision mediump float;
uniform vec3 uColor;
void main() {
    gl_FragColor = vec4(uColor, 1.0);
}
)";

// Compilar un shader
GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    return shader;
}

// Dibujar el botón
void draw() {
    glClearColor(0.15f, 0.18f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLint posLoc = glGetAttribLocation(program, "aPos");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer(posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

    GLint colorLoc = glGetUniformLocation(program, "uColor");
    if (buttonPressed)
        glUniform3f(colorLoc, 0.0f, 1.0f, 0.4f);
    else
        glUniform3f(colorLoc, 0.2f, 0.6f, 1.0f);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}





// Manejar click del mouse

EM_BOOL onClick(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    float mouseX = e->targetX;
    float mouseY = e->targetY;

    // En Emscripten, (0,0) está arriba-izquierda del canvas
    // Si tus coordenadas del botón están medidas igual, no hace falta invertir Y.
    std::cout << "Click en: " << mouseX << ", " << mouseY << std::endl;

    if (dentroDelBoton(mouseX, mouseY)) {
        std::cout << "✅ Click dentro del botón!" << std::endl;
        // Llama aquí tu función enviarDatos();
    } else {
        std::cout << "❌ Click fuera del botón." << std::endl;
    }
    vec2 a = ndcToPx(-0.3f, -0.1f);
    vec2 b = ndcToPx(0.6f, 0.2f);

      std::cout<< a.x <<" " << b.y << " s " << b.x << " " << b.y<<std::endl; 

    return EM_TRUE;
}

int oh = 0;
// Loop principal
void loop() {
    draw();
    buttonPressed = false; // vuelve al color original tras dibujar
    if(oh < 1)
    {
      vec2 l = ndcToPx(button.nx, button.ny);
      vec2 h = ndcToPx(button.nw, button.nh);
      button.x = l.x;
      button.y = l.y;
      button.w = h.x;
      button.h = h.y;
          
      oh++;
    }
}

double viewX;
double viewY;
double viewW;
double viewH;

int main() {
    // Crear contexto WebGL
    
    
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
    emscripten_webgl_make_context_current(ctx);

    // Compilar shaders
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    // Crear VBO (rectángulo)
    float vertices[] = {
        button.nx, button.ny,
        button.nx + button.nw, button.ny,
        button.nx, button.ny + button.nh,
        button.nx + button.nw, button.ny + button.nh
    };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Registrar click
    emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, onClick);

    double ws, hs;
    emscripten_get_element_css_size("#canvas", &viewW, &viewW);
    std::cout << "Ventana (CSS) -> ancho: " << ws << ", alto: " << hs << std::endl;


    std::cout << "🎨 Frontend C++ iniciado (WebGL + Fetch)\n";

      

    printViewportSize();
    emscripten_set_main_loop(loop, 0, 1);
    

      
    return 0;
}
