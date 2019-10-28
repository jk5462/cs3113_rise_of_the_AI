#include "Entity.h"

Entity::Entity()
{
    entityType = PLATFORM;
    isActive = true;
    position = glm::vec3(0);
    speed = 0;
    width = 1;
    height = 1;

	vertices[0] = -0.5;
	vertices[1] = -0.5;

	vertices[2] = 0.5;
	vertices[3] = -0.5;

	vertices[4] = 0.5;
	vertices[5] = 0.5;

	vertices[6] = -0.5;
	vertices[7] = -0.5;

	vertices[8] = 0.5;
	vertices[9] = 0.5;

	vertices[10] = -0.5;
	vertices[11] = 0.5;
}

void Entity::CheckBoundary()
{
	if ((position.x + (width / 2.0f)) > 5.0f) {
		position.x = 5.0f - (width / 2.0f);
		velocity.x = 0;
		collidedRightBoundary = true;
	}
	else if ((position.x - (width / 2.0f)) < -5.0f) {
		position.x = -5.0f + (width / 2.0f);
		velocity.x = 0;
		collidedLeftBoundary = true;
	}
}

bool Entity::CheckCollision(Entity other)
{
    if (isActive == false || other.isActive == false) return false;
    
    float xdist = fabs(position.x - other.position.x) - ((width + other.width) / 2.0f);
    float ydist = fabs(position.y - other.position.y) - ((height + other.height) / 2.0f);

    if (xdist < 0 && ydist < 0)
    {
        return true;
    }
    
    return false;
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        
        if (CheckCollision(object))
        {
            float ydist = fabs(position.y - object.position.y);
            float penetrationY = fabs(ydist - (height / 2) - (object.height / 2));
            if (velocity.y > 0) {
                position.y -= penetrationY;
                velocity.y = 0;
                collidedTop = true;
            }
            else if (velocity.y < 0) {
                position.y += penetrationY;
                velocity.y = 0;
                collidedBottom = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
    for (int i = 0; i < objectCount; i++)
    {
        Entity object = objects[i];
        
        if (CheckCollision(object))
        {
            float xdist = fabs(position.x - object.position.x);
            float penetrationX = fabs(xdist - (width / 2) - (object.width / 2));
            if (velocity.x > 0) {
                position.x -= penetrationX;
                velocity.x = 0;
				collidedRight = true;
            }
            else if (velocity.x < 0) {
                position.x += penetrationX;
                velocity.x = 0;
				collidedLeft = true;
            }
        }
    }
}


void Entity::Jump()
{
    if (collidedBottom)
    {
        velocity.y = 5.0f;
    }
}

void Entity::Shoot(Entity* bullets, int bulletCount)
{
	if (isActive != true) return;
	for (int i = 0; i < bulletCount; i++) {
		if (bullets[i].isActive == false) {
			bullets[i].isActive = true;
			bullets[i].position = position;
			if (direction == LEFT) {
				bullets[i].position.x -= 0.5;
				bullets[i].velocity.x = -10.0f;
			}
			else {
				bullets[i].position.x += 0.5;
				bullets[i].velocity.x = 10.0f;
			}
			break;
		}
	}
}


void Entity::AI(Entity* player, Entity* bullets, int bulletCount) {
	if (glm::distance(position, player->position) < 2.5f && aiState != GUARD) {
		aiState = WALK;
	}
	else {
		aiState = defaultState;
	}
	switch (aiState) {
	case IDLE:
		velocity = glm::vec3(0, 0, 0);
		break;

	case WALK:
		if (glm::distance(position, player->position) < 0.5f) {
			player->isActive = false;
		}
		if (player->position.x > position.x)
		{
			velocity.x = 1.0f;
		}
		else
		{
			velocity.x = -1.0f;
		}
		break;

	case PATROL:
		if (collidedLeftBoundary == true || collidedLeft == true) {
			if (timer > 1.0f) {
				velocity.x = 1.0f;
				timer = 0.0f;
			}
			else {
				timer += 0.02;
				velocity.x = -1.0f;
			}
		}
		else if (collidedRightBoundary == true || collidedRight == true) {
			if (timer > 1.0f) {
				velocity.x = -1.0f;
				timer = 0.0f;
			}
			else {
				timer += 0.02;
				velocity.x = 1.0f;
			}

		}
		break;

	case GUARD:
		if (timer > 1.5f) {
			Shoot(bullets, bulletCount);
			timer = 0.0f;
		}
		else {
			timer += 0.02;
		}
	}
}


void Entity::Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Entity* enemies, int enemyCount, Entity* bullets, int bulletCount)
{
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
	collidedLeftBoundary = false;
	collidedRightBoundary = false;

	if (isActive) {
		velocity += acceleration * deltaTime;

		if (entityType == BULLET) {
			position.x += velocity.x * deltaTime;
			if (position.x > 6 || position.x < -6) {
				isActive = false;
			}
			else {
				if (CheckCollision(*player)){
					player->isActive = false;
				}

				for (int i = 0; i < enemyCount; i++)
				{
					if (CheckCollision(enemies[i])) {
						enemies[i].isActive = false;
						isActive = false;
						player->killCount += 1;
					}
				}

				for (int i = 0; i < objectCount; i++)
				{
					if (CheckCollision(objects[i])) {
						isActive = false;
					}
				}
			}
		}
		else {

			position.y += velocity.y * deltaTime;
			CheckCollisionsY(objects, objectCount);

			position.x += velocity.x * deltaTime;
			CheckCollisionsX(objects, objectCount);

			CheckBoundary();

			if (entityType == ENEMY) {
				AI(player, bullets, bulletCount);
			}


		}
	}

}



void Entity::Render(ShaderProgram *program) {
	if (isActive) {
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		modelMatrix = glm::translate(modelMatrix, position);
		program->SetModelMatrix(modelMatrix);

		//float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

		glBindTexture(GL_TEXTURE_2D, textureID);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);

		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}
}

