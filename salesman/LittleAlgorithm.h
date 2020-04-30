#pragma once
#include <vector>
#include <QStringList>
#include "utils/strfmt.h"
#include "namedstarsystem.h"

class LittleAlgorithm
{
private:
    std::vector<NamedStarSystem> source;
public:
    using weight_type = uint64_t;
    using matrix_type = std::map <size_t, std::map<size_t, weight_type>>;


    float originalLength{0.f};
    float lastRouteLen{0.f};

    explicit LittleAlgorithm(QStringList source_names); //star systems names list
    QStringList getRoute(const QString& startAt);
    static QStringList route(QStringList source_names, const QString& start, float *length = nullptr);

    static float pathLength(const std::vector<NamedStarSystem>& src);



    static void selfTest();
    static void selfTest2();
    static void selfTest3();
private:
    struct bisector
    {
        constexpr static auto INVALID_IJ = std::numeric_limits<size_t>::max();
        size_t i{INVALID_IJ}; //index
        size_t j{INVALID_IJ}; //index
        LittleAlgorithm::weight_type d{0};
        bisector() = default;
        bisector(size_t i, size_t j): i(i), j(j) {}
        bisector(size_t i, size_t j, weight_type d): i(i), j(j), d(d) {}
        bool operator < (const bisector& c) const noexcept
        {
            return d < c.d;
        }

        bool operator <= (const bisector& c) const noexcept
        {
            return d <= c.d;
        }

        matrix_type makeSmallerMatrix(matrix_type matrix) const;

        operator std::string () const
        {
            return stringfmt("(I: %zu; J: %zu), D: %u", i + 1, j + 1, d);
        }
    };
    std::vector<bisector> result;

    LittleAlgorithm() = default;
    static std::vector<bisector> matrixProcedure(const std::vector<std::vector<weight_type>> &src);
};

