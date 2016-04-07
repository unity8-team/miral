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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "miral/status_bar.h"

#include <mir/server.h>
#include <mir/graphics/display.h>
#include <mir/graphics/display_configuration.h>
#include <mir/graphics/buffer_properties.h>
#include <mir/graphics/buffer.h>
#include <mir/graphics/platform.h>
#include <mir/graphics/graphic_buffer_allocator.h>
#include <mir/scene/buffer_stream_factory.h>
#include <mir/scene/surface_factory.h>
#include <mir/shell/surface_stack.h>
#include <mir/input/input_reception_mode.h>
#include <mir/frontend/buffer_sink.h>
#include <mir/frontend/buffer_stream.h>

/* FIXME: need to be able to convert mc::BufferStream to mf::BufferStream....
 *        addressed in future NBS work in lp:~kdub/mir/mend-streams-server-api
 */
#include <../../mir/src/include/server/mir/compositor/buffer_stream.h>

using namespace mir::geometry;
namespace mg = mir::graphics;
namespace mf = mir::frontend;

namespace
{
class BufferSink : public mf::BufferSink
{
    void send_buffer(mf::BufferStreamId, mg::Buffer&, mg::BufferIpcMsgType) override {}
};
}

void miral::StatusBar::operator()(mir::Server& server)
{
    server.add_init_callback([&]
    {
        Size largest_size { 0, 0 }; 
        auto display_config = server.the_display()->configuration();
        display_config->for_each_output([&](auto config)
        {
            if ((config.connected) && 
                (config.modes[config.current_mode_index].size.width > largest_size.width))
                largest_size = config.modes[config.current_mode_index].size;
        });
        auto status_bar_size = Size{largest_size.width, largest_size.height.as_float() * 0.05};
        mg::BufferProperties properties(
            status_bar_size, mir_pixel_format_abgr_8888, mg::BufferUsage::software);
        //FIXME: see include fixme that we need a way to convert the stream.
        auto mc_stream = 
            server.the_buffer_stream_factory()->create_buffer_stream(
                {}, std::make_shared<BufferSink>(), properties);
        auto stream = std::dynamic_pointer_cast<mf::BufferStream>(mc_stream);
        //FIXME: Accessing buffers will be improved with NBS work in
        //       lp:~kdub/mir/session-allocated-buffers-server-api
        //       This currently will throw for OBS. 
        auto buffer_id = stream->allocate_buffer(properties);
        stream->with_buffer(buffer_id, [&](auto& buffer) {

            //FIXME: it is unreasonable to expect the user to alloc a full buffer, and then
            //       copy it, when most platforms will simply be mmap-ing memory that the
            //       user could write to instead of an intermediary buffer (esp in the case
            //       of >1080p!). Its also unreasonable that the vector has to be a specific size,
            //       and write() will throw to indicate that writing is not supported. 
            auto size = buffer.stride().as_int() * buffer.size().height.as_int();
            std::vector<unsigned char> pix(size);
            for(auto i = 0u; i < pix.size(); i++)
            {
                if (i % 2)
                    pix[i] = 0xFF;
            }
            buffer.write(pix.data(), pix.size());

            //FIXME: once NBS replaces OBS, this should be mf::BufferStream::submit_buffer(mg::Buffer*)
            stream->swap_buffers(&buffer, [](auto){});
        });

        auto surface_factory = server.the_surface_factory();
        auto surface = surface_factory->create_surface(mc_stream, mir::scene::a_surface());

        auto stack = server.the_surface_stack();
        stack->add_surface(surface, mir::input::InputReceptionMode::normal);
    });
}
