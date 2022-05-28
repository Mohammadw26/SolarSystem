
// 3D vector with double components (larger range and precision than vec3)
class dvec3
{
public:
	dvec3() : x(0), y(0), z(0)
	{
	}

	dvec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_)
	{
	}

	void operator+=(const dvec3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void operator-=(const dvec3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}

	dvec3 operator+(const dvec3& other)
	{
		return dvec3(x + other.x, y + other.y, z + other.z);
	}

	dvec3 operator-(const dvec3& other)
	{
		return dvec3(x - other.x, y - other.y, z - other.z);
	}

	dvec3 operator*(double k)
	{
		return dvec3(x * k, y * k, z * k);
	}

	double length()
	{
		return sqrt(x * x + y * y + z * z);
	}

	void normalize()
	{
		double k = 1 / sqrt(x * x + y * y + z * z);
		x *= k;
		y *= k;
		z *= k;
	}

	double x, y, z;
};

void lookAt(mat4x4 m, dvec3 e, dvec3 c, dvec3 u)
{
	// convert dvec3 to vec3
	vec3 eye = { (float)e.x, (float)e.y, (float)e.z };
	vec3 center = { (float)c.x, (float)c.y, (float)c.z };
	vec3 up = { (float)u.x, (float)u.y, (float)u.z };
	mat4x4_look_at(m, eye, center, up);
}
