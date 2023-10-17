/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright 2023- The GROMACS Authors
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
/* This file was inspired by ch5md by Pierre de Buyl (BSD license). */

#ifndef GMX_FILEIO_HDF5MDIO_H
#define GMX_FILEIO_HDF5MDIO_H

#include <string>

#include "gromacs/math/vectypes.h"
#include "gromacs/utility/real.h"

struct gmx_mtop_t;
typedef int64_t hid_t;


class GmxHdf5MdParticlesBox
{
private:
    int64_t numDatasetFrames_;
    int64_t numWrittenFrames_;
    int     numFramesPerChunk_;
    char    name_[16];
    void    initNumFramesPerChunk(int numFramesPerChunk);
public:
    GmxHdf5MdParticlesBox();
    GmxHdf5MdParticlesBox(int numFramesPerChunk);
    ~GmxHdf5MdParticlesBox()
    {};
    void setupForWriting(int numFramesPerChunk);
    void writeFrame(int64_t     step,
                    real        time,
                    hid_t       container,
                    const rvec* box);
};

class GmxHdf5MdParticlesProperties
{
private:
    int64_t numDatasetFrames_;
    int64_t numWrittenFrames_;
    int     numFramesPerChunk_;
    int64_t numAtoms_;
    char    name_[16];
    void    initNumFramesPerChunkAndNumAtoms(int numFramesPerChunk, int64_t numAtoms);
public:
    GmxHdf5MdParticlesProperties();
    GmxHdf5MdParticlesProperties(const char* name, int numFramesPerChunk, int64_t numAtoms);
    ~GmxHdf5MdParticlesProperties()
    {};
    void setupForWriting(int numFramesPerChunk, int64_t numAtoms);
    void writeFrame(int64_t          step,
                    real             time,
                    hid_t            container,
                    const rvec*      data);
};

class GmxHdf5MdIo
{
private:
    hid_t   file_;
    GmxHdf5MdParticlesBox box_;
    GmxHdf5MdParticlesProperties x_;
    GmxHdf5MdParticlesProperties v_;
    GmxHdf5MdParticlesProperties f_;
public:
    GmxHdf5MdIo();

    /*! Construct a GmxHdf5MdIo object and open a GmxHdf5 file.
     *
     * \param[in] fileName    Name of the file to open. The same as the file path.
     * \param[in] modeString  The mode to open the file, described by a case-insensitive string of
     *                        letters, up to three characters long. Reading is always assumed.
     *                        'w' means writing,
     *                        't' means truncate, i.e., that existing files will be overwritten
     *                        'e' results in a failure if the file already exists.
     *                        All these modes can be combined.
     */
    GmxHdf5MdIo(const char* fileName, const char* modeString);

    ~GmxHdf5MdIo();

    /*! Open an GmxHdf5 file.
     *
     * \param[in] fileName    Name of the file to open. The same as the file path.
     * \param[in] modeString  The mode to open the file, described by a case-insensitive string of
     *                        letters, up to three characters long. Reading is always assumed.
     *                        'w' means writing,
     *                        't' means truncate, i.e., that existing files will be overwritten
     *                        'e' results in a failure if the file already exists.
     *                        All these modes can be combined.
     */
    void openFile(const char* fileName, const char* modeString);

    void closeFile();

    void flush();

    void setupMolecularSystem(const gmx_mtop_t& topology);

    void writeFrame(int64_t          step,
                    real             time,
                    real             lambda,
                    const rvec*      box,
                    int64_t          natoms,
                    const rvec*      x,
                    const rvec*      v,
                    const rvec*      f);
};

#endif // GMX_FILEIO_HDF5MDIO_H