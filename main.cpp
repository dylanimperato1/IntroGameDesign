/**
* Author: Dylan Imperato
* Assignment: Simple 2D Scene
* Date due: 2023-06-11, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "main.hpp"
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << '\n'

#include <cmath>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

const int VIEWPORT_X      = 0,
          VIEWPORT_Y      = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const int NUM_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

const char PLAYER_SPRITE_1[] = "bloon2.png";
const char PLAYER_SPRITE_2[] = "bloon3.png";

GLuint player_texture_id_1;
GLuint player_texture_id_2;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND     = 90.0f;

SDL_Window* g_display_window;

bool g_game_is_running = true;
bool g_is_growing      = true;

ShaderProgram g_program;
glm::mat4 g_view_matrix,
          g_model_matrix_one,
          g_model_matrix_two,
          g_projection_matrix;

float g_bloon1_x      = 0.0f;
float g_bloon2_x      = 0.0f;
float g_bloon1_y      = 0.0f;
float g_bloon2_y      = 0.0f;

float g_triangle_rotate = 0.0f;
float g_previous_ticks  = 0.0f;

GLuint load_texture(const char* filepath){
    
    int width, height, num_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &num_of_components, STBI_rgb_alpha);
    
    if(image == NULL){
        LOG("pooped");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Hello, Delta Time!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    player_texture_id_1 = load_texture(PLAYER_SPRITE_1);
    player_texture_id_2 = load_texture(PLAYER_SPRITE_2);

    
    g_view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.
    
    g_program.SetProjectionMatrix(g_projection_matrix);
    g_program.SetViewMatrix(g_view_matrix);
    // Notice we haven't set our model matrix yet!
        
    glUseProgram(g_program.programID);
    
    glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE)
        {
            g_game_is_running = false;
        }
    }
}

const int MAX_FRAME = 3;

float g_counter = 0;
int g_sliding = 1;

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    
    g_previous_ticks = ticks;

    g_bloon1_x += 1.0f * delta_time * g_sliding;
    LOG(g_bloon1_x);
    g_bloon2_x += 1.0f * delta_time * g_sliding;
    
    g_counter += 1.0f * delta_time;
    
    if(g_counter >= MAX_FRAME){
        g_sliding *= -1;
        g_counter = 0;
    }
    

    g_triangle_rotate += DEGREES_PER_SECOND * delta_time;
    
    g_model_matrix_one = glm::mat4(1.0f);
    
    g_model_matrix_two = glm::mat4(1.0f);

    g_model_matrix_one = glm::translate(g_model_matrix_one, glm::vec3(g_bloon1_x, 0.0f, 0.0f));
    g_model_matrix_one = glm::rotate(g_model_matrix_one, glm::radians(g_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    
    g_model_matrix_two = glm::translate(g_model_matrix_two, glm::vec3(0.0f, g_bloon2_x, 0.0f));
    g_model_matrix_two = glm::rotate(g_model_matrix_two, glm::radians(-g_triangle_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id){
    g_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
        
    float vertices[] = {
         -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
         -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };
    
    glVertexAttribPointer(g_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.positionAttribute);
    
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.texCoordAttribute);
    
    draw_object(g_model_matrix_one, player_texture_id_1);
    
    draw_object(g_model_matrix_two, player_texture_id_2);
    
    glDisableVertexAttribArray(g_program.positionAttribute);
    glDisableVertexAttribArray(g_program.texCoordAttribute);
    
    SDL_GL_SwapWindow(g_display_window);


}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too much about the details yet.
 */
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
