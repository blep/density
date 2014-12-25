/*
 * Centaurean Density
 *
 * Copyright (c) 2013, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Centaurean nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 23/12/14 17:01
 */

#include "teleport_input.h"

DENSITY_FORCE_INLINE density_teleport_input *density_teleport_input_allocate(uint_fast64_t size) {
    density_teleport_input *teleport = (density_teleport_input *) malloc(sizeof(density_teleport_input));
    teleport->source = DENSITY_TELEPORT_INPUT_SOURCE_DIRECT_ACCESS;
    teleport->stagingMemoryLocation = (density_staging_memory_location *) malloc(sizeof(density_staging_memory_location));
    teleport->stagingMemoryLocation->pointer = (density_byte *) malloc(DENSITY_TELEPORT_INPUT_BUFFER_SIZE * sizeof(density_byte));
    teleport->indirectMemoryLocation = (density_memory_location *) malloc(sizeof(density_memory_location));
    return teleport;
}

DENSITY_FORCE_INLINE void density_teleport_input_free(density_teleport_input *teleport) {
    free(teleport->indirectMemoryLocation);
    free(teleport->stagingMemoryLocation->pointer);
    free(teleport->stagingMemoryLocation);
    free(teleport);
}

DENSITY_FORCE_INLINE void density_teleport_input_store(density_teleport_input *restrict teleport, density_memory_location *restrict data) {
    teleport->directMemoryLocation = data;
}

DENSITY_FORCE_INLINE density_memory_location *density_teleport_input_access(density_teleport_input *restrict teleport, uint_fast64_t bytes) {
    uint_fast64_t missingBytes;
    switch (teleport->source) {
        case
            DENSITY_TELEPORT_INPUT_SOURCE_INDIRECT_ACCESS:
            missingBytes = bytes - teleport->stagingMemoryLocation->position;
            if (teleport->directMemoryLocation->available_bytes >= missingBytes) {
                memcpy(teleport->stagingMemoryLocation->pointer + teleport->stagingMemoryLocation->position, teleport->directMemoryLocation->pointer, missingBytes);
                teleport->indirectMemoryLocation->pointer = teleport->stagingMemoryLocation->pointer;
                teleport->indirectMemoryLocation->available_bytes = bytes;
                teleport->source = DENSITY_TELEPORT_INPUT_SOURCE_DIRECT_ACCESS;
                return teleport->indirectMemoryLocation;
            } else {
                memcpy(teleport->stagingMemoryLocation->pointer + teleport->stagingMemoryLocation->position, teleport->directMemoryLocation->pointer, teleport->directMemoryLocation->available_bytes);
                teleport->stagingMemoryLocation->position += teleport->directMemoryLocation->available_bytes;
                return NULL;
            }
        case
            DENSITY_TELEPORT_INPUT_SOURCE_DIRECT_ACCESS:
            if (teleport->directMemoryLocation->available_bytes >= bytes)
                return teleport->directMemoryLocation;
            else {
                memcpy(teleport->stagingMemoryLocation->pointer, teleport->directMemoryLocation->pointer, teleport->directMemoryLocation->available_bytes);
                teleport->stagingMemoryLocation->position += teleport->directMemoryLocation->available_bytes;
                teleport->source = DENSITY_TELEPORT_INPUT_SOURCE_INDIRECT_ACCESS;
                return NULL;
            }
    }
}