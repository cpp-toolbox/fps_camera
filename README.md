# fps_camera
A simple fps camera with no clip movement

```cpp
camera.process_input(
    input_state.is_pressed(EKey::LEFT_CONTROL),
    input_state.is_pressed(EKey::TAB),
    input_state.is_pressed(EKey::w),
    input_state.is_pressed(EKey::a),
    input_state.is_pressed(EKey::s),
    input_state.is_pressed(EKey::d),
    input_state.is_pressed(EKey::SPACE),
    input_state.is_pressed(EKey::LEFT_SHIFT),
    delta_time);
}
```
