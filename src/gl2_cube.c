/*
 * glcube
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>

#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define CTX_OPENGL_MAJOR 3
#define CTX_OPENGL_MINOR 3

#include "linmath.h"
#include "gl2_util.h"

static const char* frag_shader_filename = "shaders/cube.fsh";
static const char* vert_shader_filename = "shaders/cube.vsh";

static GLfloat view_dist = -40.0f;
static GLfloat view_rotx = 20.f, view_roty = 30.f, view_rotz = 0.f;
static GLfloat angle = 0.f;
static bool animate_enable = 0;
static bool debug_vertices = 0;

static GLuint program;
static GLuint vao[1];
static GLuint vbo[1];
static GLuint ibo[1];
static vertex_buffer vb[1];
static index_buffer ib[1];
static mat4x4 gm[1];
static mat4x4 m, v, p, mvp;

typedef struct {
    float zoom;
    vec2 mouse_pos;
    vec2 origin;
} zoom_state;
static zoom_state state = { 32.0f }, state_save;
static const float min_zoom = 16.0f, max_zoom = 32768.0f;
static bool mouse_left_drag = false;
static bool mouse_right_drag = false;

static void cube(vertex_buffer *vb, index_buffer *ib, float s, vec4f col)
{
    uint idx;
    float S = s;
    float r = col.r, g = col.g, b = col.b, a = col.a;

    /*
     * front, right, top, left, bottom, back
     *
     * triangles = {0,1,2,2,1,3};
     * quads     = {0,1,2,0,2,3};
     */

    vertex vertices[6*4] = {
        /* front */
        { { -s,  s,  S }, { 0, 0, 1 }, { 0, 1 }, { r, g, b, a } }, /* fr.tl */
        { { -s, -s,  S }, { 0, 0, 1 }, { 0, 0 }, { r, g, b, a } }, /* fr.bl */
        { {  s,  s,  S }, { 0, 0, 1 }, { 1, 1 }, { r, g, b, a } }, /* fr.tr */
        { {  s, -s,  S }, { 0, 0, 1 }, { 1, 0 }, { r, g, b, a } }, /* fr.br */

        /* right */
        { {  S, -s,  s }, { 1, 0, 0 }, { 0, 1 }, { r, g, b, a } }, /* ri.tl */
        { {  S, -s, -s }, { 1, 0, 0 }, { 0, 0 }, { r, g, b, a } }, /* ri.bl */
        { {  S,  s,  s }, { 1, 0, 0 }, { 1, 1 }, { r, g, b, a } }, /* ri.tr */
        { {  S,  s, -s }, { 1, 0, 0 }, { 1, 0 }, { r, g, b, a } }, /* ri.br */

        /* top */
        { {  s,  S, -s }, { 0, 1, 0 }, { 0, 1 }, { r, g, b, a } }, /* to.tl */
        { { -s,  S, -s }, { 0, 1, 0 }, { 0, 0 }, { r, g, b, a } }, /* to.bl */
        { {  s,  S,  s }, { 0, 1, 0 }, { 1, 1 }, { r, g, b, a } }, /* to.tr */
        { { -s,  S,  s }, { 0, 1, 0 }, { 1, 0 }, { r, g, b, a } }, /* to.br */

        /* left */
        { { -S,  s, -s }, {-1, 0, 0 }, { 0, 1 }, { r, g, b, a } }, /* le.tl */
        { { -S, -s, -s }, {-1, 0, 0 }, { 0, 0 }, { r, g, b, a } }, /* le.bl */
        { { -S,  s,  s }, {-1, 0, 0 }, { 1, 1 }, { r, g, b, a } }, /* le.tr */
        { { -S, -s,  s }, {-1, 0, 0 }, { 1, 0 }, { r, g, b, a } }, /* le.br */

        /* bottom */
        { { -s, -S,  s }, { 0,-1, 0 }, { 0, 1 }, { r, g, b, a } }, /* bo.tl */
        { { -s, -S, -s }, { 0,-1, 0 }, { 0, 0 }, { r, g, b, a } }, /* bo.bl */
        { {  s, -S,  s }, { 0,-1, 0 }, { 1, 1 }, { r, g, b, a } }, /* bo.tr */
        { {  s, -S, -s }, { 0,-1, 0 }, { 1, 0 }, { r, g, b, a } }, /* bo.br */

        /* back */
        { {  s, -s, -S }, { 0, 0,-1 }, { 0, 1 }, { r, g, b, a } }, /* ba.tl */
        { { -s, -s, -S }, { 0, 0,-1 }, { 0, 0 }, { r, g, b, a } }, /* ba.bl */
        { {  s,  s, -S }, { 0, 0,-1 }, { 1, 1 }, { r, g, b, a } }, /* ba.tr */
        { { -s,  s, -S }, { 0, 0,-1 }, { 1, 0 }, { r, g, b, a } }, /* ba.br */
    };

    idx = vertex_buffer_count(vb);
    for (int i = 0; i < 24; i++) vertex_buffer_add(vb, vertices[i]);
    index_buffer_add_primitves(ib, primitive_topology_quad_strip, 12, idx);

    if (debug_vertices) {
        vertex_buffer_dump(vb);
    }
}

static void draw()
{
    //mat4x4_translate(v, 0.0, 0.0, view_dist);
    mat4x4_translate(v, 0.0, 0.0, -state.zoom);
    mat4x4_rotate(v, v, 1.0, 0.0, 0.0, (view_rotx / 180) * M_PI);
    mat4x4_rotate(v, v, 0.0, 1.0, 0.0, (view_roty / 180) * M_PI);
    mat4x4_rotate(v, v, 0.0, 0.0, 1.0, (view_rotz / 180) * M_PI);
    mat4x4_translate(m, state.origin[0]*0.01f, state.origin[1]*0.01f, 0.0);
    mat4x4_rotate_Y(gm[0], m, (angle / 180) * M_PI);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao[0]);
    uniform_matrix_4fv("u_model", (const GLfloat *)gm[0]);
    uniform_matrix_4fv("u_view", (const GLfloat *)v);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo[0]);
    glBindVertexArray(vao[0]);
    glDrawElements(GL_TRIANGLES, (GLsizei)ib[0].count, GL_UNSIGNED_INT, (void*)0);
}

static void animate()
{
    if (animate_enable) {
        angle = 100.f * (float) glfwGetTime();
    }
}

void key( GLFWwindow* window, int k, int s, int action, int mods )
{
    if( action != GLFW_PRESS ) return;

    float modm = (mods & GLFW_MOD_SHIFT) ? 1.0f : -1.0f;

    switch (k) {
    case GLFW_KEY_A:     animate_enable = !animate_enable; break;
    case GLFW_KEY_C:     view_dist += 5.0 * modm; break;
    case GLFW_KEY_Z:     view_rotz += 5.0 * modm; break;
    case GLFW_KEY_UP:    view_rotx += 5.0; break;
    case GLFW_KEY_DOWN:  view_rotx -= 5.0; break;
    case GLFW_KEY_LEFT:  view_roty += 5.0; break;
    case GLFW_KEY_RIGHT: view_roty -= 5.0; break;
    case GLFW_KEY_ESCAPE:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    default:
        return;
    }
}

void reshape( GLFWwindow* window, int width, int height )
{
    GLfloat h = (GLfloat) height / (GLfloat) width;

    glViewport(0, 0, (GLint) width, (GLint) height);
    mat4x4_frustum(p, -1.0, 1.0, -h, h, 5.0, 1e9);
    uniform_matrix_4fv("u_projection", (const GLfloat *)p);
}

/* mouse callbacks */

static void scroll(GLFWwindow* window, double xoffset, double yoffset)
{
    float quantum = state.zoom / 16.0f;
    float ratio = 1.0f + (float)quantum / (float)state.zoom;
    if (yoffset < 0 && state.zoom < max_zoom) {
        state.origin[0] *= ratio;
        state.origin[1] *= ratio;
        state.zoom += quantum;
    } else if (yoffset > 0 && state.zoom > min_zoom) {
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

static void init()
{
    GLuint vsh, fsh;

    /* shader program */
    vsh = compile_shader(GL_VERTEX_SHADER, vert_shader_filename);
    fsh = compile_shader(GL_FRAGMENT_SHADER, frag_shader_filename);
    program = link_program(vsh, fsh);

    /* create cube vertex and index buffers */
    vertex_buffer_init(&vb[0]);
    index_buffer_init(&ib[0]);
    cube(&vb[0], &ib[0], 3.0f, (vec4f){0.3f, 0.3f, 0.3f, 1.f});

    /* create vertex array, vertex buffer and index buffer objects */
    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);
    vertex_buffer_create(&vbo[0], GL_ARRAY_BUFFER, vb[0].data, vb[0].count * sizeof(vertex));
    vertex_buffer_create(&ibo[0], GL_ELEMENT_ARRAY_BUFFER, ib[0].data, ib[0].count * sizeof(uint));
    vertex_array_pointer("a_pos", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,pos));
    vertex_array_pointer("a_normal", 3, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,norm));
    vertex_array_pointer("a_uv", 2, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,uv));
    vertex_array_pointer("a_color", 4, GL_FLOAT, 0, sizeof(vertex), offsetof(vertex,col));

    /* set light position uniform */
    glUseProgram(program);
    uniform_3f("u_lightpos", 5.f, 5.f, 10.f);

    /* enable OpenGL capabilities */
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char *argv[])
{
    GLFWwindow* window;
    int width, height;

    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        exit( EXIT_FAILURE );
    }

    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    window = glfwCreateWindow( 1024, 1024, "GL2 Cube", NULL, NULL );
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
        draw();
        animate();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    exit(EXIT_SUCCESS);
}

