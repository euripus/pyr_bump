#ifndef INPUT_H
#define INPUT_H

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <glm/glm.hpp>

#include "key_codes.h"

KeyboardKey MapKeyCode(int32_t platformKeyCode);
std::string KeyDescription(KeyboardKey key);

class Input
{
public:
    using func_ptr = std::pair<std::function<void()>, std::string>;   // functor, description

    Input()          = default;
    virtual ~Input() = default;

    virtual void update();   // call binding functions

    void bindKeyFunctor(KeyboardKey key, std::function<void()> func, std::string desc = {});
    void bindButtonFunctor(Buttons button, std::function<void()> func, std::string desc = {});
    void unbindKeyFunctor(KeyboardKey key);
    // !Note: called by input callback
    void buttonEvent(Buttons button_id, bool press);
    void mousePos(int32_t xpos, int32_t ypos);
    void mouseWhell(int32_t offset);
    void keyEvent(KeyboardKey key_code, bool press);

    // Keyboard
    KeyboardKey getKeyPressed() const { return m_last_key; }
    bool        isKeyPressed(KeyboardKey key_id) const { return m_keys_states[static_cast<size_t>(key_id)]; }
    bool        isAnyKeyPressed() const;

    // Mouse
    glm::ivec2 const & getMousePosition() const { return m_mouse_position; }
    bool               getMouseButton(Buttons button_id) const;
    int32_t            getMouseWheel() const { return m_mouse_wheel; }

    // TODO set mouse pos & hide cursor

private:
    std::map<KeyboardKey, std::vector<func_ptr>> m_key_bind_map;
    std::map<Buttons, std::vector<func_ptr>>     m_mouse_bind_map;
    // Keyboard states
    KeyboardKey m_last_key                                                     = KeyboardKey::Key_MaxKeyNum;
    bool        m_keys_states[static_cast<size_t>(KeyboardKey::Key_MaxKeyNum)] = {};
    // Mouse states
    glm::ivec2 m_mouse_position                                                 = {};
    int32_t    m_mouse_wheel                                                    = 0;
    bool       m_mouse_buttons_state[static_cast<size_t>(Buttons::ButtonCount)] = {};
};

#endif   // INPUT_H
