#include "gfc_vector.h"
#include "gfc_types.h"
#include "gf2d_sprite.h"

#ifndef __ALARA_PHYSICS__
#define __ALARA_PHYSICS__

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

typedef struct PhysicsBody {
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
	Sprite *sprite;
} PhysicsBody;

typedef struct Collision {
	Vector2D position; // in global space
	Vector2D normal;
	float distance;
	Bool hit;
} Collision;

typedef struct PhysicsWorld {
	PhysicsBody *physics_bodies;
	unsigned int max_physics_bodies;
	unsigned int last_allocated_body;
	float gravity;
} PhysicsWorld;

PhysicsWorld init_physics(unsigned int max_physics_bodies, Bool allocate);
PhysicsBody *allocate_physics_body(PhysicsWorld *world);
int physics_get_body_id(PhysicsWorld *world, PhysicsBody *body);
PhysicsBody *get_body(PhysicsWorld *world, int id);
void apply_central_impulse(PhysicsBody *body, Vector2D impulse);
void apply_impulse(PhysicsBody *body, Vector2D impulse, Vector2D point);
void apply_central_force(PhysicsBody *body, Vector2D force, float delta);
void apply_force(PhysicsBody *body, Vector2D force, Vector2D point, float delta);
void free_physics(PhysicsWorld *world);
void physics_step(PhysicsWorld *world, float delta);
void physics_draw_sprites(PhysicsWorld *world);
void physics_debug_draw(PhysicsWorld *world);
void physics_create_test_world(PhysicsWorld *world);

#endif