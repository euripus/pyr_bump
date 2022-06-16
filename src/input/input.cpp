#include "input.h"
#include <algorithm>

void Input::update()
{
	// call keys
	for(auto & [key, vec] : m_key_bind_map)
	{
		if(m_keys_states[static_cast<size_t>(key)])
		{
			for(auto & [func, desc] : vec)
			{
				func();
			}
		}
	}
	
	// call buttons
	for(auto & [but, vec] : m_mouse_bind_map)
	{
		if(m_mouse_buttons_state[static_cast<size_t>(but)])
		{
			for(auto & [func, desc] : vec)
			{
				func();
			}
		}
	}
	// clear button state
	m_mouse_buttons_state[static_cast<size_t>(Buttons::Button_Pos)] = false;
	m_mouse_buttons_state[static_cast<size_t>(Buttons::Button_Whell)] = false;
}

void Input::bindKeyFunctor(KeyboardKey key, std::function<void()> func, std::string desc)
{
    if(auto it = m_key_bind_map.find(key); it != m_key_bind_map.end())
    {
        it->second.push_back({func, desc});
    }
    else
    {
        auto v = std::vector<func_ptr>{};
        v.push_back({func, desc});
        m_key_bind_map[key] = std::move(v);
    }
}

void Input::bindButtonFunctor(Buttons button, std::function<void()> func, std::string desc)
{
    if(auto it = m_mouse_bind_map.find(button); it != m_mouse_bind_map.end())
    {
        it->second.push_back({func, desc});
    }
    else
    {
        auto v = std::vector<func_ptr>{};
        v.push_back({func, desc});
        m_mouse_bind_map[button] = std::move(v);
    }
}

void Input::keyEvent(KeyboardKey key, bool press)
{
    auto key_ind = static_cast<size_t>(key);

    if(press)
    {
        m_last_key             = key;
        m_keys_states[key_ind] = true;
    }
    else
    {
        m_keys_states[key_ind] = false;
    }
}

void Input::buttonEvent(Buttons button_id, bool press)
{
    if(press)
    {
        m_mouse_buttons_state[static_cast<size_t>(button_id)] = true;
    }
    else
    {
        m_mouse_buttons_state[static_cast<size_t>(button_id)] = false;
    }
}

void Input::mousePos(int32_t xpos, int32_t ypos)
{
    m_mouse_position = glm::ivec2{xpos, ypos};
    m_mouse_buttons_state[static_cast<size_t>(Buttons::Button_Pos)] = true;
}

void Input::mouseWhell(int32_t offset)
{
    m_mouse_wheel = offset;
	m_mouse_buttons_state[static_cast<size_t>(Buttons::Button_Whell)] = true;
}

bool Input::isAnyKeyPressed() const
{
    return std::any_of(std::begin(m_keys_states), std::end(m_keys_states), [](bool k) { return k; });
}

bool Input::getMouseButton(Buttons button_id) const
{
    return m_mouse_buttons_state[static_cast<size_t>(button_id)];
}

std::string KeyDescription(KeyboardKey key)
{
    switch(key)
    {
        case KeyboardKey::Key_0:
            return "0";
        case KeyboardKey::Key_1:
            return "1";
        case KeyboardKey::Key_2:
            return "2";
        case KeyboardKey::Key_3:
            return "3";
        case KeyboardKey::Key_4:
            return "4";
        case KeyboardKey::Key_5:
            return "5";
        case KeyboardKey::Key_6:
            return "6";
        case KeyboardKey::Key_7:
            return "7";
        case KeyboardKey::Key_8:
            return "8";
        case KeyboardKey::Key_9:
            return "9";
        case KeyboardKey::Key_A:
            return "A";
        case KeyboardKey::Key_B:
            return "B";
        case KeyboardKey::Key_C:
            return "C";
        case KeyboardKey::Key_D:
            return "D";
        case KeyboardKey::Key_E:
            return "E";
        case KeyboardKey::Key_F:
            return "F";
        case KeyboardKey::Key_G:
            return "G";
        case KeyboardKey::Key_H:
            return "H";
        case KeyboardKey::Key_I:
            return "I";
        case KeyboardKey::Key_J:
            return "J";
        case KeyboardKey::Key_K:
            return "K";
        case KeyboardKey::Key_L:
            return "L";
        case KeyboardKey::Key_M:
            return "M";
        case KeyboardKey::Key_N:
            return "N";
        case KeyboardKey::Key_O:
            return "O";
        case KeyboardKey::Key_P:
            return "P";
        case KeyboardKey::Key_Q:
            return "Q";
        case KeyboardKey::Key_R:
            return "R";
        case KeyboardKey::Key_S:
            return "S";
        case KeyboardKey::Key_T:
            return "T";
        case KeyboardKey::Key_U:
            return "U";
        case KeyboardKey::Key_V:
            return "V";
        case KeyboardKey::Key_W:
            return "W";
        case KeyboardKey::Key_X:
            return "X";
        case KeyboardKey::Key_Y:
            return "Y";
        case KeyboardKey::Key_Z:
            return "Z";
        case KeyboardKey::Key_F1:
            return "F1";
        case KeyboardKey::Key_F2:
            return "F2";
        case KeyboardKey::Key_F3:
            return "F3";
        case KeyboardKey::Key_F4:
            return "F4";
        case KeyboardKey::Key_F5:
            return "F5";
        case KeyboardKey::Key_F6:
            return "F6";
        case KeyboardKey::Key_F7:
            return "F7";
        case KeyboardKey::Key_F8:
            return "F8";
        case KeyboardKey::Key_F9:
            return "F9";
        case KeyboardKey::Key_F10:
            return "F10";
        case KeyboardKey::Key_F11:
            return "F11";
        case KeyboardKey::Key_F12:
            return "F12";
        case KeyboardKey::Key_F13:
            return "F13";
        case KeyboardKey::Key_F14:
            return "F14";
        case KeyboardKey::Key_F15:
            return "F15";
        case KeyboardKey::Key_F16:
            return "F16";
        case KeyboardKey::Key_F17:
            return "F17";
        case KeyboardKey::Key_F18:
            return "F18";
        case KeyboardKey::Key_F19:
            return "F19";
        case KeyboardKey::Key_F20:
            return "F20";
        case KeyboardKey::Key_F21:
            return "F21";
        case KeyboardKey::Key_F22:
            return "F22";
        case KeyboardKey::Key_F23:
            return "F23";
        case KeyboardKey::Key_F24:
            return "F24";
        case KeyboardKey::Key_UpArrow:
            return "Up";
        case KeyboardKey::Key_DownArrow:
            return "Down";
        case KeyboardKey::Key_LeftArrow:
            return "Left";
        case KeyboardKey::Key_RightArrow:
            return "Right";
        case KeyboardKey::Key_Home:
            return "Home";
        case KeyboardKey::Key_End:
            return "End";
        case KeyboardKey::Key_PageUp:
            return "PageUp";
        case KeyboardKey::Key_PageDown:
            return "PageDown";
        case KeyboardKey::Key_Insert:
            return "Insert";
        case KeyboardKey::Key_Delete:
            return "Delete";
        case KeyboardKey::Key_Escape:
            return "Escape";
        case KeyboardKey::Key_NumLock:
            return "NumLock";
        case KeyboardKey::Key_NumPad0:
            return "Numpad0";
        case KeyboardKey::Key_NumPad1:
            return "Numpad1";
        case KeyboardKey::Key_NumPad2:
            return "Numpad2";
        case KeyboardKey::Key_NumPad3:
            return "Numpad3";
        case KeyboardKey::Key_NumPad4:
            return "Numpad4";
        case KeyboardKey::Key_NumPad5:
            return "Numpad5";
        case KeyboardKey::Key_NumPad6:
            return "Numpad6";
        case KeyboardKey::Key_NumPad7:
            return "Numpad7";
        case KeyboardKey::Key_NumPad8:
            return "Numpad8";
        case KeyboardKey::Key_NumPad9:
            return "Numpad9";
        case KeyboardKey::Key_NumPadEnter:
            return "NumpadReturn";
        case KeyboardKey::Key_NumSubtract:
            return "Numpad-";
        case KeyboardKey::Key_NumAdd:
            return "NumpadPlus";
        case KeyboardKey::Key_NumMultiply:
            return "Multiply";
        case KeyboardKey::Key_NumDivide:
            return "Numpad/";
        case KeyboardKey::Key_NumPoint:
            return "Numpad.";
        case KeyboardKey::Key_NumComma:
            return "Numpad,";
        case KeyboardKey::Key_Num00:
            return "Numpad00";
        case KeyboardKey::Key_Num000:
            return "Numpad000";
        case KeyboardKey::Key_NumLeftParen:
            return "Numpad(";
        case KeyboardKey::Key_NumRightParen:
            return "Numpad)";
        case KeyboardKey::Key_NumLeftBrace:
            return "Numpadcase return ";
        case KeyboardKey::Key_NumRightBrace:
            return "Numpad}";
        case KeyboardKey::Key_NumTab:
            return "NumpadTab";
        case KeyboardKey::Key_NumBackspace:
            return "NumpadBackspace";
        case KeyboardKey::Key_NumA:
            return "NumpadA";
        case KeyboardKey::Key_NumB:
            return "NumpadB";
        case KeyboardKey::Key_NumC:
            return "NumpadC";
        case KeyboardKey::Key_NumD:
            return "NumpadD";
        case KeyboardKey::Key_NumE:
            return "NumpadE";
        case KeyboardKey::Key_NumF:
            return "NumpadF";
        case KeyboardKey::Key_NumXor:
            return "NumpadXor";
        case KeyboardKey::Key_NumPower:
            return "NumpadPower";
        case KeyboardKey::Key_NumPercent:
            return "Numpad%";
        case KeyboardKey::Key_NumLess:
            return "Numpad<";
        case KeyboardKey::Key_NumGreater:
            return "Numpad>";
        case KeyboardKey::Key_NumAmpersand:
            return "Numpad&";
        case KeyboardKey::Key_NumDblAmpersand:
            return "Numpad&&";
        case KeyboardKey::Key_NumVerticalBar:
            return "Numpad|";
        case KeyboardKey::Key_NumDblVerticalBar:
            return "Numpad||";
        case KeyboardKey::Key_NumColon:
            return "Numpad:";
        case KeyboardKey::Key_NumHash:
            return "Numpad#";
        case KeyboardKey::Key_NumSpace:
            return "NumpadSpace";
        case KeyboardKey::Key_NumAt:
            return "Numpad@";
        case KeyboardKey::Key_NumExclam:
            return "Numpad!";
        case KeyboardKey::Key_NumMemStore:
            return "NumpadMemStore";
        case KeyboardKey::Key_NumMemRecall:
            return "NumpadMemRecall";
        case KeyboardKey::Key_NumMemClear:
            return "NumpadMemClear";
        case KeyboardKey::Key_NumMemAdd:
            return "NumpadMemAdd";
        case KeyboardKey::Key_NumMemSubtract:
            return "NumpadMemSubtract";
        case KeyboardKey::Key_NumMemMultiply:
            return "NumpadMemMultiply";
        case KeyboardKey::Key_NumMemDivide:
            return "NumpadMemDivide";
        case KeyboardKey::Key_NumPlusMinus:
            return "NumpadPlusMinus";
        case KeyboardKey::Key_NumClear:
            return "NumpadClear";
        case KeyboardKey::Key_NumClearEntry:
            return "NumpadClearEntry";
        case KeyboardKey::Key_NumBinary:
            return "NumBinary";
        case KeyboardKey::Key_NumOctal:
            return "NumOctal";
        case KeyboardKey::Key_NumDecimal:
            return "NumDecimal";
        case KeyboardKey::Key_NumHexadecimal:
            return "NumHexadecimal";
        case KeyboardKey::Key_LeftBracket:
            return "[";
        case KeyboardKey::Key_LeftCtrl:
            return "LeftControl";
        case KeyboardKey::Key_LeftAlt:
            return "LeftAlt";
        case KeyboardKey::Key_LeftShift:
            return "LeftShift";
        case KeyboardKey::Key_LeftWin:
            return "LeftStart";
        case KeyboardKey::Key_RightBracket:
            return "]";
        case KeyboardKey::Key_RightCtrl:
            return "RightControl";
        case KeyboardKey::Key_RightAlt:
            return "RightAlt";
        case KeyboardKey::Key_RightShift:
            return "RightShift";
        case KeyboardKey::Key_RightWin:
            return "RightStart";
        case KeyboardKey::Key_PrintScreen:
            return "PrintScreen";
        case KeyboardKey::Key_ScrollLock:
            return "ScrollLock";
        case KeyboardKey::Key_Pause:
            return "Pause";
        case KeyboardKey::Key_Spacebar:
            return "Space";
        case KeyboardKey::Key_Backspace:
            return "Backspace";
        case KeyboardKey::Key_Enter:
            return "Return";
        case KeyboardKey::Key_Tab:
            return "Tab";
        case KeyboardKey::Key_Apps:
            return "AppMenu";
        case KeyboardKey::Key_CapsLock:
            return "Capital";
        case KeyboardKey::Key_Slash:
            return "/";
        case KeyboardKey::Key_Backslash:
            return "Backslash";
        case KeyboardKey::Key_Comma:
            return ",";
        case KeyboardKey::Key_Semicolon:
            return ";";
        case KeyboardKey::Key_Period:
            return ".";
        case KeyboardKey::Key_Grave:
            return "`";
        case KeyboardKey::Key_Apostrophe:
            return "'";
        case KeyboardKey::Key_Minus:
            return "-";
        case KeyboardKey::Key_Equals:
            return "=";

        default:
            return "UNKNOWN_KEY";
    }
}
