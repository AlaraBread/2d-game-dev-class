#include "physics.h"

PhysicsBody *init_wall(PhysicsWorld *world, Vector2D position, Vector2D normal, float friction, float bounce) {
	PhysicsBody *wall = allocate_physics_body(world);
	wall->physics_type = STATIC;
	wall->position = position;
	wall->shape_type = PLANE;
	wall->shape.plane.normal = normal;
	vector2d_normalize(&wall->shape.plane.normal);
	wall->linear_velocity = vector2d(0.0, 0.0);
	wall->mass = INFINITY;
	wall->moment_of_inertia = INFINITY;
	wall->physics_material.friction = friction;
	wall->physics_material.bounce = bounce;
}

PhysicsBody *init_floor(PhysicsWorld *world, Vector2D position, Vector2D normal, float friction, float bounce) {
	PhysicsBody *floor = init_wall(world, position, normal, friction, bounce);
	world->floor_idx = physics_get_body_id(world, floor);
	return floor;
}