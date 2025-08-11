#pragma once

#include <esp_log.h>

#include <cmath>

#define PI_F         3.1415926535897932384626433832795f
#define HALF_PI_F    1.5707963267948966192313216916398f
#define TWO_PI_F     6.283185307179586476925286766559f
#define DEG_TO_RAD_F 0.017453292519943295769236907684886f
#define RAD_TO_DEG_F 57.295779513082320876798154814105f
#define EULER_F      2.718281828459045235360287471352f

struct Vector {
    float x, y, z;

    Vector(float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : x(x), y(y), z(z) {}

    void normalize() {
        float norm = std::sqrt(x * x + y * y + z * z);
        if (norm > 0.0f) {
            x /= norm;
            y /= norm;
            z /= norm;
        }
    }
};

struct Quaternion {
    float w, x, y, z;

    Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : w(w), x(x), y(y), z(z) {}

    void normalize() {
        float norm = std::sqrt(w * w + x * x + y * y + z * z);
        if (norm > 0.0f) {
            w /= norm;
            x /= norm;
            y /= norm;
            z /= norm;
        }
    }

    // Quaternion multiplication (Hamilton product)
    Quaternion operator*(const Quaternion& rhs) const {
        return Quaternion(
            w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z,
            w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
            w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
            w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w
        );
    }

    // Quaternion inverse (for unit quaternions, just conjugate)
    Quaternion inverse() const {
        return Quaternion(w, -x, -y, -z);
    }

    Vector toEulerAngles() const {
        const float sinx = 2.0f * (w * x + y * z);
        const float cosx = 1.0f - 2.0f * (x * x + y * y);
        const float ex = std::atan2(sinx, cosx);
        float t2 = 2.0f * (w * y - z * x);
        t2 = t2 > 1.0f ? 1.0f : t2;
        t2 = t2 < -1.0f ? -1.0f : t2;
        const float ey = std::asin(t2);
        const float sinz = 2.0f * (w * z + x * y);
        const float cosz = 1.0f - 2.0f * (y * y + z * z);
        const float ez = std::atan2(sinz, cosz);
        return Vector(ex, ey, ez);
    }

    static Quaternion fromEulerAngles(const Vector &euler) {
        float cx = std::cos(euler.x * 0.5f);
        float sx = std::sin(euler.x * 0.5f);
        float cy = std::cos(euler.y * 0.5f);
        float sy = std::sin(euler.y * 0.5f);
        float cz = std::cos(euler.z * 0.5f);
        float sz = std::sin(euler.z * 0.5f);
        return Quaternion(
            cx * cy * cz + sx * sy * sz,
            sx * cy * cz - cx * sy * sz,
            cx * sy * cz + sx * cy * sz,
            cx * cy * sz - sx * sy * cz
        );
    }

private:
    static bool testQEuler(float x, float y, float z) {
        Vector euler(x, y, z);
        Quaternion q = fromEulerAngles(euler);
        Vector eulerBack = q.toEulerAngles();
        const bool ok = (fabsf(euler.x - eulerBack.x) < 0.0001f &&
                fabsf(euler.y - eulerBack.y) < 0.0001f &&
                fabsf(euler.z - eulerBack.z) < 0.0001f);
        ESP_LOGI("Test", "%s: Euler: (%.3f, %.3f, %.3f) -> Quaternion: (%.3f, %.3f, %.3f, %.3f) -> Euler Back: (%.3f, %.3f, %.3f)",
                ok ? "Pass" : "Fail",
                x, y, z, q.w, q.x, q.y, q.z,
                eulerBack.x, eulerBack.y, eulerBack.z);
        return ok;
    }
public:
    static void testQEulers() {
        testQEuler(0.0f, 0.0f, 0.0f);
        testQEuler(0.0f, 0.0f, 1.5707963267948966f); // 90 degrees around Y-axis
        testQEuler(0.0f, 1.5707963267948966f, 0.0f); // 90 degrees around X-axis
        testQEuler(1.5707963267948966f, 0.0f, 0.0f); // 90 degrees around Z-axis
        testQEuler(0.0f, 1.5707963267948966f, 1.5707963267948966f); // 90 degrees around Y and Z
        testQEuler(1.5707963267948966f, 0.0f, 1.5707963267948966f); // 90 degrees around X and Z
        testQEuler(1.5707963267948966f, 1.5707963267948966f, 0.0f); // 90 degrees around X and Y
        testQEuler(1.5707963267948966f, 1.5707963267948966f, 1.5707963267948966f); // 90 degrees around all axes
        testQEuler(0.0f, -1.5707963267948966f, 0.0f);   // -90 degrees around Y-axis
        testQEuler(-1.5707963267948966f, 0.0f, 0.0f);   // -90 degrees around X-axis
        testQEuler(0.0f, 0.0f, -1.5707963267948966f);   // -90 degrees around Z-axis
        testQEuler(0.0f, -1.5707963267948966f, -1.5707963267948966f); // -90 degrees around Y and Z
        testQEuler(-1.5707963267948966f, 0.0f, -1.5707963267948966f); // -90 degrees around X and Z
        testQEuler(-1.5707963267948966f, -1.5707963267948966f, 0.0f); // -90 degrees around X and Y
        testQEuler(-1.5707963267948966f, -1.5707963267948966f, -1.5707963267948966f); // -90 degrees around all axes
    }
};

