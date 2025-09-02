#pragma once

const int BPI = 1600;    // 1600 bits per inch
const double IPS = 10.0; // 10 inches per second

class tape_location_t
{
    double inches_;

public:
    class position
    {
    };
    class time
    {
    };
    class byte
    {
    };

    tape_location_t(double inches = 0.0, position xx = {}) : inches_(inches) {}
    tape_location_t(double t, time xx) : inches_(t * IPS) {}
    tape_location_t(size_t index, byte xx) : inches_(index * 8.0 / BPI) {}

    double inches() const { return inches_; }
    double seconds() const { return inches_ / IPS; }
    double bytes() const { return inches_ * BPI / 8.0; }

    bool operator<(const tape_location_t &other) const
    {
        return inches_ < other.inches_;
    }

    bool operator==(const tape_location_t &other) const
    {
        return inches_ == other.inches_;
    }

    tape_location_t operator+(tape_location_t other) const
    {
        return tape_location_t(inches_ + other.inches_);
    }

    static tape_location_t one_inch() { return tape_location_t(1.0); }
    static tape_location_t one_second() { return tape_location_t(1.0, time{}); }
    static tape_location_t one_byte() { return tape_location_t(1, byte{}); }

    std::string as_string() const
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%04.4fi|%03.5fs", inches_, seconds());
        return buffer;
    }
};
