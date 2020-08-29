#include "Main.h"

// handles closing the window
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		// if esc key was pressed
		if (wParam == 27)
			PostQuitMessage(0);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}
}

HRESULT ThrowErrorWindow(const wchar_t* message)
{
	MessageBox(NULL, message, L"Error!", MB_ICONEXCLAMATION | MB_OK);
	exit(0);
	return E_FAIL;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int mCmdShow)
{
#pragma region Window Initialization
	// if it was able to allocate me a console
	if (AllocConsole())
	{
		// file pointer that we will interface with our console
		FILE* stream;
		_wfreopen_s(&stream, L"CONIN$", L"rb", stdin);
		_wfreopen_s(&stream, L"CONOUT$", L"wb", stdout);
		_wfreopen_s(&stream, L"CONOUT$", L"wb", stdout);
	}

	// setting a lot of properties of our window
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(wc.hInstance, (LPCTSTR)IDI_ICON);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"MEWINDOW";
	wc.hIconSm = LoadIcon(wc.hInstance, (LPCTSTR)IDI_ICON);

	if (!RegisterClassEx(&wc))
	{
		return ThrowErrorWindow(L"Window registration failed!");
	}

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwWindowStyle = WS_OVERLAPPEDWINDOW;

	RECT rc = { (long)0, (long)0, (long)600, (long)600 };
	AdjustWindowRectEx(&rc, dwWindowStyle, FALSE, dwExStyle);

	HWND hWindowHandler = CreateWindowEx(dwExStyle, L"MEWINDOW", L"OpenGL - Window", dwWindowStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		100, 100, 600, 600, NULL, NULL, hInstance, NULL);

	if (hWindowHandler == NULL)
	{
		return ThrowErrorWindow(L"Window creation failed!");
	}

	ShowWindow(hWindowHandler, mCmdShow);
	UpdateWindow(hWindowHandler);

#pragma endregion

	// think of OpenGL like canvas in HTML
#pragma region OpenGL Initialization
	HDC hDC = NULL;

	if (!(hDC = GetDC(hWindowHandler)))
	{
		return ThrowErrorWindow(L"Unable to get window drawing context!");
	}

	static PIXELFORMATDESCRIPTOR pfd = {	// tells window how we want to render
		sizeof(PIXELFORMATDESCRIPTOR),		// size of this pixel format descriptor
		1,									// version number
		PFD_DRAW_TO_WINDOW |				// must draw to window
		PFD_SUPPORT_OPENGL |				// must support OpenGL
		PFD_DOUBLEBUFFER,					// must support double buffering
		PFD_TYPE_RGBA,						// ask for rgba format
		16,									// select color depth
		0, 0, 0, 0, 0, 0,					// color bits ignored (all 6)
		0,									// no alpha buffer
		0,									// no shift bit
		0,									// no accumulation buffer
		0, 0, 0, 0,							// accumulation bits ignored
		32,									// 32 bit Z (Depth) buffer
		0,									// no stencil buffer
		0,									// no auxiliary bufer
		PFD_MAIN_PLANE,						// main drawing layer
		0,									// reserved
		0, 0, 0								// layer masks ginored
	};

	GLuint PixelFormat;
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		return ThrowErrorWindow(L"Unable to find suitable pixel format!");
	}
	if (!(SetPixelFormat(hDC, PixelFormat, &pfd)))
	{
		return ThrowErrorWindow(L"Unable to find suitable pixel format!");
	}

	HGLRC hRC = NULL;
	if (!(hRC = wglCreateContext(hDC)))
	{
		return ThrowErrorWindow(L"Cannot create a rendering context!");
	}

	HGLRC tempHRC = wglCreateContext(hDC);
	if (!(wglMakeCurrent(hDC, tempHRC)))
	{
		return ThrowErrorWindow(L"Could not activate the rendering context!");
	}

	// Pick OpenGL version 3.1
	int nMajor = 3;
	int nMinor = 1;

	int attributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, nMajor,
		WGL_CONTEXT_MINOR_VERSION_ARB, nMinor,
		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		0
	};

	if (wglewIsSupported("GL_ARB_create_context") == 1)
	{
		// create context with specified versions
		hRC = wglCreateContextAttribsARB(hDC, NULL, attributes);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tempHRC);
		wglMakeCurrent(hDC, hRC);
	}

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		return ThrowErrorWindow(L"Failed to initialize GLEW!");
	}

	glClearColor(0.39f, 0.58f, 0.93f, 1.0f); // cornflower blue

#pragma endregion

	// GLSL - OpenGL shading language
#pragma region Shader Initialization

	// literal string
	const GLchar* vertexShader = R"glsl(
		#version 330
		in vec3 positionBuffer;
		void main()
		{
			gl_Position = vec4(positionBuffer, 1.0);
		}
	)glsl";

	// that vec4 is the color setter
	const GLchar* fragmentShader = R"glsl(
		#version 330
		out vec4 fragment;
		void main()
		{
			fragment = vec4(1.0, 0.0, 0.0, 1.0);
			return;
		}
	)glsl";

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexShader, NULL);
	glCompileShader(vertexShaderID);

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentShader, NULL);
	glCompileShader(fragmentShaderID);

	GLuint shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShaderID);
	glAttachShader(shaderProgramID, fragmentShaderID);
	glLinkProgram(shaderProgramID);

	GLint result = false;
	GLint log = false;

	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &result);
	glGetProgramiv(shaderProgramID, GL_INFO_LOG_LENGTH, &log);

	if (log > 0)
	{
		return ThrowErrorWindow(L"Shaders Failed to Compile");
	}

	glUseProgram(shaderProgramID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
#pragma endregion

#pragma region Shape Initialization

	// make one vertex array object and make it active
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// making corners of the triangle (x, y, z)
	GLfloat positions[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	// vertex buffer object
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), &positions, GL_STATIC_DRAW);

	GLint positionBufferID = glGetAttribLocation(shaderProgramID, "positionBuffer");

	glEnableVertexAttribArray(positionBufferID);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
#pragma endregion

#pragma region Game Loop

	std::cout << "Game Loop Started" << std::endl;
	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// otherwise run our game code
		else
		{
			glClear(GL_COLOR_BUFFER_BIT);

			glDrawArrays(GL_TRIANGLES, 0, 3);
			SwapBuffers(hDC);
		}
	}
#pragma endregion

	glDeleteProgram(shaderProgramID);
	glDeleteBuffers(1, &vbo);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);

	// code after we rendered to the console in Window Initialization
	// Project -> Properties -> Linker -> System -> SubSystem
	std::cout << "Hello OpenGL!\n";

	// ending the program
	std::cout << "Press enter to finish";
	getchar();
	return 0;
}