
class Camera
{
public:
	Camera() : speed(0), moveRight(0), moveUp(0), moveFront(0), dragging(false), cursorX(0), cursorY(0)
	{
	}

	void set(dvec3 target, dvec3 direction, double distance, double speed_)
	{
		// set camera
		position = target + direction * distance;
		speed = speed_;
		moveRight = 0;
		moveUp = 0;
		moveFront = 0;

		// update directions
		front = normalize(-direction);
		right = normalize(cross(front, dvec3(0, 1, 0)));
		up = normalize(cross(right, front));
	}

	void keyPress(int key, int action)
	{
		// start moving
		if (action == GLFW_PRESS)
		{
			if (key == GLFW_KEY_LEFT) moveRight = -1;
			if (key == GLFW_KEY_RIGHT) moveRight = +1;
			if (key == GLFW_KEY_PAGE_UP) moveUp = +1;
			if (key == GLFW_KEY_PAGE_DOWN) moveUp = -1;
			if (key == GLFW_KEY_UP) moveFront = +1;
			if (key == GLFW_KEY_DOWN) moveFront = -1;
		}

		// stop moving
		if (action == GLFW_RELEASE)
		{
			if (key == GLFW_KEY_LEFT) moveRight = 0;
			if (key == GLFW_KEY_RIGHT) moveRight = 0;
			if (key == GLFW_KEY_PAGE_UP) moveUp = 0;
			if (key == GLFW_KEY_PAGE_DOWN) moveUp = 0;
			if (key == GLFW_KEY_UP) moveFront = 0;
			if (key == GLFW_KEY_DOWN) moveFront = 0;
		}
	}

	void mouseClick(GLFWwindow* window, int button, int action)
	{
		// enable dragging on left button click
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			dragging = true;
			glfwGetCursorPos(window, &cursorX, &cursorY);
		}

		// disable dragging on left button release
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			dragging = false;
	}

	void mouseMove(double xpos, double ypos)
	{
		// cursor dragging
		if (dragging)
		{
			// rotation angles
			double yaw = (xpos - cursorX) * 0.003;
			double pitch = (ypos - cursorY) * 0.003;

			// update directions
			right.rotate(yaw, up);
			front.rotate(yaw, up);
			up.rotate(pitch, right);
			front.rotate(pitch, right);

			cursorX = xpos;
			cursorY = ypos;
		}
	}

	void update(double timeStep)
	{
		double step = speed * timeStep;

		// update position
		if (moveRight != 0)
			position += right * moveRight * step;
		if (moveUp != 0)
			position += up * moveUp * step;
		if (moveFront != 0)
			position += front * moveFront * step;
	}

	void GetViewMatrix(mat4x4 matrix)
	{
		// set view matrix
		lookAt(matrix, position, position + front, up);
	}

private:
	dvec3 position;
	double speed;
	dvec3 right, up, front;           // direction vectors
	int moveRight, moveUp, moveFront; // moving state
	bool dragging;                    // rotation state
	double cursorX, cursorY;
};
