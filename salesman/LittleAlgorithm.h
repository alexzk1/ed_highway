#pragma once

#include "edsmwrapper.h"
#include "namedstarsystem.h"
#include "utils/strfmt.h"

#include <QString>
#include <QStringList>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <vector>

class LittleAlgorithm
{
  private:
    std::vector<NamedStarSystem> source;

  public:
    using weight_type = std::uint64_t;
    using matrix_type = std::map<std::size_t, std::map<std::size_t, weight_type>>;

    float originalLength{0.f};
    float lastRouteLen{0.f};

    explicit LittleAlgorithm(QStringList source_names); // star systems names list
    QStringList getRoute(const QString &startAt);
    static QStringList route(QStringList source_names, const QString &start,
                             float *length = nullptr);

    static float pathLength(const std::vector<NamedStarSystem> &src);

    static void selfTest();
    static void selfTest2();
    static void selfTest3();
    struct bisector
    {
        constexpr static auto INVALID_IJ = std::numeric_limits<std::size_t>::max();
        std::size_t i{INVALID_IJ}; // index
        std::size_t j{INVALID_IJ}; // index
        LittleAlgorithm::weight_type d{0};
        bisector() = default;
        // NOLINTNEXTLINE
        bisector(size_t i, size_t j) :
            i(i),
            j(j)
        {
        }
        // NOLINTNEXTLINE
        bisector(size_t i, size_t j, weight_type d) :
            i(i),
            j(j),
            d(d)
        {
        }
        bool operator<(const bisector &c) const noexcept
        {
            return d < c.d;
        }

        bool operator<=(const bisector &c) const noexcept
        {
            return d <= c.d;
        }

        [[nodiscard]]
        matrix_type makeSmallerMatrix(matrix_type matrix) const;

        // NOLINTNEXTLINE
        operator std::string() const
        {
            return stringfmt("(I: %zu; J: %zu), D: %u", i + 1, j + 1, d);
        }
    };

  private:
    std::vector<bisector> result;

    LittleAlgorithm() = default;
    static std::vector<bisector> matrixProcedure(const std::vector<std::vector<weight_type>> &src);
};
