#include <physics.h>
#include <gf2d_draw.h>

#define randf() ((rand()&0xFFFF)/(float)0xFFFF)
#define crand() (randf()*2.0-1.0)

Vector2D vector2d_project(Vector2D v, Vector2D normal) {
	Vector2D p;
	float d = vector2d_dot_product(v, normal);
	vector2d_scale(p, normal, d);
	return p;
}

Vector2D vector2d_slide(Vector2D v, Vector2D normal) {
	Vector2D tangent = vector2d_rotate(normal, M_PI/2.0);
	return vector2d_project(v, tangent);
}

float vector2d_squared_distance_between(Vector2D a, Vector2D b) {
	Vector2D c;
	vector2d_sub(c, a, b);
	return vector2d_magnitude_squared(c);
}

Collision circle_plane_collision(PhysicsBody *circle, PhysicsBody *plane) {
	Collision col;
	col.hit = false;
	Vector2D c;
	vector2d_sub(c, circle->position, plane->position);
	Vector2D plane_normal = vector2d_rotate(plane->shape.plane.normal, plane->rotation);
	c = vector2d_project(c, plane_normal);
	float dist = vector2d_magnitude(c);
	if(dist < circle->shape.circle.radius || vector2d_dot_product(c, plane_normal) < 0.0) {
		col.hit = true;
		col.normal = plane_normal;
		col.distance = circle->shape.circle.radius-dist;
		Vector2D pos;
		vector2d_scale(pos, plane_normal, -circle->shape.circle.radius);
		vector2d_add(pos, pos, circle->position);
		col.position = pos;
	}
	return col;
}

Collision circle_circle_collision(PhysicsBody *a, PhysicsBody *b) {
	Collision col;
	col.hit = false;
	Vector2D diff;
	vector2d_sub(diff, a->position, b->position);
	float dist = vector2d_magnitude(diff);
	float radius = a->shape.circle.radius+b->shape.circle.radius;
	if(dist < radius) {
		col.hit = true;
		col.distance = radius-dist;
		col.normal = diff;
		vector2d_normalize(&col.normal);
		vector2d_scale(col.position, col.normal, -a->shape.circle.radius+col.distance/2.0);
		vector2d_add(col.position, col.position, a->position);
	}
	return col;
}

Collision capsule_plane_collision(PhysicsBody *capsule, PhysicsBody *plane) {
	// TODO
	Collision col;
	col.hit = false;
	return col;
}

Vector2D velocity_at_point(PhysicsBody *body, Vector2D point) {
	Vector2D v = body->linear_velocity;
	Vector2D diff;
	vector2d_sub(diff, point, body->position);
	//float dist = vector2d_magnitude(diff);
	Vector2D angular_component = vector2d_rotate(diff, M_PI/2.0);
	vector2d_scale(angular_component, angular_component, body->angular_velocity);
	vector2d_add(v, v, angular_component);
	return v;
}

void apply_central_impulse(PhysicsBody *body, Vector2D impulse) {
	if(body->physics_type != RIGID) {
		return;
	}
	vector2d_scale(impulse, impulse, 1.0/body->mass);
	vector2d_add(body->linear_velocity, body->linear_velocity, impulse);
}

float sign(float val) {
    return (0.0 < val) - (val < 0.0);
}

void apply_impulse(PhysicsBody *body, Vector2D impulse, Vector2D point) {
	if(body->physics_type != RIGID) {
		return;
	}
	Vector2D diff;
	vector2d_sub(diff, point, body->position);
	Vector2D normal = diff;
	vector2d_normalize(&normal);
	Vector2D normal_component = vector2d_project(impulse, normal);
	Vector2D tangent_component;
	vector2d_sub(tangent_component, impulse, normal_component);

	Vector2D linear_tangent;
	vector2d_scale(linear_tangent, tangent_component, body->moment_of_inertia/(body->mass+body->moment_of_inertia));
	Vector2D angular_tangent;
	vector2d_sub(angular_tangent, tangent_component, linear_tangent);

	Vector2D linear_component;
	vector2d_add(linear_component, linear_tangent, normal_component);
	apply_central_impulse(body, linear_component);

	float dist = vector2d_magnitude(diff);
	if(dist != 0.0) {
		body->angular_velocity += sign(vector2d_dot_product(
				vector2d_rotate(normal, M_PI/2.0), angular_tangent))*
				vector2d_magnitude(angular_tangent)/dist;
	}
}

void apply_central_force(PhysicsBody *body, Vector2D force, float delta) {
	Vector2D impulse;
	vector2d_scale(impulse, force, delta);
	apply_central_impulse(body, impulse);
}

void apply_force(PhysicsBody *body, Vector2D force, Vector2D point, float delta) {
	Vector2D impulse;
	vector2d_scale(impulse, force, delta);
	apply_impulse(body, impulse, point);
}

void solve_collision(PhysicsBody *a, PhysicsBody *b, float delta) {
	if(a->physics_type != RIGID) {
		return;
	}
	Collision col;
	col.hit = false;
	switch (a->shape_type) {
		case CIRCLE:
			switch (b->shape_type) {
				case CIRCLE:
					col = circle_circle_collision(a, b);
					break;
				case PLANE:
					col = circle_plane_collision(a, b);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	if(col.hit) {
		if(a->physics_type != RIGID && b->physics_type != RIGID) {
			return;
		}
		if(a->physics_type == TRIGGER || b->physics_type == TRIGGER) {
			return;
		}
		Vector2D recovery_vector;
		vector2d_scale(recovery_vector, col.normal, col.distance/((a->physics_type == RIGID) + (b->physics_type == RIGID)));
		if(a->physics_type == RIGID) {
			vector2d_add(a->position, a->position, recovery_vector);
		}
		if(b->physics_type == RIGID) {
			vector2d_sub(b->position, b->position, recovery_vector);
		}
		Vector2D a_v = velocity_at_point(a, col.position);
		Vector2D b_v = velocity_at_point(b, col.position);

		Vector2D delta_v;
		vector2d_sub(delta_v, b_v, a_v);

		Vector2D a_impulse;
		Vector2D b_impulse;
		float d = vector2d_dot_product(delta_v, col.normal);
		vector2d_scale(a_impulse, col.normal, a->physics_material.bounce*d*(b->mass/a->mass));
		vector2d_scale(b_impulse, col.normal, -b->physics_material.bounce*d*(a->mass/b->mass));

		Vector2D friction = vector2d_slide(delta_v, col.normal);

		float f = 1.0;//a->physics_material.friction*b->physics_material.friction;

		vector2d_scale(friction, friction, f);

		vector2d_add(a_impulse, a_impulse, friction);
		vector2d_scale(friction, friction, -1.0);
		vector2d_add(b_impulse, b_impulse, friction);

		if(a->physics_type == RIGID) {
			a->linear_velocity = vector2d_slide(a->linear_velocity, col.normal);
		}
		if(b->physics_type == RIGID) {
			b->linear_velocity = vector2d_slide(b->linear_velocity, col.normal);
		}

		apply_impulse(a, a_impulse, col.position);
		apply_impulse(b, b_impulse, col.position);
	}
}

void apply_gravity(PhysicsBody *body, float delta) {
	if(body->physics_type != RIGID) {
		return;
	}
	body->linear_velocity.y += 400.0*delta;
}

void apply_damping(PhysicsBody *body, float delta) {
	if(body->physics_type != RIGID) {
		return;
	}
	// linear_velocity -= linear_velocity*delta*damping
	Vector2D linear_damping;
	vector2d_scale(linear_damping, body->linear_velocity, delta*1.0);
	vector2d_sub(body->linear_velocity, body->linear_velocity, linear_damping);

	float angular_damping = body->angular_velocity*delta*0.0;
	body->angular_velocity -= angular_damping;
}

void apply_world_bounds(PhysicsBody *body) {
	if(body->position.x > 3000 || body->position.x < -3000 ||
			body->position.y < -3000 || body->position.y > 3000) {
		body->position.x = crand()*300.0+600.0;
		body->position.y = crand()*300.0+600.0;
		body->linear_velocity = vector2d(0.0, 0.0);
		body->angular_velocity = 0.0;
	}
}

void integrate(PhysicsBody *body, float delta) {
	Vector2D linear_move;
	vector2d_scale(linear_move, body->linear_velocity, delta);
	vector2d_add(body->position, body->position, linear_move);
	body->rotation = fmodf(body->rotation+body->angular_velocity*delta, 2.0*M_PI);
}

PhysicsWorld init_physics(unsigned int max_physics_bodies) {
	PhysicsWorld world;
	world.max_physics_bodies = max_physics_bodies;
	world.physics_bodies = calloc(sizeof(PhysicsBody), max_physics_bodies);
	world.last_allocated_body = max_physics_bodies-1;
	return world;
}

void free_physics(PhysicsWorld *world) {
	free(world->physics_bodies);
}

PhysicsBody *allocate_physics_body(PhysicsWorld *world) {
	for(int i = (world->last_allocated_body+1)%world->max_physics_bodies;
			i != world->last_allocated_body;
			i = (i+1)%world->max_physics_bodies) {
		if(!world->physics_bodies[i].inuse) {
			world->last_allocated_body = i;
			PhysicsBody *body = &world->physics_bodies[i];
			memset(body, 0, sizeof(PhysicsBody));
			body->inuse = true;
			return body;
		}
	}
	return NULL;
}

void physics_step(PhysicsWorld *world, float delta) {
	for(int i = 0; i < world->max_physics_bodies; i++) {
		PhysicsBody *body = &world->physics_bodies[i];
		if(!body->inuse) {
			continue;
		}
		apply_world_bounds(body);
		for(int j = 0; j < world->max_physics_bodies; j++) {
			if(!world->physics_bodies[j].inuse || j == i) {
				continue;
			}
			solve_collision(body, &world->physics_bodies[j], delta);
		}
		apply_damping(body, delta);
		apply_gravity(body, delta);
		integrate(body, delta);
	}
}

void draw_sprites(PhysicsWorld *world) {
	for(int i = 0; i < world->max_physics_bodies; i++) {
		PhysicsBody *body = &world->physics_bodies[i];
		if(!body->inuse) {
			continue;
		}
		switch (body->shape_type) {
			case CIRCLE:
			{
				gf2d_draw_circle(body->position, (int)body->shape.circle.radius, gfc_color(1.0, 0.0, 1.0, 1.0));

				Vector2D start = vector2d_rotate(vector2d(body->shape.circle.radius, 0.0), body->rotation);
				Vector2D end;
				vector2d_scale(end, start, -1.0);

				vector2d_add(start, start, body->position);
				vector2d_add(end, end, body->position);

				gf2d_draw_line(start, end, gfc_color(1.0, 0.0, 1.0, 1.0));
				break;
			}
			case PLANE:
			{
				Vector2D tangent = vector2d_rotate(body->shape.plane.normal, M_PI/2.0);
				vector2d_scale(tangent, tangent, 1000.0);
				Vector2D start;
				Vector2D end;
				vector2d_add(start, body->position, tangent);
				vector2d_sub(end, body->position, tangent);
				
				gf2d_draw_line(start, end, gfc_color(0.0, 1.0, 1.0, 1.0));
				break;
			}
			default:
				break;
		}
		if(!body->sprite) {
			continue;
		}
		float rotation = body->rotation*(180.0/M_PI);
		gf2d_sprite_draw(
				body->sprite,
				body->position,
				NULL,
				NULL,
				&rotation,
				NULL,
				NULL,
				0);
	}
}

void create_test_world(PhysicsWorld *world) {
	PhysicsBody *floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(500.0, 700.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(0.0, -1.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = 1.0;
	floor->moment_of_inertia = 1.0;
	floor->physics_material.friction = 1.0;

	floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(500.0, 100.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(0.0, 1.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = 1.0;
	floor->moment_of_inertia = 1.0;
	floor->physics_material.friction = 1.0;

	floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(100.0, 700.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(1.0, 0.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = 1.0;
	floor->moment_of_inertia = 1.0;
	floor->physics_material.friction = 1.0;

	floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(1100.0, 700.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(-1.0, 0.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = 1.0;
	floor->moment_of_inertia = 1.0;
	floor->physics_material.friction = 1.0;

	PhysicsBody *ball;

	for(int i = 0; i < 10; i++) {
		ball = allocate_physics_body(world);
		ball->physics_type = RIGID;
		ball->position = vector2d(crand()*200.0+300.0, crand()*200.0+300.0);
		ball->shape_type = CIRCLE;
		ball->shape.circle.radius = 10.0;
		ball->linear_velocity = vector2d(crand()*1.0, crand()*1.0);
		ball->mass = 1.0;
		ball->moment_of_inertia = 1.0;
		ball->physics_material.bounce = 0.6;
		floor->physics_material.friction = 1.0;
	}
}