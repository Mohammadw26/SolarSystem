
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

	dvec3 operator-()
	{
		return dvec3(-x, -y, -z);
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

	void rotate(double angle, const dvec3& axis)
	{
		// rotation matrix
		mat4x4 M;
		mat4x4_identity(M);
		mat4x4_rotate(M, M, (float)axis.x, (float)axis.y, (float)axis.z, (float)angle);

		// matrix-vector multiplication
		vec4 v = { (float)x, (float)y, (float)z, 0 };
		vec4 r;
		mat4x4_mul_vec4(r, M, v);
		*this = dvec3(r[0], r[1], r[2]);
	}

	double x, y, z;
};

dvec3 normalize(const dvec3& other)
{
	dvec3 result = other;
	result.normalize();
	return result;
}

dvec3 cross(const dvec3& a, const dvec3& b)
{
	return dvec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

void lookAt(mat4x4 m, dvec3 eye, dvec3 center, dvec3 up)
{
	// adapted from mat4x4_look_at()

	dvec3 b = normalize(eye - center);
	dvec3 r = normalize(cross(up, b));
	dvec3 u = cross(b, r);

	mat4x4_identity(m);

	m[0][0] = (float)r.x;
	m[0][1] = (float)u.x;
	m[0][2] = (float)b.x;

	m[1][0] = (float)r.y;
	m[1][1] = (float)u.y;
	m[1][2] = (float)b.y;

	m[2][0] = (float)r.z;
	m[2][1] = (float)u.z;
	m[2][2] = (float)b.z;

	mat4x4_translate_in_place(m, -(float)eye.x, -(float)eye.y, -(float)eye.z);
}
