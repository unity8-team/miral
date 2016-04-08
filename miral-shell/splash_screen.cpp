/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alan Griffiths <alan@octopull.co.uk>
 */

#include "splash_screen.h"

#include <mir_toolkit/mir_client_library.h>

#include <chrono>

static void render_pattern(MirGraphicsRegion *region, uint32_t pf)
{
    char *row = region->vaddr;
    int j;

    for (j = 0; j < region->height; j++)
    {
        int i;
        uint32_t *pixel = (uint32_t*)row;

        for (i = 0; i < region->width; i++)
        {
            pixel[i] = pf;
        }

        row += region->stride;
    }
}

static MirPixelFormat find_8888_format(MirPixelFormat *formats, unsigned int num_formats)
{
    MirPixelFormat pf = mir_pixel_format_invalid;

    for (unsigned int i = 0; i < num_formats; ++i)
    {
        MirPixelFormat cur_pf = formats[i];
        if (cur_pf == mir_pixel_format_abgr_8888 ||
            cur_pf == mir_pixel_format_xbgr_8888 ||
            cur_pf == mir_pixel_format_argb_8888 ||
            cur_pf == mir_pixel_format_xrgb_8888)
        {
            pf = cur_pf;
            break;
        }
    }

    return pf;
}

static void fill_pattern(uint32_t pattern[2], MirPixelFormat pf)
{
    switch(pf)
    {
    case mir_pixel_format_abgr_8888:
    case mir_pixel_format_xbgr_8888:
        pattern[0] = 0xFF00FF00;
        pattern[1] = 0xFFFF0000;
        break;

    case mir_pixel_format_argb_8888:
    case mir_pixel_format_xrgb_8888:
        pattern[0] = 0xFF00FF00;
        pattern[1] = 0xFF0000FF;
        break;

    default:
        break;
    };
}

void splash_screen(MirConnection* connection)
{
    int swapinterval = 1;

    unsigned int const num_formats = 32;
    MirPixelFormat pixel_formats[num_formats];
    unsigned int valid_formats;
    mir_connection_get_available_surface_formats(connection, pixel_formats, num_formats, &valid_formats);
    MirPixelFormat pixel_format = find_8888_format(pixel_formats, valid_formats);

    MirSurfaceSpec *spec =
        mir_connection_create_spec_for_normal_surface(connection, 640, 480, pixel_format);

    mir_surface_spec_set_name(spec, __FILE__);
    mir_surface_spec_set_buffer_usage(spec, mir_buffer_usage_software);

    auto surface = mir_surface_create_sync(spec);
    mir_surface_spec_release(spec);


    mir_surface_set_swapinterval(surface, swapinterval);

    uint32_t pattern[2] = {0};
    fill_pattern(pattern, pixel_format);

    MirGraphicsRegion graphics_region;
    int i=0;
    MirBufferStream *bs = mir_surface_get_buffer_stream(surface);

    auto const time_limit = std::chrono::steady_clock::now() + std::chrono::seconds(3);

    while (std::chrono::steady_clock::now() < time_limit)
    {
        mir_buffer_stream_get_graphics_region(bs, &graphics_region);
        i++;
        render_pattern(&graphics_region, pattern[i & 1]);
        mir_buffer_stream_swap_buffers_sync(bs);
    }

    mir_surface_release_sync(surface);
}
