
class Body
{
public:
	Body(double distance, double speed, double radius_, double mass_, double tilt_, double rotSpeed_, unsigned int texture_)
	{
		// initialize parameters
		position = dvec3(0, 0, distance);
		velocity = dvec3(speed, 0, 0);
		radius = radius_;
		mass = mass_;
		tilt = tilt_;
		rotAngle = 0;
		rotSpeed = rotSpeed_;
		texture = texture_;
	}

	dvec3 calcForce(Body& other)
	{
		// direction vector and distance to other body
		dvec3 direction = other.position - position;
		double distance = direction.length();
		direction.normalize();

		// gravitational force from other body
		const double gravity = 6.6743e-11;
		double k = gravity * mass * other.mass / (distance * distance);
		return direction * k;
	}

	void update(dvec3 force, double timeStep)
	{
		// acceleration is force/mass
		dvec3 acceleration = force * (1 / mass);

		// update velocity, position, and rotational angle
		velocity += acceleration * timeStep;
		position += velocity * timeStep;
		rotAngle += rotSpeed * timeStep;
	}

	dvec3 position, velocity;        // orbital properties
	double radius, mass;             // physical properties
	double tilt, rotAngle, rotSpeed; // spinning properties
	unsigned int texture;            // texture ID
};
