#include "user_interface.h"
#include "oled.h"
#include "config.h"
#include <cstdarg>

// Message queue
std::queue<UIMessage> messageQueue;
UIMessage* currentMessage = nullptr;
unsigned long messageDisplayStart = 0;
bool messageActive = false;

// Custom comparator for priority queue
struct MessageComparator {
    bool operator()(const UIMessage& a, const UIMessage& b) {
        return static_cast<int>(a.priority) < static_cast<int>(b.priority);
    }
};

// Priority queue for critical messages
std::priority_queue<UIMessage, std::vector<UIMessage>, MessageComparator> priorityQueue;

void uiInit() {
    // Initialize any required UI components
}
bool uiShouldThrottle(const String& message) {
  static String lastMessage;
  static unsigned long lastTime = 0;
  
  if (message == lastMessage && 
      millis() - lastTime < THROTTLE_INTERVAL) {
    return true;
  }
  
  lastMessage = message;
  lastTime = millis();
  return false;
}

void uiEnqueueMessage(const String& message, 
                     MessagePriority priority,
                     unsigned long customDuration,
                     std::function<void()> callback) {
    if (uiShouldThrottle(message)) {
        return; // Skip throttled messages
    }
    unsigned long duration = customDuration;
    if (duration == 0) {
        switch (priority) {
            case MessagePriority::PRIO_LOW: duration = PRIO_LOW_DURATION; break;
            case MessagePriority::PRIO_HIGH: duration = PRIO_HIGH_DURATION; break;
            case MessagePriority::PRIO_CRITICAL: duration = PRIO_CRITICAL_DURATION; break;
            default: duration = PRIO_NORMAL_DURATION;
        }
    }

    UIMessage newMsg = {
        message,
        priority,
        duration,
        callback
    };

    if (priority == MessagePriority::PRIO_CRITICAL) {
        priorityQueue.push(newMsg);
    } else {
        messageQueue.push(newMsg);
    }
}

void uiEnqueueMessageF(MessagePriority priority, const char* format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    uiEnqueueMessage(buffer, priority);
}

void uiProcessQueue() {
    // Handle critical messages first
    if (!priorityQueue.empty()) {
        // Clear normal queue if critical message arrives
        while (!messageQueue.empty()) messageQueue.pop();
        
        currentMessage = new UIMessage(priorityQueue.top());
        priorityQueue.pop();
    } 
    // Process normal queue
    else if (!messageQueue.empty() && !messageActive) {
        currentMessage = new UIMessage(messageQueue.front());
        messageQueue.pop();
    }

    // Display new message if available
    if (currentMessage != nullptr && !messageActive) {
        displayPage(3, currentMessage->text.c_str(), "");
        messageActive = true;
        messageDisplayStart = millis();
        return;
    }

    // Check if current message should expire
    if (messageActive && currentMessage != nullptr) {
        if (millis() - messageDisplayStart >= currentMessage->duration) {
            // Execute callback if exists
            if (currentMessage->callback) {
                currentMessage->callback();
            }
            
            // Clean up current message
            delete currentMessage;
            currentMessage = nullptr;
            messageActive = false;
            
            // Return to previous display
            nextPage();
        }
    }
}

void uiClearAll() {
    // Clear queues
    while (!messageQueue.empty()) messageQueue.pop();
    while (!priorityQueue.empty()) priorityQueue.pop();
    
    // Clear current message
    if (currentMessage) {
        delete currentMessage;
        currentMessage = nullptr;
    }
    
    messageActive = false;
}

bool uiIsMessageActive() {
    return messageActive;
}

String uiGetCurrentMessage() {
    return messageActive && currentMessage ? currentMessage->text : "";
}