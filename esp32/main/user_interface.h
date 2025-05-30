#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include <Arduino.h>
#include <queue>
#include <functional>
#include "config.h"

#define THROTTLE_INTERVAL 2000 // Minimum time between same messages (ms)

// Message display durations (ms)
constexpr unsigned long 
    PRIO_LOW_DURATION = 2000,
    PRIO_NORMAL_DURATION = 3000,
    PRIO_HIGH_DURATION = 5000,
    PRIO_CRITICAL_DURATION = 8000;

// Message structure
struct UIMessage {
    String text;
    MessagePriority priority;
    unsigned long duration;
    std::function<void()> callback; // Optional callback
};

// Initialize the UI system
void uiInit();

// Add message to queue
void uiEnqueueMessage(const String& message, 
                     MessagePriority priority = MessagePriority::PRIO_NORMAL,
                     unsigned long customDuration = 0,
                     std::function<void()> callback = nullptr);

// Add formatted message (printf-style)
void uiEnqueueMessageF(MessagePriority priority, const char* format, ...);

// Process queue (call this in main loop)
void uiProcessQueue();

// Clear current message and queue
void uiClearAll();

// Check if any message is active
bool uiIsMessageActive();

// Get current message text
String uiGetCurrentMessage();

#endif // USER_INTERFACE_H