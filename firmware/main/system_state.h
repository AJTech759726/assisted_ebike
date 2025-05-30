#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    MODE_MANUAL,
    MODE_CRUISE_CONTROL
} bike_mode_t;

typedef enum {
    DIRECTION_LEFT,
    DIRECTION_CENTER,
    DIRECTION_RIGHT
} direction_t;

typedef struct {
    bool system_active;
    bool emergency_stop;
    uint8_t assistance_level; // 0-100%
    bike_mode_t bike_mode;
    float battery_voltage;
    float speed;
    bool left_radar;
    bool right_radar;
    direction_t direction;
    bool cruise_control_active;
    uint8_t cruise_control_speed;
} system_state_t;

// Events for UI updates
typedef enum {
    EVENT_SYSTEM_ACTIVATED,
    EVENT_SYSTEM_DEACTIVATED,
    EVENT_MODE_CHANGED,
    EVENT_RADAR_DETECTION,
    EVENT_SPEED_UPDATE,
    EVENT_CRUISE_CONTROL
} ui_event_type_t;

typedef struct {
    ui_event_type_t type;
    union {
        struct {
            bool left;
            bool right;
        } radar;
        float speed;
        bike_mode_t mode;
        bool cruise_active;
    } data;
} ui_event_t;
