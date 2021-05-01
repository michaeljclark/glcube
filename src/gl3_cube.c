/*
 * glcube
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "gl2_util.h"

typedef struct model_object {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    vertex_buffer vb;
    index_buffer ib;
    mat4x4 m;
} model_object_t;

typedef struct zoom_state {
    float zoom;
    vec2f mouse_pos;
    vec2f origin;
    vec3f rotation;
} zoom_state_t;

static const char* frag_shader_filename = "shaders/cube.fsh";
static const char* vert_shader_filename = "shaders/cube.vsh";

static GLfloat angle = 0.f;
static bool help = 0;
static bool debug = 0;
static bool animation = 0;
static GLuint program;
static mat4x4 v, p;
static model_object_t mo[1];
static zoom_state_t state = { 32.0f, { 0.f }, { 0.f }, { 20.f, 30.f, 0.f } }, state_save;
static const float min_zoom = 16.0f, max_zoom = 32768.0f;
static bool mouse_left_drag = false;
static bool mouse_right_drag = false;

static void model_object_init(model_object_t *mo)
{
    vertex_buffer_init(&mo->vb);
    index_buffer_init(&mo->ib);
}

static void model_object_freeze(model_object_t *mo)
{
    glGenVertexArrays(1, &mo->vao);
    glBindVertexArray(mo->vao);
    buffer_object_create(&mo->vbo, GL_ARRAY_BUFFER, &mo->vb);
    buffer_object_create(&mo->ibo, GL_ELEMENT_ARRAY_BUFFER, &mo->ib);
    vertex_array_pointer("a_pos", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,pos));
    vertex_array_pointer("a_normal", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,norm));
    vertex_array_pointer("a_uv", 2, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,uv));
    vertex_array_pointer("a_color", 4, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,col));
}

static void model_object_cube(model_object_t *mo, float s, vec4f col)
{
    float r = col.r, g = col.g, b = col.b, a = col.a;

    const float f[6][3][3] = {
        /* front */  { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, },
        /* right */  { { 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 }, },
        /* top */    { { 0, 1, 0 }, { 0, 0, 1 }, { 1, 0, 0 }, },
        /* left */   { { 0, 0,-1 }, { 0, 1, 0 }, { 1, 0, 0 }, },
        /* bottom */ { { 1, 0, 0 }, { 0, 0,-1 }, { 0, 1, 0 }, },
        /* back */   { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0,-1 }, }
    };

    const vertex t[4] = {
        { { -s,  s,  s }, { 0, 0, 1 }, { 0, 1 }, { r, g, b, a } },
        { { -s, -s,  s }, { 0, 0, 1 }, { 0, 0 }, { r, g, b, a } },
        { {  s, -s,  s }, { 0, 0, 1 }, { 1, 0 }, { r, g, b, a } },
        { {  s,  s,  s }, { 0, 0, 1 }, { 1, 1 }, { r, g, b, a } },
    };

    const float colors[6][4] = {
        { 1.0f, 0.0f, 0.0f, 1 }, /* red */
        { 0.0f, 1.0f, 0.0f, 1 }, /* green */
        { 0.0f, 0.0f, 1.0f, 1 }, /* blue */
        { 0.0f, 0.7f, 0.7f, 1 }, /* cyan */
        { 0.7f, 0.0f, 0.7f, 1 }, /* magenta */
        { 0.7f, 0.7f, 0.0f, 1 }, /* yellow */
    };

    uint idx = vertex_buffer_count(&mo->vb);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            vertex v = t[j];
            v.col.r = colors[i][0];
            v.col.g = colors[i][1];
            v.col.b = colors[i][2];
            v.col.a = colors[i][3];
            v.pos.x = f[i][0][0]*t[j].pos.x + f[i][0][1]*t[j].pos.y + f[i][0][2]*t[j].pos.z;
            v.pos.y = f[i][1][0]*t[j].pos.x + f[i][1][1]*t[j].pos.y + f[i][1][2]*t[j].pos.z;
            v.pos.z = f[i][2][0]*t[j].pos.x + f[i][2][1]*t[j].pos.y + f[i][2][2]*t[j].pos.z;
            v.norm.x = f[i][0][0]*t[j].norm.x + f[i][0][1]*t[j].norm.y + f[i][0][2]*t[j].norm.z;
            v.norm.y = f[i][1][0]*t[j].norm.x + f[i][1][1]*t[j].norm.y + f[i][1][2]*t[j].norm.z;
            v.norm.z = f[i][2][0]*t[j].norm.x + f[i][2][1]*t[j].norm.y + f[i][2][2]*t[j].norm.z;
            vertex_buffer_add(&mo->vb, v);
        }
    }
    index_buffer_add_primitves(&mo->ib, primitive_topology_quads, 6, idx);
}

static void model_view_pos(float zoom, vec3f rot)
{
    mat4x4_translate(v, 0.f, 0.f, zoom);
    mat4x4_rotate(v, v, 1.f, 0.f, 0.f, (rot.x / 180.f) * M_PI);
    mat4x4_rotate(v, v, 0.f, 1.f, 0.f, (rot.y / 180.f) * M_PI);
    mat4x4_rotate(v, v, 0.f, 0.f, 1.f, (rot.z / 180.f) * M_PI);
}

static void model_object_pos(model_object_t *mo,float angle,
    float posx, float posy, float posz)
{
    mat4x4 m;
    mat4x4_translate(m, posx, posy, posz);
    mat4x4_rotate_Y(mo->m, m, (angle / 180) * M_PI);
}

static void model_object_draw(model_object_t *mo)
{
    glBindVertexArray(mo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mo->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mo->ibo);
    glBindVertexArray(mo->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)mo->ib.count, GL_UNSIGNED_INT, (void*)0);
}

static void draw()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    model_view_pos(-state.zoom, state.rotation);
    model_object_pos(&mo[0], angle, state.origin.x * 0.01f, state.origin.y * 0.01f, 0.f);
    uniform_matrix_4fv("u_model", (const GLfloat *)mo->m);
    uniform_matrix_4fv("u_view", (const GLfloat *)v);
    model_object_draw(&mo[0]);
}

static float last_time, current_time, delta_time;

static void animate()
{
    last_time = current_time;
    current_time = (float) glfwGetTime();
    delta_time = current_time - last_time;

    if (animation) {
        angle += 100.f * delta_time;
    }
}

void reshape( GLFWwindow* window, int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;

    glViewport(0, 0, (GLint) width, (GLint) height);
    mat4x4_frustum(p, -1., 1., -h, h, 5.f, 1e9f);
    uniform_matrix_4fv("u_projection", (const GLfloat *)p);
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    float quantum = state.zoom / 16.f;
    float ratio = 1.f + (float)quantum / (float)state.zoom;
    if (yoffset < 0. && state.zoom < max_zoom) {
        state.origin.x *= ratio;
        state.origin.y *= ratio;
        state.zoom += quantum;
    } else if (yoffset > 0. && state.zoom > min_zoom) {
        state.origin.x /= ratio;
        state.origin.y /= ratio;
        state.zoom -= quantum;
    }
}

static void mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        mouse_left_drag = (action == GLFW_PRESS);
        state_save = state;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mouse_right_drag = (action == GLFW_PRESS);
        state_save = state;
        break;
    }
}

static void cursor_position(GLFWwindow* window, double xpos, double ypos)
{
    state.mouse_pos.x = xpos;
    state.mouse_pos.y = ypos;

    if (mouse_left_drag) {
        state.origin.x += state.mouse_pos.x - state_save.mouse_pos.x;
        state.origin.y += state.mouse_pos.y - state_save.mouse_pos.y;
        state_save.mouse_pos.x = state.mouse_pos.x;
        state_save.mouse_pos.y = state.mouse_pos.y;
    }
    if (mouse_right_drag) {
        float delta0 = state.mouse_pos.x - state_save.mouse_pos.x;
        float delta1 = state.mouse_pos.y - state_save.mouse_pos.y;
        float zoom = state_save.zoom * powf(65.0f/64.0f,(float)-delta1);
        if (zoom != state.zoom && zoom > min_zoom && zoom < max_zoom) {
            state.zoom = zoom;
            state.origin.x = (state.origin.x * (zoom / state.zoom));
            state.origin.y = (state.origin.y * (zoom / state.zoom));
        }
    }
}

void key( GLFWwindow* window, int k, int s, int action, int mods )
{
    if( action != GLFW_PRESS ) return;

    float shiftz = (mods & GLFW_MOD_SHIFT ? -1.f : 1.f);

    switch (k) {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
    case GLFW_KEY_X: animation = !animation; break;
    case GLFW_KEY_Z: state.rotation.z += 5.f * shiftz; break;
    case GLFW_KEY_C: state.zoom += 5.f * shiftz; break;
    case GLFW_KEY_W: state.rotation.x += 5.f; break;
    case GLFW_KEY_S: state.rotation.x -= 5.f; break;
    case GLFW_KEY_A: state.rotation.y += 5.f; break;
    case GLFW_KEY_D: state.rotation.y -= 5.f; break;
    default: return;
    }
}

static void init()
{
    GLuint shaders[2];

    /* shader program */
    shaders[0] = compile_shader(GL_VERTEX_SHADER, vert_shader_filename);
    shaders[1] = compile_shader(GL_FRAGMENT_SHADER, frag_shader_filename);
    program = link_program(shaders, 2, NULL);

    /* create cube vertex and index buffers and buffer objects */
    model_object_init(&mo[0]);
    model_object_cube(&mo[0], 3.f, (vec4f){0.3f, 0.3f, 0.3f, 1.f});
    model_object_freeze(&mo[0]);

    if (debug) {
        vertex_buffer_dump(&mo[0].vb);
    }

    /* set light position uniform */
    glUseProgram(program);
    uniform_3f("u_lightpos", 5.f, 5.f, 10.f);

    /* enable OpenGL capabilities */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

static void print_help(int argc, char **argv)
{
    fprintf(stderr,
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -d, --debug                        debug geometry\n"
        "  -h, --help                         command line help\n",
        argv[0]);
}

static int match_opt(const char *arg, const char *opt, const char *longopt)
{
    return strcmp(arg, opt) == 0 || strcmp(arg, longopt) == 0;
}

static void parse_options(int argc, char **argv)
{
    int i = 1;
    while (i < argc) {
        if (match_opt(argv[i], "-d", "--debug")) {
            debug++;
            i++;
        } else if (match_opt(argv[i], "-h", "--help")) {
            help++;
            i++;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            help++;
            break;
        }
    }

    if (help) {
        print_help(argc, argv);
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    GLFWwindow* window;
    int width, height;

    parse_options(argc, argv);

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow( 1024, 1024, "OpenGL Cube", NULL, NULL );
    if (!window)
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();

    glfwSetFramebufferSizeCallback(window, reshape);
    glfwSetKeyCallback(window, key);
    glfwSetScrollCallback(window, scroll);
    glfwSetMouseButtonCallback(window, mouse_button);
    glfwSetCursorPosCallback(window, cursor_position);
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &width, &height);
    glfwSwapInterval(1);
    glfwSetWindowOpacity(window, 0.99f);

    init();
    reshape(window, width, height);

    while(!glfwWindowShouldClose(window)) {
        animate();
        draw(&mo[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

