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

typedef struct mvp_t {
    mat4x4 projection;
    mat4x4 model;
    mat4x4 view;
    vec4 lightpos;
} mvp_t;

typedef struct model_object {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint ubo;
    vertex_buffer vb;
    index_buffer ib;
    mat4x4 m, v;
    mvp_t mvp;
} model_object_t;

typedef struct zoom_state {
    float zoom;
    vec2 mouse_pos;
    vec2 origin;
    vec3 rotation;
} zoom_state_t;

static const char* frag_shader_filename = "shaders/cube.frag";
static const char* vert_shader_filename = "shaders/cube.vert";

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
        /* rear */   { { 0, 1, 0 }, { 1, 0, 0 }, { 0, 0,-1 }, },
        /* left */   { { 0, 0,-1 }, { 0, 1, 0 }, { 1, 0, 0 }, },
        /* bottom */ { { 1, 0, 0 }, { 0, 0,-1 }, { 0, 1, 0 }, },
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
        { 0.7f, 0.0f, 0.7f, 1 }, /* magenta */
        { 0.7f, 0.7f, 0.0f, 1 }, /* yellow */
        { 0.0f, 0.7f, 0.7f, 1 }, /* cyan */
    };

    uint idx = vertex_buffer_count(&mo->vb);
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 4; j++) {
            vertex v;
            v.pos.x = f[i][0][0]*t[j].pos.x + f[i][0][1]*t[j].pos.y + f[i][0][2]*t[j].pos.z;
            v.pos.y = f[i][1][0]*t[j].pos.x + f[i][1][1]*t[j].pos.y + f[i][1][2]*t[j].pos.z;
            v.pos.z = f[i][2][0]*t[j].pos.x + f[i][2][1]*t[j].pos.y + f[i][2][2]*t[j].pos.z;
            v.norm.x = f[i][0][0]*t[j].norm.x + f[i][0][1]*t[j].norm.y + f[i][0][2]*t[j].norm.z;
            v.norm.y = f[i][1][0]*t[j].norm.x + f[i][1][1]*t[j].norm.y + f[i][1][2]*t[j].norm.z;
            v.norm.z = f[i][2][0]*t[j].norm.x + f[i][2][1]*t[j].norm.y + f[i][2][2]*t[j].norm.z;
            v.uv.x = t[j].uv.x;
            v.uv.y = t[j].uv.y;
            v.col.r = colors[i][0];
            v.col.g = colors[i][1];
            v.col.b = colors[i][2];
            v.col.a = colors[i][3];
            vertex_buffer_add(&mo->vb, v);
        }
    }
    index_buffer_add_primitves(&mo->ib, primitive_topology_quads, 6, idx);
}

static float degrees_to_radians(float a) { return a * M_PI / 180.0f; }

static void model_matrix_transform(mat4x4 m, vec3 scale, vec3 trans, vec3 rot)
{
    mat4x4_identity(m);
    mat4x4_scale_aniso(m, m, scale[0], scale[1], scale[2]);
    mat4x4_translate_in_place(m, trans[0], trans[1], trans[2]);
    mat4x4_rotate_X(m, m, degrees_to_radians(rot[0]));
    mat4x4_rotate_Y(m, m, degrees_to_radians(rot[1]));
    mat4x4_rotate_Z(m, m, degrees_to_radians(rot[2]));
}

static void model_update_matrices(model_object_t *mo)
{
    memcpy(mo[0].mvp.model, mo[0].m, sizeof(mo[0].m));
    memcpy(mo[0].mvp.view, mo[0].v, sizeof(mo[0].v));

    glBindBuffer(GL_UNIFORM_BUFFER, mo->ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mo[0].mvp), &mo[0].mvp);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static void model_object_draw(model_object_t *mo)
{
    glBindVertexArray(mo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mo->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mo->ibo);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, mo->ubo);
    glDrawElements(GL_TRIANGLES, (GLsizei)mo->ib.count, GL_UNSIGNED_INT, (void*)0);
}

static void draw()
{
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 model_scale = { 1.0f, 1.0f, 1.0f };
    vec3 model_trans = { 0.0f, 0.0f, 0.0f };
    vec3 model_rot = { 0.0f, angle, 0.0f };
    vec3 view_scale = { 1.0f, 1.0f, 1.0f };
    vec3 view_trans = { state.origin[0] * 0.01f, state.origin[1] * 0.01f, -state.zoom };

    model_matrix_transform(mo[0].m, model_scale, model_trans, model_rot);
    model_matrix_transform(mo[0].v, view_scale, view_trans, state.rotation);
    model_update_matrices(&mo[0]);
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
    memcpy(mo[0].mvp.projection, p, sizeof(p));
}

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    float quantum = state.zoom / 16.f;
    float ratio = 1.f + (float)quantum / (float)state.zoom;
    if (yoffset < 0. && state.zoom < max_zoom) {
        state.origin[0] *= ratio;
        state.origin[1] *= ratio;
        state.zoom += quantum;
    } else if (yoffset > 0. && state.zoom > min_zoom) {
        state.origin[0] /= ratio;
        state.origin[1] /= ratio;
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
    state.mouse_pos[0] = xpos;
    state.mouse_pos[1] = ypos;

    if (mouse_left_drag) {
        state.origin[0] += state.mouse_pos[0] - state_save.mouse_pos[0];
        state.origin[1] += state.mouse_pos[1] - state_save.mouse_pos[1];
        state_save.mouse_pos[0] = state.mouse_pos[0];
        state_save.mouse_pos[1] = state.mouse_pos[1];
    }
    if (mouse_right_drag) {
        float delta0 = state.mouse_pos[0] - state_save.mouse_pos[0];
        float delta1 = state.mouse_pos[1] - state_save.mouse_pos[1];
        float zoom = state_save.zoom * powf(65.0f/64.0f,(float)-delta1);
        if (zoom != state.zoom && zoom > min_zoom && zoom < max_zoom) {
            state.zoom = zoom;
            state.origin[0] = (state.origin[0] * (zoom / state.zoom));
            state.origin[1] = (state.origin[1] * (zoom / state.zoom));
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
    case GLFW_KEY_Z: state.rotation[2] += 5.f * shiftz; break;
    case GLFW_KEY_C: state.zoom += 5.f * shiftz; break;
    case GLFW_KEY_W: state.rotation[0] += 5.f; break;
    case GLFW_KEY_S: state.rotation[0] -= 5.f; break;
    case GLFW_KEY_A: state.rotation[1] += 5.f; break;
    case GLFW_KEY_D: state.rotation[1] -= 5.f; break;
    default: return;
    }
}

static GLuint bind(GLuint program)
{
    GLuint blockIndex = glGetUniformBlockIndex(program, "UBO");
    glUniformBlockBinding(program, blockIndex, 0);
    glBindFragDataLocation(program, 0, "outFragColor");
    return GL_TRUE;
}

static void init()
{
    GLuint shaders[2];

    /* shader program */
    shaders[0] = compile_shader(GL_VERTEX_SHADER, vert_shader_filename);
    shaders[1] = compile_shader(GL_FRAGMENT_SHADER, frag_shader_filename);
    program = link_program(shaders, 2, bind);

    /* create cube vertex and index buffers and buffer objects */
    model_object_init(&mo[0]);
    model_object_cube(&mo[0], 3.f, (vec4f){0.3f, 0.3f, 0.3f, 1.f});
    model_object_freeze(&mo[0]);

    if (debug) {
        vertex_buffer_dump(&mo[0].vb);
    }

    /* create uniform buffer object */
    glGenBuffers(1, &mo[0].ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, mo[0].ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(mo[0].mvp), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    /* set light position uniform */
    glUseProgram(program);
    vec4 lightpos = { 5.f, 5.f, 10.f, 0.f };
    memcpy(mo[0].mvp.lightpos, lightpos, sizeof(lightpos));

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

