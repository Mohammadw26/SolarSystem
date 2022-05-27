
class Body
{
public:
	Body(float x, float y, float z, float r, float m, unsigned int t)
	{
		// initialize parameters
		position[0] = x;
		position[1] = y;
		position[2] = z;
		velocity[0] = 0;
		velocity[1] = 0;
		velocity[2] = 0;
		radius = r;
		mass = m;
		texture = t;
	}

	void calcForce(const Body& other, vec3 force)
	{
		// direction vector to other body
		vec3 direction;
		vec3_sub(direction, other.position, position);

		// distance to other body
		float distance = vec3_len(direction);

		// normalized direction vector
		vec3_norm(direction, direction);

		// gravitational force from other body
		const float gravity = 1000;
		vec3_scale(force, direction, gravity * mass * other.mass / (distance * distance));
	}

	void update(vec3 force, float timeStep)
	{
		// acceleration is force/mass
		vec3 acceleration;
		vec3_scale(acceleration, force, 1 / mass);

		// update velocity by acceleration*timeStep
		vec3 velocityChange;
		vec3_scale(velocityChange, acceleration, timeStep);
		vec3_add(velocity, velocity, velocityChange);

		// update position by velocity*timeStep
		vec3 positionChange;
		vec3_scale(positionChange, velocity, timeStep);
		vec3_add(position, position, positionChange);
	}

	vec3 position;
	vec3 velocity;
	float radius;
	float mass;
	unsigned int texture;
};
