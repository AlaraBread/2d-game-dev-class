#include "gf2d_draw.h"
#include "physics.h"
#include "mosher.h"
#include "wall.h"

float vector2d_cross(Vector2D a, Vector2D b) {
	return a.x * b.y - a.y * b.x;
}

Bool float_is_finite(float f) {
	u_int32_t x = *((Uint32 *)(&f));
	x <<= 1;
	x >>= 24;
	return x != 0xFF;
}

Bool vector2d_is_finite(Vector2D v) {
	return float_is_finite(v.x) && float_is_finite(v.y);
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
	Bool side_lr = false;
	if(vector2d_dot_product(capsule_right, c_rel) > 0.0) {
		side_lr = true;
	}

	Vector2D c_rel_top;
	vector2d_sub(c_rel_top, c_rel, top_circle);
	Bool side_top = false;
	if(vector2d_dot_product(capsule_up, c_rel_top) > 0.0) {
		side_top = true;
	}

	Vector2D c_rel_bot;
	vector2d_sub(c_rel_bot, c_rel, bottom_circle);
	Bool side_bottom = false;
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

// taken from Real Time Collision Detection by Christer Ericson

// Computes closest points C1 and C2 of S1(s)=P1+s*(Q1-P1) and
// S2(t)=P2+t*(Q2-P2), returning s and t. Function result is squared
// distance between between S1(s) and S2(t)
float ClosestPtSegmentSegment(Vector2D p1, Vector2D q1, Vector2D p2, Vector2D q2,
float *s, float *t, Vector2D *c1, Vector2D *c2)
{
	Vector2D d1; // Direction vector of segment S1
	vector2d_sub(d1, q1, p1);
	Vector2D d2; // Direction vector of segment S2
	vector2d_sub(d2, q2, p2);
	Vector2D r;
	vector2d_sub(r, p1, p2);
	float a = vector2d_dot_product(d1, d1); // Squared length of segment S1, always nonnegative
	float e = vector2d_dot_product(d2, d2); // Squared length of segment S2, always nonnegative
	float f = vector2d_dot_product(d2, r);
	// Check if either or both segments degenerate into points
	if (a <= GFC_EPSILON && e <= GFC_EPSILON) {
		// Both segments degenerate into points
		*s = *t = 0.0f;
		vector2d_copy((*c1), p1);
		vector2d_copy((*c2), p2);
		Vector2D tmp;
		vector2d_sub(tmp, (*c1), (*c2));
		return vector2d_dot_product(tmp, tmp);
	}
	if (a <= GFC_EPSILON) {
		// First segment degenerates into a point
		*s = 0.0f;
		*t = f / e; // s = 0 => t = (b*s + f) / e = f / e
		*t = SDL_clamp(*t, 0.0f, 1.0f);
	} else {
		float c = vector2d_dot_product(d1, r);
		if (e <= GFC_EPSILON) {
			// Second segment degenerates into a point
			*t = 0.0f;
			*s = SDL_clamp(-c / a, 0.0f, 1.0f); // t = 0 => s = (b*t - c) / a = -c / a
		} else {
			// The general nondegenerate case starts here
			float b = vector2d_dot_product(d1, d2);
			float denom = a*e-b*b; // Always nonnegative
			// If segments not parallel, compute closest point on L1 to L2 and
			// clamp to segment S1. Else pick arbitrary s (here 0)
			if (denom != 0.0f) {
				*s = SDL_clamp((b*f - c*e) / denom, 0.0f, 1.0f);
			} else *s = 0.0f;
			// Compute point on L2 closest to S1(s) using
			// t = Dot((P1 + D1*s) - P2,D2) / Dot(D2,D2) = (b*s + f) / e
			*t = (b*(*s) + f) / e;
			// If t in [0,1] done. Else clamp t, recompute s for the new value
			// of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
			// and clamp s to [0, 1]
			if (*t < 0.0f) {
				*t = 0.0f;
				*s = SDL_clamp(-c / a, 0.0f, 1.0f);
			} else if (*t > 1.0f) {
				*t = 1.0f;
				*s = SDL_clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}
	vector2d_scale((*c1), d1, (*s));
	vector2d_add((*c1), (*c1), p1);
	vector2d_scale((*c2), d2, (*t));
	vector2d_add((*c2), (*c2), p2);
	Vector2D tmp;
	vector2d_sub(tmp, (*c1), (*c2));
	return vector2d_dot_product(tmp, tmp);
}

Collision capsule_capsule_collision(PhysicsBody *a, PhysicsBody *b) {
	Vector2D a_p1, a_p2, b_p1, b_p2;
	vector2d_add(a_p1, a->position, vector2d_rotate(vector2d(0.0, a->shape.capsule.height/2.0), a->rotation));
	vector2d_add(a_p2, a->position, vector2d_rotate(vector2d(0.0, -a->shape.capsule.height/2.0), a->rotation));
	vector2d_add(b_p1, b->position, vector2d_rotate(vector2d(0.0, b->shape.capsule.height/2.0), b->rotation));
	vector2d_add(b_p2, b->position, vector2d_rotate(vector2d(0.0, -b->shape.capsule.height/2.0), b->rotation));
	
	Vector2D a_c, b_c;
	float a_t, b_t;
	Collision col;
	col.distance = ClosestPtSegmentSegment(a_p1, a_p2, b_p1, b_p2, &a_t, &b_t, &a_c, &b_c);
	col.distance = sqrtf(col.distance) - (a->shape.capsule.radius+b->shape.capsule.radius);
	
	vector2d_sub(col.normal, b_c, a_c);
	vector2d_normalize(&col.normal);

	vector2d_scale(col.position, col.normal, -a->shape.circle.radius+col.distance/2.0);
	vector2d_add(col.position, col.position, a->position);
	col.hit = col.distance < 0.0;
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

void solve_collision(PhysicsWorld *world, PhysicsBody *a, PhysicsBody *b, float delta, Bool simple) {
	if(a->physics_type != RIGID) {
		return;
	}
	if(!(a->mask & b->layer)) {
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
				case CAPSULE:
					col = capsule_capsule_collision(a, b);
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
	if(!simple) {
		for(int i = 0; i < MAX_REPORTED_COLLISIONS; i++) {
			if(!a->collisions[i].hit) {
				col.a_idx = physics_get_body_id(world, a);
				col.b_idx = physics_get_body_id(world, b);
				a->collisions[i] = col;
				break;
			}
		}
		for(int i = 0; i < MAX_REPORTED_COLLISIONS; i++) {
			if(!b->collisions[i].hit) {
				col.a_idx = physics_get_body_id(world, b);
				col.b_idx = physics_get_body_id(world, a);
				b->collisions[i] = col;
				reverse_collision(&b->collisions[i]);
				break;
			}
		}
	}
	Vector2D recovery_vector;
	vector2d_scale(recovery_vector, col.normal, col.distance/((a->physics_type == RIGID) + (b->physics_type == RIGID)));
	if(a->physics_type == RIGID) {
		vector2d_add(a->position, a->position, recovery_vector);
	}
	if(b->physics_type == RIGID) {
		vector2d_sub(b->position, b->position, recovery_vector);
	}

	if(simple) {
		return;
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

void apply_gravity(PhysicsWorld *world, PhysicsBody *body, float delta) {
	if(body->physics_type != RIGID) {
		return;
	}
	body->linear_velocity.y += world->gravity*body->gravity_scale*delta;
}

void apply_damping(PhysicsBody *body, float delta) {
	if(body->physics_type != RIGID) {
		return;
	}
	// linear_velocity -= linear_velocity*delta*damping
	Vector2D linear_damping;
	vector2d_scale(linear_damping, body->linear_velocity, delta*0.1);
	vector2d_sub(body->linear_velocity, body->linear_velocity, linear_damping);

	float angular_damping = body->angular_velocity*delta*1.0;
	body->angular_velocity -= angular_damping;
}

void apply_world_bounds(PhysicsBody *body) {
	Bool reset = false;
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
		body->position.x = gfc_random()*1200.0;
		body->position.y = -2500.0;
		body->linear_velocity = vector2d(0.0, 0.0);
		body->angular_velocity = 0.0;
		body->rotation = 0.0;
	}
}

void drop_to_floor(PhysicsBody *body, PhysicsWorld *world) {
	if(world->floor_idx < 0) {
		return;
	}
	PhysicsBody *floor = physics_get_body(world, world->floor_idx);
	if(!floor->inuse || floor->shape_type != PLANE) {
		return;
	}
	body->position.y = 1000.0;
	solve_collision(world, body, floor, 0.0, true);
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

PhysicsWorld init_physics(unsigned int max_physics_bodies, Bool allocate) {
	PhysicsWorld world;
	world.max_physics_bodies = max_physics_bodies;
	if(allocate) {
		world.physics_bodies = calloc(sizeof(PhysicsBody), max_physics_bodies);
	} else {
		world.physics_bodies = NULL;
	}
	world.last_allocated_body = max_physics_bodies-1;
	world.floor_idx = -1;
	world.player_idx = -1;
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
			body->gravity_scale = 1.0;
			return body;
		}
	}
	return NULL;
}

void physics_clear_bodies(PhysicsWorld *world) {
	memset(world->physics_bodies, 0, sizeof(PhysicsBody) * world->max_physics_bodies);
}

int physics_get_body_id(PhysicsWorld *world, PhysicsBody *body) {
	return body-world->physics_bodies;
}

PhysicsBody *physics_get_body(PhysicsWorld *world, int id) {
	return &world->physics_bodies[id];
}

void physics_step(PhysicsWorld *world, float delta) {
	for(int i = 0; i < world->max_physics_bodies; i++) {
		PhysicsBody *body = &world->physics_bodies[i];
		if(!body->inuse) {
			continue;
		}
		memset(body->collisions, 0, MAX_REPORTED_COLLISIONS*sizeof(Collision));
		apply_world_bounds(body);
		apply_damping(body, delta);
		apply_gravity(world, body, delta);
		integrate(body, delta);
		for(int j = 0; j < world->max_physics_bodies; j++) {
			if(!world->physics_bodies[j].inuse || j == i) {
				continue;
			}
			solve_collision(world, body, &world->physics_bodies[j], delta, false);
		}
		if(body->update) {
			body->update(body, world, delta);
		}
	}
}

void physics_draw_sprites(PhysicsWorld *world) {
	for(int i = 0; i < world->max_physics_bodies; i++) {
		PhysicsBody *body = &world->physics_bodies[i];
		if(!body->inuse) {
			continue;
		}
		if(!body->sprite) {
			continue;
		}
		float rotation = body->rotation*(180.0/M_PI);
		
		Vector2D position;
		vector2d_add(position, body->position, vector2d_rotate(body->sprite_offset, body->rotation));

		gf2d_sprite_draw(
				body->sprite,
				position,
				&body->sprite_scale,
				NULL,
				&rotation,
				NULL,
				NULL,
				0);
	}
}

void physics_debug_draw(PhysicsWorld *world) {
	for(int i = 0; i < world->max_physics_bodies; i++) {
		PhysicsBody *body = &world->physics_bodies[i];
		if(!body->inuse) {
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
				if(body->tags & TAG_DEAD) {
					color.a = body->timer;
				}
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
				gf2d_draw_circle(com, 20, gfc_color(1.0, 1.0, 0.0, color.a));
				break;
			}
			default:
				break;
		}
	}
}