#include <physics.h>
#include <gf2d_draw.h>

#define randf() ((rand()&0xFFFF)/(float)0xFFFF)
#define crand() (randf()*2.0-1.0)

// https://stackoverflow.com/questions/4633177/how-to-wrap-a-float-to-the-interval-pi-pi
// answer by Tim Čas
/* wrap x -> [0,max) */
double wrapMax(double x, double max)
{
    /* integer math: `(max + x % max) % max` */
    return fmod(max + fmod(x, max), max);
}
/* wrap x -> [min,max) */
double wrapMinMax(double x, double min, double max)
{
    return min + wrapMax(x - min, max - min);
}

float vector2d_cross(Vector2D a, Vector2D b) {
	return a.x * b.y - a.y * b.x;
}

bool float_is_finite(float f) {
	u_int32_t x = *((Uint32 *)(&f));
	x <<= 1;
	x >>= 24;
	return x != 0xFF;
}

bool vector2d_is_finite(Vector2D v) {
	return float_is_finite(v.x) && float_is_finite(v.y);
}

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

void reverse_collision(Collision *col) {
	Vector2D offset;
	vector2d_scale(offset, col->normal, -2.0*col->distance);
	vector2d_add(col->position, col->position, offset);
	vector2d_scale(col->normal, col->normal, -1.0);
}

Collision circle_plane_collision(PhysicsBody *circle, PhysicsBody *plane) {
	Collision col;
	col.hit = false;
	Vector2D c;
	vector2d_sub(c, circle->position, plane->position);
	Vector2D plane_normal = vector2d_rotate(plane->shape.plane.normal, plane->rotation);
	float dist = vector2d_dot_product(c, plane_normal);
	if(dist < circle->shape.circle.radius) {
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
	Vector2D original_position = capsule->position;
	Vector2D top_circle = vector2d_rotate(vector2d(0.0, capsule->shape.capsule.height/2.0), capsule->rotation);
	vector2d_add(capsule->position, capsule->position, top_circle);
	Collision top_col = circle_plane_collision(capsule, plane);
	capsule->position = original_position;
	Vector2D bottom_circle = vector2d_rotate(vector2d(0.0, -capsule->shape.capsule.height/2.0), capsule->rotation);
	vector2d_add(capsule->position, capsule->position, bottom_circle);
	Collision bot_col = circle_plane_collision(capsule, plane);
	capsule->position = original_position;

	if(top_col.hit && bot_col.hit) {
		Vector2D average;
		vector2d_add(average, top_col.position, bot_col.position);
		vector2d_scale(average, average, 0.5);
		top_col.position = average;
		if(bot_col.distance > top_col.distance) {
			top_col.distance = bot_col.distance;
		}
		return top_col;
	}
	if(bot_col.hit) {
		return bot_col;
	}
	return top_col;
}

Collision capsule_circle_collision(PhysicsBody *capsule, PhysicsBody *circle) {
	// figure out which sextant of the capsule the circle's center is in

	Vector2D top_circle = vector2d_rotate(vector2d(0.0, capsule->shape.capsule.height/2.0), capsule->rotation);
	Vector2D bottom_circle = vector2d_rotate(vector2d(0.0, -capsule->shape.capsule.height/2.0), capsule->rotation);

	Vector2D capsule_right = vector2d_rotate(vector2d(1.0, 0.0), capsule->rotation);
	Vector2D capsule_up = vector2d_rotate(vector2d(0.0, 1.0), capsule->rotation);
	
	Vector2D c_rel;
	vector2d_sub(c_rel, circle->position, capsule->position);
	bool side_lr = false;
	if(vector2d_dot_product(capsule_right, c_rel) > 0.0) {
		side_lr = true;
	}

	Vector2D c_rel_top;
	vector2d_sub(c_rel_top, c_rel, top_circle);
	bool side_top = false;
	if(vector2d_dot_product(capsule_up, c_rel_top) > 0.0) {
		side_top = true;
	}

	Vector2D c_rel_bot;
	vector2d_sub(c_rel_bot, c_rel, bottom_circle);
	bool side_bottom = false;
	if(vector2d_dot_product(capsule_up, c_rel_bot) < 0.0) {
		side_bottom = true;
	}

	if(side_top) {
		Vector2D original_position = capsule->position;
		vector2d_add(capsule->position, capsule->position, top_circle);
		Collision col = circle_circle_collision(capsule, circle);
		capsule->position = original_position;
		return col;
	}
	if(side_bottom) {
		Vector2D original_position = capsule->position;
		vector2d_add(capsule->position, capsule->position, bottom_circle);
		Collision col = circle_circle_collision(capsule, circle);
		capsule->position = original_position;
		return col;
	}
	// we cant use the top or bottom circle, so we will use the left/right line
	PhysicsBody plane;
	vector2d_add(plane.position, capsule->position, vector2d_rotate(
			vector2d(side_lr?(capsule->shape.capsule.radius):(-capsule->shape.capsule.radius), 0.0),
			capsule->rotation));
	plane.shape.plane.normal = capsule_right;
	if(!side_lr) {
		vector2d_scale(plane.shape.plane.normal, plane.shape.plane.normal, -1.0);
	}
	plane.rotation = 0.0;
	Collision col = circle_plane_collision(circle, &plane);
	reverse_collision(&col);
	return col;
}

Vector2D velocity_at_point(PhysicsBody *body, Vector2D point) {
	Vector2D v = body->linear_velocity;
	Vector2D diff;
	vector2d_sub(diff, point, body->position);
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
	Vector2D linear_impulse;
	vector2d_scale(linear_impulse, impulse, 1.0/body->mass);
	vector2d_add(body->linear_velocity, body->linear_velocity, linear_impulse);
	
	Vector2D com;
	vector2d_add(com, body->position, vector2d_rotate(body->center_of_mass, body->rotation));
	vector2d_sub(point, point, com);
	body->angular_velocity += vector2d_cross(point, impulse)/body->moment_of_inertia;
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

Vector2D get_normal_and_tangent_mass(PhysicsBody *a, PhysicsBody *b, Collision collision) {
	// borrowed from godot physics's godot_body_pair_2d.cpp
	Vector2D rA;
	Vector2D a_com;
	vector2d_add(a_com, a->position, vector2d_rotate(a->center_of_mass, a->rotation));
	vector2d_sub(rA, collision.position, a_com);
	Vector2D rB;
	Vector2D b_com;
	vector2d_add(b_com, b->position, vector2d_rotate(b->center_of_mass, b->rotation));
	vector2d_sub(rB, collision.position, b_com);

	float rnA = vector2d_dot_product(rA, collision.normal);
	float rnB = vector2d_dot_product(rB, collision.normal);
	float kNormal = 1.0/(a->mass) + 1.0/(b->mass);
	kNormal += (1.0/a->moment_of_inertia) * (vector2d_dot_product(rA, rA) - rnA * rnA) + (1.0/b->moment_of_inertia) * (vector2d_dot_product(rB, rB) - rnB * rnB);
	float mass_normal = 1.0f / kNormal;

	Vector2D tangent = vector2d(-collision.normal.x, collision.normal.y);
	float rtA = vector2d_dot_product(rA, tangent);
	float rtB = vector2d_dot_product(rB, tangent);
	float kTangent = 1.0/(a->mass) + 1.0/(b->mass);
	kTangent += (1.0/a->moment_of_inertia) * (vector2d_dot_product(rA, rA) - rtA * rtA) + (1.0/b->moment_of_inertia) * (vector2d_dot_product(rB, rB) - rtB * rtB);
	float mass_tangent = 1.0f / kTangent;

	return vector2d(mass_normal, mass_tangent);
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
		case CAPSULE:
			switch (b->shape_type) {
				case PLANE:
					col = capsule_plane_collision(a, b);
					break;
				case CIRCLE:
					col = capsule_circle_collision(a, b);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	if(!col.hit) {
		return;
	}
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

	Vector2D nt_mass = get_normal_and_tangent_mass(a, b, col);
	float normal_mass = nt_mass.x;
	float tangent_mass = nt_mass.y;

	Vector2D tangent = vector2d_rotate(col.normal, M_PI/2.0);
	Vector2D a_v = velocity_at_point(a, col.position);
	Vector2D b_v = velocity_at_point(b, col.position);
	Vector2D dv;
	vector2d_sub(dv, a_v, b_v);
	float dvn = vector2d_dot_product(col.normal, dv);
	float dvt = vector2d_dot_product(tangent, dv);

	float bounce = SDL_clamp(a->physics_material.bounce*b->physics_material.bounce, 0.0, 1.0);

	Vector2D impulse;

	float normal_impulse = -dvn*bounce*normal_mass-dvn*normal_mass;
	vector2d_scale(impulse, col.normal, normal_impulse);

	float friction = SDL_clamp(a->physics_material.friction*b->physics_material.friction, 0.0, 1.0);
	float tangent_impulse = SDL_clamp(-dvt*tangent_mass*0.1, -friction*SDL_fabsf(normal_impulse), friction*SDL_fabsf(normal_impulse));
	Vector2D tangent_impulse_v;
	vector2d_scale(tangent_impulse_v, tangent, tangent_impulse);

	vector2d_add(impulse, impulse, tangent_impulse_v);

	apply_impulse(a, impulse, col.position);
	vector2d_scale(impulse, impulse, -1.0);
	apply_impulse(b, impulse, col.position);
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
	vector2d_scale(linear_damping, body->linear_velocity, delta*0.1);
	vector2d_sub(body->linear_velocity, body->linear_velocity, linear_damping);

	float angular_damping = body->angular_velocity*delta*0.1;
	body->angular_velocity -= angular_damping;
}

void apply_world_bounds(PhysicsBody *body) {
	bool reset = false;
	if(!vector2d_is_finite(body->position)) {
		reset = true;
	}
	if(!vector2d_is_finite(body->linear_velocity)) {
		reset = true;
	}
	if(!float_is_finite(body->angular_velocity)) {
		reset = true;
	}
	if(!float_is_finite(body->rotation)) {
		reset = true;
	}
	if(body->position.x > 3000 || body->position.x < -3000 ||
			body->position.y < -3000 || body->position.y > 3000) {
		reset = true;
	}
	if(reset) {
		body->position.x = 600.0;
		body->position.y = 0.0;
		body->linear_velocity = vector2d(0.0, 0.0);
		body->angular_velocity = 0.0;
		body->rotation = 0.0;
	}
}

void integrate(PhysicsBody *body, float delta) {
	Vector2D linear_move;
	vector2d_scale(linear_move, body->linear_velocity, delta);
	vector2d_add(body->position, body->position, linear_move);
	body->rotation = fmodf(body->rotation+body->angular_velocity*delta, 2.0*M_PI);
	Vector2D com;
	vector2d_add(com, body->position, vector2d_rotate(body->center_of_mass, body->rotation));
	body->position = vector2d_rotate_around_center(body->position, body->angular_velocity*delta, com);
}

void apply_righting(PhysicsBody *body, float delta) {
	if(body->shape_type != CAPSULE) {
		return;
	}
	body->angular_velocity -= delta*SDL_clamp(30.0*wrapMinMax(body->rotation, -M_PI, M_PI), -5.0, 5.0);
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
		apply_righting(body, delta);
		apply_damping(body, delta);
		apply_gravity(body, delta);
		integrate(body, delta);
		for(int j = 0; j < world->max_physics_bodies; j++) {
			if(!world->physics_bodies[j].inuse || j == i) {
				continue;
			}
			solve_collision(body, &world->physics_bodies[j], delta);
		}
	}
}

void draw_sprites(PhysicsWorld *world) {
	for(int i = 0; i < world->max_physics_bodies; i++) {
		PhysicsBody *body = &world->physics_bodies[i];
		if(!body->inuse) {
			continue;
		}
		if(vector2d_magnitude_between(body->position, vector2d(500, 500)) > 1000.0) {
			continue;
		}
		switch (body->shape_type) {
			case CIRCLE:
			{
				Color color = body->shape.circle.radius == 50.0?gfc_color(1.0, 0.0, 1.0, 1.0):gfc_color(1.0, 1.0, 1.0, 1.0);
				gf2d_draw_circle(body->position, (int)body->shape.circle.radius, color);

				Vector2D start = vector2d_rotate(vector2d(body->shape.circle.radius, 0.0), body->rotation);
				Vector2D end;
				vector2d_scale(end, start, -1.0);

				vector2d_add(start, start, body->position);
				vector2d_add(end, end, body->position);

				gf2d_draw_line(start, end, color);
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
			case CAPSULE:
			{
				Color color = gfc_color(1.0, 1.0, 1.0, 1.0);
				Vector2D top_circle = vector2d_rotate(vector2d(0.0, body->shape.capsule.height/2.0), body->rotation);
				vector2d_add(top_circle, body->position, top_circle);
				Vector2D bottom_circle = vector2d_rotate(vector2d(0.0, -body->shape.capsule.height/2.0), body->rotation);
				vector2d_add(bottom_circle, body->position, bottom_circle);
				gf2d_draw_circle(top_circle, (int)body->shape.capsule.radius, color);
				gf2d_draw_circle(bottom_circle, (int)body->shape.capsule.radius, color);

				Vector2D left_start;
				vector2d_add(left_start, top_circle, vector2d_rotate(vector2d(0.0, body->shape.capsule.radius), M_PI/2+body->rotation));
				Vector2D left_end;
				vector2d_add(left_end, bottom_circle, vector2d_rotate(vector2d(0.0, body->shape.capsule.radius), M_PI/2+body->rotation));
				gf2d_draw_line(left_start, left_end, color);

				Vector2D right_start;
				vector2d_add(right_start, top_circle, vector2d_rotate(vector2d(0.0, -body->shape.capsule.radius), M_PI/2+body->rotation));
				Vector2D right_end;
				vector2d_add(right_end, bottom_circle, vector2d_rotate(vector2d(0.0, -body->shape.capsule.radius), M_PI/2+body->rotation));
				gf2d_draw_line(right_start, right_end, color);

				Vector2D com;
				vector2d_add(com, body->position, vector2d_rotate(body->center_of_mass, body->rotation));
				gf2d_draw_circle(com, 20, gfc_color(1.0, 1.0, 0.0, 1.0));
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
	float f = 0.7;
	float b = 0.8;

	PhysicsBody *floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(500.0, 720.0-50.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(0.0, -1.0);
	vector2d_normalize(&floor->shape.plane.normal);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = INFINITY;
	floor->moment_of_inertia = INFINITY;
	floor->physics_material.friction = b;
	floor->physics_material.bounce = f;

	floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(500.0, 50.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(0.0, 1.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = INFINITY;
	floor->moment_of_inertia = INFINITY;
	floor->physics_material.friction = f;
	floor->physics_material.bounce = b;

	floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(50.0, 700.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(1.0, 0.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = INFINITY;
	floor->moment_of_inertia = INFINITY;
	floor->physics_material.friction = f;
	floor->physics_material.bounce = b;

	floor = allocate_physics_body(world);
	floor->physics_type = STATIC;
	floor->position = vector2d(1200.0-50.0, 700.0);
	floor->shape_type = PLANE;
	floor->shape.plane.normal = vector2d(-1.0, 0.0);
	floor->linear_velocity = vector2d(0.0, 0.0);
	floor->mass = INFINITY;
	floor->moment_of_inertia = INFINITY;
	floor->physics_material.friction = f;
	floor->physics_material.bounce = b;

	PhysicsBody *ball;

	for(int i = 0; i < 4; i++) {
		ball = allocate_physics_body(world);
		ball->physics_type = RIGID;
		ball->position = vector2d(crand()*200.0+300.0, crand()*200.0+300.0);
		ball->shape_type = CIRCLE;
		ball->shape.circle.radius = 40.0;
		ball->mass = 100.0;
		ball->moment_of_inertia = 0.5*ball->mass*ball->shape.circle.radius*ball->shape.circle.radius;
		ball->physics_material.bounce = 1.0;
		ball->physics_material.friction = 1.0;
	}
}