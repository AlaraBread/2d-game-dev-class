#include "gfc_vector.h"
#include "gfc_types.h"
#include "gf2d_sprite.h"

#ifndef __ALARA_PHYSICS__
#define __ALARA_PHYSICS__

#define MAX_REPORTED_COLLISIONS 1

typedef struct CapsuleShape {
	float radius;
	float height;
} CapsuleShape;

typedef struct CircleShape {
	float radius;
} CircleShape;

typedef struct PlaneShape {
	Vector2D normal;
} PlaneShape;

typedef enum ShapeType {
	PLANE,
	CIRCLE,
	CAPSULE,
} ShapeType;

typedef union ColliderShape {
	PlaneShape plane;
	CircleShape circle;
	CapsuleShape capsule;
} ColliderShape;

typedef enum PhysicsType {
	RIGID,
	STATIC,
	TRIGGER,
} PhysicsType;

typedef struct PhysicsMaterial {
	float bounce;
	float friction;
} PhysicsMaterial;

typedef struct Collision {
	Vector2D position; // in global space
	Vector2D normal;
	float distance;
	Bool hit;
	int a_idx;
	int b_idx;
} Collision;

// for PhysicsBody.type
typedef enum EnemyType {
	NORMAL,
	LAZY,
	SCARED,
	AGGRESSIVE,
	SHORT,
} EnemyType;

// for PhysicsBody.tags
#define TAG_PLAYER (1<<0)
#define TAG_DEAD (1<<1)
#define TAG_ENEMY (1<<2)

typedef struct PhysicsBody_S PhysicsBody;
typedef struct PhysicsWorld_S PhysicsWorld;

typedef struct PhysicsBody_S {
	Bool inuse;
	Vector2D position;
	float rotation;
	Vector2D linear_velocity;
	float angular_velocity;
	float mass;
	float moment_of_inertia;
	Vector2D center_of_mass;
	PhysicsType physics_type;
	ShapeType shape_type;
	ColliderShape shape;
	PhysicsMaterial physics_material;
	float gravity_scale;
	Collision collisions[MAX_REPORTED_COLLISIONS];
	float timer;
	float cooldown;
	long int tags;
	long int layer;
	long int mask;
	EnemyType type;
	Sprite *sprite;
	Vector2D sprite_offset;
	Vector2D sprite_scale;
	void (*update)(PhysicsBody *body, PhysicsWorld *world, float delta);
} PhysicsBody;

typedef struct PhysicsWorld_S {
	PhysicsBody *physics_bodies;
	int player_idx;
	int floor_idx;
	unsigned int max_physics_bodies;
	unsigned int last_allocated_body;
	float gravity;
	Uint8 * keys;
	Uint32 prev_mouse_buttons;
	Uint32 mouse_buttons;
	int mouse_x, mouse_y;
	float jump_velocity;
} PhysicsWorld;

PhysicsWorld init_physics(unsigned int max_physics_bodies, Bool allocate);
void physics_copy(PhysicsWorld *from, PhysicsWorld *to);
PhysicsBody *allocate_physics_body(PhysicsWorld *world);
int physics_get_body_id(PhysicsWorld *world, PhysicsBody *body);
PhysicsBody *physics_get_body(PhysicsWorld *world, int id);
void apply_central_impulse(PhysicsBody *body, Vector2D impulse);
void apply_impulse(PhysicsBody *body, Vector2D impulse, Vector2D point);
void apply_central_force(PhysicsBody *body, Vector2D force, float delta);
void apply_force(PhysicsBody *body, Vector2D force, Vector2D point, float delta);
void free_physics(PhysicsWorld *world);
void physics_body_free(PhysicsBody *body);
void physics_step(PhysicsWorld *world, float delta);
void drop_to_floor(PhysicsBody *body, PhysicsWorld *world);
void physics_draw_sprites(PhysicsWorld *world);
void physics_debug_draw(PhysicsWorld *world);
void physics_create_test_world(PhysicsWorld *world);
void physics_clear_bodies(PhysicsWorld *world);

#endif