#include <gfc_vector.h>
#include <gfc_types.h>
#include <gf2d_sprite.h>

#ifndef __ALARA_PHYSICS__
#define __ALARA_PHYSICS__

#define bool Bool

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
	bool inuse;
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
	bool hit;
} Collision;

typedef struct PhysicsWorld {
	PhysicsBody *physics_bodies;
	unsigned int max_physics_bodies;
	unsigned int last_allocated_body;
} PhysicsWorld;


PhysicsWorld init_physics(unsigned int max_physics_bodies, bool allocate);
PhysicsBody *allocate_physics_body(PhysicsWorld *world);
void free_physics(PhysicsWorld *world);
void physics_step(PhysicsWorld *world, float delta);
void draw_sprites(PhysicsWorld *world);
void create_test_world(PhysicsWorld *world);

#endif