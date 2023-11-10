/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright 2020- The GROMACS Authors
 * and the project initiators Erik Lindahl, Berk Hess and David van der Spoel.
 * Consult the AUTHORS/COPYING files and https://www.gromacs.org for details.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * https://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at https://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out https://www.gromacs.org.
 */
/*! \internal \file
 * \brief
 * Implements nblib simulation box
 *
 * \author Victor Holanda <victor.holanda@cscs.ch>
 * \author Joe Jordan <ejjordan@kth.se>
 * \author Prashanth Kanduri <kanduri@cscs.ch>
 * \author Sebastian Keller <keller@cscs.ch>
 */
#include "nblib/box.h"

#include <cmath>

#include <algorithm>

#include "nblib/exception.h"

namespace nblib
{

Box::Box(real l) : Box(l, l, l) {}

Box::Box(real x, real y, real z) : Box({ x, 0, 0, 0, y, 0, 0, 0, z }) {}

Box::Box(std::array<real, 9> boxMatrix) : legacyMatrix_{ { 0 } }
{
    bool hasNaN =
            std::any_of(boxMatrix.begin(), boxMatrix.end(), [](auto val) { return std::isnan(val); });
    bool hasInf =
            std::any_of(boxMatrix.begin(), boxMatrix.end(), [](auto val) { return std::isinf(val); });
    if (hasNaN || hasInf)
    {
        throw InputException("Cannot have NaN or Inf box length.");
    }

    fillMatrix(boxMatrix, legacyMatrix_);
}

void fillMatrix(std::array<real, 9> boxMatrix, Box::LegacyMatrix legacyMatrix)
{
    for (int i = 0; i < dimSize; ++i)
    {
        for (int j = 0; j < dimSize; ++j)
        {
            legacyMatrix[i][j] = boxMatrix[j + i * dimSize];
        }
    }
}

bool operator==(const Box& rhs, const Box& lhs)
{
    using real_ptr = const real*;
    return std::equal(real_ptr(rhs.legacyMatrix()),
                      real_ptr(rhs.legacyMatrix()) + dimSize * dimSize,
                      real_ptr(lhs.legacyMatrix()));
}

} // namespace nblib
