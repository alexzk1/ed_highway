#include "LittleAlgorithm.h" // IWYU pragma: keep

#include "dump_help.h"
#include "edsmwrapper.h"
#include "namedstarsystem.h"
#include "point.h"
#include "utils/containers_helpers.h"
#include "utils/strutils.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <utility>

#ifdef SRC_PATH
// #define DUMP_DURING_WORKING
#endif

#define INFW (std::numeric_limits<LittleAlgorithm::weight_type>::max())

// NOLINTNEXTLINE
LittleAlgorithm::LittleAlgorithm(QStringList source_names)
{
    utility::RemoveDuplicatesKeepOrder<QString, QStringList>(source_names);
    if (source_names.size() < 3)
    {
        if (source_names.size() == 1)
        {
            result.emplace_back(0, 0, 0);
        }

        if (source_names.size() == 2)
        {
            result.emplace_back(0, 1, 0);
        }

        return;
    }
    {
        const auto src_js = EDSMWrapper::requestManySysInfo(source_names);
        source.reserve(src_js.size());
        std::transform(std::begin(src_js), std::end(src_js), std::back_inserter(source),
                       [](const auto &js) {
                           return NamedStarSystem::fromJsonInfo(js);
                       });

        types_ns::remove_if(source, [](const auto &v) {
            return v.blank;
        });

        originalLength = pathLength(source);

        if (source.size() < 3)
        {
            if (source.size() == 1)
            {
                result.emplace_back(0, 0, 0);
            }

            if (source.size() == 2)
            {
                result.emplace_back(0, 1, 0);
            }

            return;
        }
    }

    // will be normalizing floats to uint64 with 2 decimal parts (float*100)
    // so algo is simplier for integer weights

    const auto static toWeightT = [](const float v) -> weight_type {
        // return static_cast<weight_type>(std::ceil(v / 30.f * 100.f));
        return static_cast<weight_type>(v * 100.f);
    };

    std::vector<std::vector<weight_type>> distances;
    distances.resize(source.size());
    for (std::size_t i = 0, sz = source.size(); i < sz; ++i)
    {
        distances.at(i).reserve(sz);
        const Point &pi = source.at(i).p;

        for (std::size_t j = 0; j < sz; ++j)
            distances.at(i).push_back((i == j) ? INFW : toWeightT(pi.no_sqrt_dist(source.at(j).p)));
    }

    result = matrixProcedure(distances);
}

QStringList LittleAlgorithm::getRoute(const QString &startAt)
{
    QStringList res;
    float length = 0.f;
    auto sit = std::find(source.begin(), source.end(), startAt);
    if (sit != source.end())
    {
        if (source.size() < 3)
        {
            res.push_back(*sit);
            if (source.size() == 2)
            {
                if (sit == source.begin())
                {
                    res.push_back(source.back());
                }
                else
                {
                    res.push_back(*source.begin());
                }
                length = source.begin()->p.distance(source.back().p);
            }
        }
        else
        {
            const size_t startindex = std::distance(source.begin(), sit);

            for (size_t index = startindex;;)
            {
                res.push_back(source.at(index));
                const auto fit =
                  std::find_if(result.begin(), result.end(), [&index](const auto &e) {
                      return e.i == index;
                  });

                if (fit == result.end())
                {
                    break;
                }

                length += source.at(index).p.distance(source.at(fit->j).p);
                index = fit->j;
                if (index == startindex)
                {
                    break;
                }
            }
        }
    }
    lastRouteLen = length;
    return res;
}

QStringList LittleAlgorithm::route(QStringList source_names, const QString &start, float *length)
{
    LittleAlgorithm a(std::move(source_names));
    QStringList l1 = a.getRoute(start);
    if (length)
    {
        *length = a.lastRouteLen;
    }
    return l1;
}

float LittleAlgorithm::pathLength(const std::vector<NamedStarSystem> &src)
{
    float res = 0.f;
    if (src.size() > 1)
    {
        for (size_t i = 0, top = src.size() - 1; i < top; ++i)
        {
            res += src.at(i).p.distance(src.at(i + 1).p);
        }
    }
    return res;
}

namespace {
template <class Cont, class Pred>
void ForEachInRow(Cont &src, size_t row, const Pred &cb)
{
    auto it = src.find(row);
    if (it != src.end())
    {
        for (auto &m2 : it->second)
        {
            cb(m2.second);
        }
    }
}

template <class Cont, class Pred>
void ForEachInCol(Cont &src, size_t col, const Pred &cb)
{
    for (auto &m1 : src)
    {
        auto it = m1.second.find(col);
        if (it != m1.second.end())
        {
            cb(it->second);
        }
    }
}

template <class Cont, class Pred>
void ForEach(Cont &src, const Pred &func)
{
    for (auto &m1 : src)
    {
        for (auto &m2 : m1.second)
        {
            func(m2.second, m1.first, m2.first);
        }
    }
}

template <class Cont>
void dumpMatrix(const Cont &src)
{
    size_t ci = INFW;
    std::cout << "\t";
    for (auto &m1 : src.begin()->second)
    {
        std::cout << "J" << m1.first + 1 << "\t";
    }
    std::cout << std::endl;
    ForEach(src, [&ci](auto v, auto i, auto) {
        if (i != ci)
        {
            if (ci != INFW)
            {
                std::cout << std::endl;
            }
            ci = i;
            std::cout << "I" << i + 1 << "\t";
        }
        if (v > INFW / 2)
        {
            std::cout << "INF";
        }
        else
        {
            std::cout << v;
        }
        std::cout << "\t";
    });
    std::cout << std::endl;
}

enum class check : std::uint8_t {
    Row,
    Col
};

template <class Cont>
LittleAlgorithm::weight_type getMin(Cont &matrix, size_t sel, check pos)
{
    auto min = std::numeric_limits<LittleAlgorithm::weight_type>::max();
    const auto cb = [&min](const auto &v) {
        min = std::min(v, min);
    };
    if (check::Row == pos)
    {
        ForEachInRow(matrix, sel, cb);
    }
    else
    {
        ForEachInCol(matrix, sel, cb);
    }

    return min;
}

template <class Cont>
LittleAlgorithm::weight_type getMax(Cont &matrix, size_t sel, check pos)
{
    auto max = std::numeric_limits<LittleAlgorithm::weight_type>::min();
    const auto cb = [&max](const auto &v) {
        max = std::max(v, max);
    };
    if (check::Row == pos)
    {
        ForEachInRow(matrix, sel, cb);
    }
    else
    {
        ForEachInCol(matrix, sel, cb);
    }

    return max;
}

template <class Cont>
LittleAlgorithm::weight_type normalizeMatrix(Cont &matrix)
{

    LittleAlgorithm::weight_type sum = 0;
    for (const auto &r : matrix)
    {
        const auto min = getMin(matrix, r.first, check::Row);
        sum += min;
        ForEachInRow(matrix, r.first, [&min](auto &v) {
            v -= min;
        });
    }

    for (const auto &c : matrix.begin()->second)
    {
        const auto min = getMin(matrix, c.first, check::Col);
        sum += min;
        ForEachInCol(matrix, c.first, [&min](auto &v) {
            v -= min;
        });
    }

    return sum;
}

template <class Cont>
void inf_if(Cont &matrix, size_t a, size_t b)
{
    auto it1 = matrix.find(a);
    if (it1 != matrix.end())
    {
        auto it2 = it1->second.find(b);
        if (it2 != it1->second.end())
        {
            it2->second = INFW;
        }
    }
}

template <class Cont>
void ensure_infinity(Cont &matrix)
{
    // need to make sure each row/column has INF
    constexpr static auto almost_inf = static_cast<LittleAlgorithm::weight_type>(0.75 * INFW);
    size_t wr = LittleAlgorithm::bisector::INVALID_IJ;
    for (const auto &kv_row : matrix)
    {
        if (getMax(matrix, kv_row.first, check::Row) < almost_inf)
        {
            wr = kv_row.first;
            break;
        }
    }

    if (wr != LittleAlgorithm::bisector::INVALID_IJ)
    {
        size_t wc = LittleAlgorithm::bisector::INVALID_IJ;
        for (const auto &kv_col : matrix.begin()->second)
        {
            if (getMax(matrix, kv_col.first, check::Col) < almost_inf)
            {
                wc = kv_col.first;
                break;
            }
        }
        inf_if(matrix, wr, wc);
    }
}
} // namespace

std::vector<LittleAlgorithm::bisector>
LittleAlgorithm::matrixProcedure(const std::vector<std::vector<weight_type>> &src)
{
    matrix_type matrix;
    for (size_t i = 0, sz = src.size(); i < sz; ++i)
    {
        auto &m1 = matrix[i];
        for (size_t j = 0; j < sz; ++j)
        {
            m1[j] = src[i][j];
        }
    }

    std::vector<bisector> result;

    while (matrix.size() > 2)
    {
#ifdef DUMP_DURING_WORKING
        std::cout << "Loop step, source:\n";
        dumpMatrix(matrix);
#endif
        ensure_infinity(matrix);
        auto S = normalizeMatrix(matrix);

#ifdef DUMP_DURING_WORKING
        std::cout << "Reduced:\n";
        dumpMatrix(matrix);
#endif
        bisector max_dij;
        std::vector<bisector> dij;
        ForEach(matrix, [&max_dij, &matrix, &dij](auto &value, auto i, auto j) {
            if (0 == value)
            {
                value = INFW;
                bisector tmp(i, j, getMin(matrix, i, check::Row) + getMin(matrix, j, check::Col));
                value = 0;
                if (max_dij.d == tmp.d)
                {
                    dij.push_back(tmp);
                }
                if (max_dij < tmp)
                {
                    max_dij = tmp;
                    dij.clear();
                    dij.push_back(tmp);
                }
            }
        });

#ifdef DUMP_DURING_WORKING
        std::cout << "Max dij options: " << std::endl;
        dump_helper::dumpContainer(dij);
#endif
        const auto Swo = max_dij.d + S;

        matrix_type reducedMatrix;
        weight_type Sw = 0; // INFW;
        for (const auto &d : dij)
        {
            auto mt = d.makeSmallerMatrix(matrix);
            auto st = normalizeMatrix(mt);
            if (st >= Sw)
            {
                Sw = st;
                reducedMatrix = std::move(mt);
                max_dij = d;
            }
        }

        if (Swo < Sw)
        {
            matrix[max_dij.i][max_dij.j] = INFW;
        }
        else
        {
            matrix = std::move(reducedMatrix);
            result.push_back(std::move(max_dij));
        }
#ifdef DUMP_DURING_WORKING
        std::cout << std::endl;
#endif
    }
    ensure_infinity(matrix);

#ifdef DUMP_DURING_WORKING
    std::cout << "Final matrix:\n";
    dumpMatrix(matrix);
#endif

    ForEach(matrix, [&result](const auto &value, auto i, auto j) {
        if (0 == value)
        {
            result.emplace_back(i, j);
        }
    });

    return result;
}

LittleAlgorithm::matrix_type
LittleAlgorithm::bisector::makeSmallerMatrix(LittleAlgorithm::matrix_type matrix) const
{
    matrix.erase(i);

    for (auto &m : matrix)
    {
        m.second.erase(j);
    }

    // i, j is used (removed), so can't do j/i as well

    inf_if(matrix, j, i);
    return matrix;
}

void LittleAlgorithm::selfTest()
{
    // https://math.semestr.ru/kom/little.php
    const auto res = matrixProcedure({
      {INFW, 31, 15, 19, 8, 55},
      {19, INFW, 22, 31, 7, 35},
      {25, 43, INFW, 53, 57, 16},
      {5, 50, 49, INFW, 39, 9},
      {24, 24, 33, 5, INFW, 14},
      {34, 26, 6, 3, 36, INFW},
    });
    // expecting list: (4,1), (1,3), (3,6), (6,2), (2,5), (5,4),
    dump_helper::dumpContainer(res);
}

void LittleAlgorithm::selfTest2()
{
    const auto names = EDSMWrapper::selectSystemsInRadiusNamesOnly("Borann", 30);
    // LittleAlgorithm a(names);
    // dump_helper::dumpContainer(a.result);
    auto l = route(names, "Borann");
    dump_helper::dumpContainer(l);
}

void LittleAlgorithm::selfTest3()
{
    float len = 0.0f;
    auto l = route(
      {
        "Atrimih",
        "Acokwech",
        "No Mina",
        "Milceni",
        "Jambavan",
        "Rang Guans",
        "CD-48 3774",
        "Limani",
        "Nunet",
        "Bean Nighe",
        "Kamarada",
        "Helgardu",
        "Sengen Sama",
        "Sounti",
        "Quariti",

        "Dountidi",
        "Shinrarta Dezhra",
      },
      "Shinrarta Dezhra", &len);
    dump_helper::dumpContainer(l);
    std::cout << "Route len: " << len << " ly." << std::endl;
}
