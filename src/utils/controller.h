#ifndef CONTROLLER_H
#define CONTROLLER_H

class Controller
{
public:
    enum class RepeatType
    {
        RT_CLAMP,
        RT_WRAP,
        RT_CYCLE
    };

    Controller() = default;
    Controller(RepeatType tp, double min, double max);

    // Conversion from application time units to controller time units.
    // Derived classes may use this in their update routines.
    double getControlTime(double application_time) const;
protected:
    RepeatType repeat   = RepeatType::RT_CLAMP;
    double     min_time = 0;
    double     max_time = 0;
    bool       active   = false;
};

#endif   // CONTROLLER_H
