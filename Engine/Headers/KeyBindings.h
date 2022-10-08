#pragma once
#include <string>

namespace Core
{
	class App;

	enum class InputDir
	{
		Forwards,
		Backwards,
		Right,
		Left,
		Jump,
		Sneak,
	};

	class KeyBindings
	{
	private:
		static bool IsKeyAlreadyUsed(int key);

	public:
		static int selectedButton;
		static int directionKeybinds[6];
	
		// Returns false if the user tried to assign a key that was already in use.
		static bool ChangeBinding(GLFWwindow* window, const ImVec2& buttonSize, const InputDir& dir);

		static std::string CharValToStr (const int& charValue);
		static std::string InputDirToStr(const InputDir& dir);
	};
}