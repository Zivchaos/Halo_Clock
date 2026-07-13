#include "Notifications.h"

namespace
{
    constexpr uint8_t QUEUE_SIZE = 6;
    HaloNotification queue[QUEUE_SIZE];
    HaloNotification visible;
    uint8_t head = 0;
    uint8_t tail = 0;
    uint8_t count = 0;
    uint32_t visibleSince = 0;
    bool showing = false;

    void showNext()
    {
        if (count == 0)
        {
            showing = false;
            return;
        }

        visible = queue[head];
        head = (head + 1) % QUEUE_SIZE;
        --count;
        visibleSince = millis();
        showing = true;
    }
}

void Notifications::begin()
{
    head = tail = count = 0;
    showing = false;
}

void Notifications::update()
{
    if (!showing)
    {
        showNext();
        return;
    }

    if (millis() - visibleSince >= visible.durationMs)
    {
        showNext();
    }
}

bool Notifications::post(NotificationType type, const String& title, const String& message, uint32_t durationMs)
{
    if (count >= QUEUE_SIZE)
    {
        return false;
    }

    queue[tail] = {type, title, message, durationMs};
    tail = (tail + 1) % QUEUE_SIZE;
    ++count;
    return true;
}

bool Notifications::active()
{
    return showing;
}

const HaloNotification& Notifications::current()
{
    return visible;
}
