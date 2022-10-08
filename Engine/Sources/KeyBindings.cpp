#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>

#include "KeyBindings.h"
#include "App.h"
using namespace Core;

int KeyBindings::directionKeybinds[6] = { GLFW_KEY_W , GLFW_KEY_S , GLFW_KEY_D , GLFW_KEY_A , GLFW_KEY_SPACE, GLFW_KEY_LEFT_SHIFT };
int KeyBindings::selectedButton = -1;

std::string KeyBindings::InputDirToStr(const InputDir& dir)
{
	switch (dir)
	{
	case Core::InputDir::Forwards:
		return "Forwards";
	case Core::InputDir::Backwards:
		return "Backwards";
	case Core::InputDir::Right:
		return "Right";
	case Core::InputDir::Left:
		return "Left";
	case Core::InputDir::Jump:
		return "Jump";
	case Core::InputDir::Sneak:
		return "Sneak";
	default:
		return "";
	}
}

bool KeyBindings::IsKeyAlreadyUsed(int key)
{
	for (int i = 0; i < 6; i++)
		if (key == directionKeybinds[i])
			return true;
	return false;
}

std::string KeyBindings::CharValToStr(const int& charValue)
{
	if (charValue >= GLFW_KEY_A && charValue <= GLFW_KEY_Z)
	{
		return "[" + std::string((char*)&charValue) + "]";
	}

	switch (charValue)
	{
	case GLFW_KEY_SPACE:
		return "[Space]";
	case GLFW_KEY_LEFT_SHIFT:
		return "[Left shift]";
	case GLFW_KEY_LEFT_CONTROL:
		return "[Left ctrl]";
	case GLFW_KEY_LEFT_ALT:
		return "[Left alt]";
	default:
		return "Unhandled key";
	}
}

bool KeyBindings::ChangeBinding(GLFWwindow* window, const ImVec2& buttonSize, const InputDir& dir)
{
	std::string nameDir = InputDirToStr(dir);
	std::string nameKey = CharValToStr(directionKeybinds[(int)dir]);

	if(ImGui::Button((nameDir + ": " + nameKey).c_str(), buttonSize))
		selectedButton = (int)dir;

	if (selectedButton == (int)dir)
	{
		for (int i = 0; i < GLFW_KEY_LAST; i++)
		{
			if (glfwGetKey(window, i) == GLFW_PRESS)
			{
				if (!IsKeyAlreadyUsed(i))
				{
					DebugLog(("The " + nameDir + " key has been updated from " + std::to_string(directionKeybinds[(int)dir]) + " to " + std::to_string(i)).c_str());
					directionKeybinds[(int)dir] = i;
					selectedButton = -1;
				}
				else
				{
					return false;
				}
			}
		}
	}
	return true;
}