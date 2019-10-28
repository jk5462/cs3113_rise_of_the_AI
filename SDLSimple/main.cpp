#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

#define PLATFORM_COUNT 20
#define ENEMY_COUNT 3
#define BULLET_COUNT 30

struct GameState {
    Entity player;
	Entity winMessage;
	Entity loseMessage;
    Entity platforms[PLATFORM_COUNT];
	Entity enemies[ENEMY_COUNT];
	Entity bullets[BULLET_COUNT];
};

GameState state;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Rise of the AI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    state.player.entityType = PLAYER;
    state.player.width = 0.8f;
    state.player.position = glm::vec3(-4, -2, 0);
    state.player.acceleration = glm::vec3(0, -9.81f, 0);
    state.player.textureID = LoadTexture("me.png");
	state.player.direction = RIGHT;
    
    GLuint dirtTextureID = LoadTexture("dirt.png");
    GLuint grassTextureID = LoadTexture("grass.png");
	GLuint evilTextureID = LoadTexture("evil.png");
	GLuint bulletTextureID = LoadTexture("bullet.png");
	GLuint winTextureID = LoadTexture("win.png");
	GLuint loseTextureID = LoadTexture("lose.png");

	state.enemies[0].entityType = ENEMY;
	state.enemies[0].width = 0.8f;
	state.enemies[0].textureID = evilTextureID;
	state.enemies[0].position = glm::vec3(3, -2.25f, 0);
	state.enemies[0].acceleration = glm::vec3(0, -9.81f, 0);
	state.enemies[0].aiState = PATROL;
	state.enemies[0].defaultState = PATROL;
	state.enemies[0].velocity.x = 1.0f;
	state.enemies[0].timer = 0.0f;

	state.enemies[1].entityType = ENEMY;
	state.enemies[1].width = 0.8f;
	state.enemies[1].textureID = evilTextureID;
	state.enemies[1].position = glm::vec3(-4, 1.75f, 0);
	state.enemies[1].acceleration = glm::vec3(0, -9.81f, 0);
	state.enemies[1].aiState = GUARD;
	state.enemies[1].defaultState = GUARD;
	state.enemies[1].direction = RIGHT;
	state.enemies[1].timer = 0.0f;

	state.enemies[2].entityType = ENEMY;
	state.enemies[2].width = 0.8f;
	state.enemies[2].textureID = evilTextureID;
	state.enemies[2].position = glm::vec3(-1, -1.25f, 0);
	state.enemies[2].acceleration = glm::vec3(0, -9.81f, 0);
	state.enemies[2].aiState = IDLE;
	state.enemies[2].defaultState = IDLE;

    
    for (int i = 0; i < 11; i++)
    {
        state.platforms[i].textureID = dirtTextureID;
        state.platforms[i].position = glm::vec3(i - 5.0f, -3.25f, 0);
    }

	state.platforms[11].textureID = dirtTextureID;
	state.platforms[11].position = glm::vec3(-1, -2.25f, 0);

	state.platforms[12].textureID = dirtTextureID;
	state.platforms[12].position = glm::vec3(0, -2.25f, 0);

	state.platforms[13].textureID = dirtTextureID;
	state.platforms[13].position = glm::vec3(1, -2.25f, 0);

	state.platforms[14].textureID = dirtTextureID;
	state.platforms[14].position = glm::vec3(0, -1.25f, 0);
	
	state.platforms[15].textureID = dirtTextureID;
	state.platforms[15].position = glm::vec3(1, -1.25f, 0);

	state.platforms[16].textureID = dirtTextureID;
	state.platforms[16].position = glm::vec3(1, -0.25f, 0);

	state.platforms[17].textureID = dirtTextureID;
	state.platforms[17].position = glm::vec3(-4.5, 0.75f, 0);

	state.platforms[18].textureID = dirtTextureID;
	state.platforms[18].position = glm::vec3(-3.5, 0.75f, 0);

	state.platforms[19].textureID = dirtTextureID;
	state.platforms[19].position = glm::vec3(-2.5, 0.75f, 0);

	for (int i = 0; i < BULLET_COUNT; i++)
	{
		state.bullets[i].entityType = BULLET;
		state.bullets[i].vertices[0] = -0.25;
		state.bullets[i].vertices[1] = -0.05;

		state.bullets[i].vertices[2] = 0.25;
		state.bullets[i].vertices[3] = -0.05;

		state.bullets[i].vertices[4] = 0.25;
		state.bullets[i].vertices[5] = 0.05;

		state.bullets[i].vertices[6] = -0.25;
		state.bullets[i].vertices[7] = -0.05;

		state.bullets[i].vertices[8] = 0.25;
		state.bullets[i].vertices[9] = 0.05;

		state.bullets[i].vertices[10] = -0.25;
		state.bullets[i].vertices[11] = 0.05;

		state.bullets[i].width = 0.3f;
		state.bullets[i].height = 0.1f;

		state.bullets[i].textureID = bulletTextureID;
		state.bullets[i].isActive = false;
	}
    
	state.winMessage.textureID = winTextureID;
	state.winMessage.position = glm::vec3(0, 0, 0);
	state.winMessage.isActive = false;
	state.winMessage.vertices[0] = -2;
	state.winMessage.vertices[2] = 2;
	state.winMessage.vertices[4] = 2;
	state.winMessage.vertices[6] = -2;
	state.winMessage.vertices[8] = 2;
	state.winMessage.vertices[10] = -2;

	state.loseMessage.textureID = loseTextureID;
	state.loseMessage.position = glm::vec3(0, 0, 0);
	state.loseMessage.isActive = false;
	state.loseMessage.vertices[0] = -1.75;
	state.loseMessage.vertices[2] = 1.75;
	state.loseMessage.vertices[4] = 1.75;
	state.loseMessage.vertices[6] = -1.75;
	state.loseMessage.vertices[8] = 1.75;
	state.loseMessage.vertices[10] = -1.75;

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void Reset() {
	state.player.position = glm::vec3(-4, -2, 0);
	state.player.acceleration = glm::vec3(0, -9.81f, 0);
	state.player.isActive = true;
	state.player.direction = RIGHT;
	state.player.killCount = 0;

	state.enemies[0].position = glm::vec3(3, -2.25f, 0);
	state.enemies[0].acceleration = glm::vec3(0, -9.81f, 0);
	state.enemies[0].aiState = PATROL;
	state.enemies[0].defaultState = PATROL;
	state.enemies[0].velocity.x = 1.0f;
	state.enemies[0].isActive = true;

	state.enemies[1].position = glm::vec3(-4, 1.75f, 0);
	state.enemies[1].acceleration = glm::vec3(0, -9.81f, 0);
	state.enemies[1].aiState = GUARD;
	state.enemies[1].defaultState = GUARD;
	state.enemies[1].direction = RIGHT;
	state.enemies[1].timer = 0.0f;
	state.enemies[1].isActive = true;

	state.enemies[2].position = glm::vec3(-1, -1.25f, 0);
	state.enemies[2].acceleration = glm::vec3(0, -9.81f, 0);
	state.enemies[2].aiState = IDLE;
	state.enemies[2].defaultState = IDLE;
	state.enemies[2].isActive = true;

	for (int i = 0; i < BULLET_COUNT; i++)
	{
		state.bullets[i].isActive = false;
	}

	state.winMessage.isActive = false;
	state.loseMessage.isActive = false;

}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        state.player.Jump();
                        break;
					case SDLK_k:
						state.player.Shoot(state.bullets, BULLET_COUNT);
						break;
					case SDLK_r:
						Reset();
						break;
                }
                break;
        }
    }
    
    state.player.velocity.x = 0;
    
    // Check for pressed/held keys below
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    if (keys[SDL_SCANCODE_A])
    {
		state.player.direction = LEFT;
        state.player.velocity.x = -3.0f;
    }
    else if (keys[SDL_SCANCODE_D])
    {
		state.player.direction = RIGHT;
        state.player.velocity.x = 3.0f;
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;

void Update() {
	if (state.player.killCount >= ENEMY_COUNT) {
		state.winMessage.isActive = true;
	}
	else if (state.player.isActive == false) {
		state.loseMessage.isActive = true;
	}

    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }
    
    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player.Update(FIXED_TIMESTEP, &state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.bullets, BULLET_COUNT);
        
		for (int i = 0; i < ENEMY_COUNT; i++)
		{
			state.enemies[i].Update(FIXED_TIMESTEP, &state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.bullets, BULLET_COUNT);
		}

		for (int i = 0; i < BULLET_COUNT; i++)
		{
			state.bullets[i].Update(FIXED_TIMESTEP, &state.player, state.platforms, PLATFORM_COUNT, state.enemies, ENEMY_COUNT, state.bullets, BULLET_COUNT);
		}

        deltaTime -= FIXED_TIMESTEP;
    }
    
    accumulator = deltaTime;
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    state.player.Render(&program);
    
    for (int i = 0; i < PLATFORM_COUNT; i++)
    {
        state.platforms[i].Render(&program);
    }
    
	for (int i = 0; i < ENEMY_COUNT; i++)
	{
		state.enemies[i].Render(&program);
	}

	for (int i = 0; i < BULLET_COUNT; i++)
	{
		state.bullets[i].Render(&program);
	}
	state.winMessage.Render(&program);
	state.loseMessage.Render(&program);

    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
