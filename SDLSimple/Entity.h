#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

enum  EntityType { PLAYER, PLATFORM, BULLET, ENEMY };
enum  AIState { IDLE, WALK, PATROL, GUARD };
enum  Direction { LEFT, RIGHT };

class Entity {
public:
	AIState defaultState;
	AIState aiState;
    EntityType entityType;
	Direction direction;
    bool isActive;
    
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
 
	float timer;
	int killCount;

    float width;
    float height;

	float vertices[12];
    
    float speed;
    
    GLuint textureID;
    
    Entity();
    
	void CheckBoundary();

    bool CheckCollision(Entity other);
    
    void CheckCollisionsX(Entity *objects, int objectCount);
    void CheckCollisionsY(Entity *objects, int objectCount);
    
	void AI(Entity* player, Entity* bullets, int bulletCount);

    void Update(float deltaTime, Entity* player, Entity *objects, int objectCount, Entity* enemies, int enemyCount, Entity* bullets, int bulletCount);
    void Render(ShaderProgram *program);
    
	void Shoot(Entity* bullets, int bulletCount);
    void Jump();
    
    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
	bool collidedLeftBoundary;
	bool collidedRightBoundary;
    
};



