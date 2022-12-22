/*
 *  $Id: container-chunk.h,v 1.8 2003/05/29 14:24:02 bootc Exp $
 *  libdaap: container-chunk.h
 *
 *  Created by Chris Boot on Mon May 05 2003.
 *  Copyright (C) 2003 Chris Boot
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __DAAP_CONTAINER_CHUNK_H__
#define __DAAP_CONTAINER_CHUNK_H__

#include <libdaap/chunk.h>
#include <iostream>
#include <vector>

namespace DAAP {

class ContainerChunk : public Chunk
{
    public:
        ContainerChunk(
            const char *           inName,
            const ContentCodes *   inContentCodes = NULL);

        ContainerChunk(
            ChunkType              inType,
            uint32_t               inLength,
            const void *           inData,
            const ContentCodes *   inContentCodes);

        virtual
        ~ContainerChunk();

        size_t
        CountChildren() const;

        const Chunk &
        operator[](size_t inIndex) const;

        Chunk &
        operator[](size_t inIndex);

        const Chunk &
        ChildNamed(const char *inName) const;

        Chunk &
        ChildNamed(const char *inName);

        virtual void
        Display(
            std::ostream &   inStream,
            int              inLevel = 0) const;

        // Allows server side to add a chunk into the list of children.
        void
        AddChild(Chunk *inChild);

        /*
         *  public ContainerChunk::GetLength()
         *
         *  Discussion:
         *    Returns the length of the data contained within this chunk.
         *
         *  Result:
         *    The number of bytes that would be returned by
         *    Chunk::GetData().
         */
        virtual uint32_t
        GetLength() const;

        /*
         *  public ContainerChunk::GetData()
         *
         *  Discussion:
         *    Copies the data contained in this chunk into a buffer.
         *
         *  Parameters:
         *
         *    inLength:
         *      The length of the buffer.  If the length is smaller than
         *      GetLength(),
         *
         *    outBuffer:
         *      The buffer to copy the data into.only inLength bytes will be copied into the
         *      buffer.
         *
         *  Result:
         *    The number of bytes copied into the buffer which will be equal
         *    to the smaller of inLength and GetLength().
         */
        virtual uint32_t
        GetData(
            uint32_t   inLength,
            void *     outBuffer) const;

        virtual const char*
        GetStringValue(const char* key) const;

    private:
        std::vector<Chunk *>   mChildren;
};

} /* namespace DAAP */

#endif /* __DAAP_CONTAINER_CHUNK_H__ */
