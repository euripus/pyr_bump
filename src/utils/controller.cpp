#include "Controller.h"
#include  <cmath>

Controller::Controller(RepeatType tp, double mind, double maxd)
    :
    repeat(tp),
    min_time(mind),
    max_time(maxd),
    active(true)
{
}

double Controller::getControlTime (double application_time) const
{
    if (repeat == RepeatType::RT_CLAMP)
    {
        // Clamp the time to the [min,max] interval.
        if (application_time < min_time)
        {
            return min_time;
        }
        if (application_time > max_time)
        {
            return max_time;    
        }
        return application_time;
    }

    double time_range = max_time - min_time;
    if (time_range > 0.0)
    {
        double multiples = (application_time - min_time)/time_range;
        double integer_time = std::floor(multiples);
        double fraction_time = multiples - integer_time;
        if (repeat == RepeatType::RT_WRAP)
        {
            return min_time + fraction_time*time_range;
        }

        // repeat == RepeatType::RT_CYCLE
        if (((int)integer_time) & 1)
        {
            // Go backward in time.
            return max_time - fraction_time*time_range;
        }
        else
        {
            // Go forward in time.
            return min_time + fraction_time*time_range;
        }
    }

    // The minimum and maximum times are the same, so return the minimum.
    return min_time;
}

