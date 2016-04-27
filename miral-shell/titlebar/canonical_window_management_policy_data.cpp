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

#include "canonical_window_management_policy_data.h"

// TODO We need a better way to support painting stuff inside the window manager
#include <mir/frontend/buffer_stream.h>
#include <mir/graphics/buffer.h>
#include <mir/graphics/buffer_properties.h>
#include <mir/scene/surface.h>

#include <atomic>

using namespace mir::geometry;

struct CanonicalWindowManagementPolicyData::StreamPainter
{
    virtual void paint(int) = 0;
    virtual ~StreamPainter() = default;
    StreamPainter() = default;
    StreamPainter(StreamPainter const&) = delete;
    StreamPainter& operator=(StreamPainter const&) = delete;
};

struct CanonicalWindowManagementPolicyData::SwappingPainter
    : CanonicalWindowManagementPolicyData::StreamPainter
{
    SwappingPainter(std::shared_ptr<mir::frontend::BufferStream> const& buffer_stream) :
        buffer_stream{buffer_stream}, buffer{nullptr}
    {
        swap_buffers(nullptr);
    }

    void swap_buffers(mir::graphics::Buffer* buf)
    {
        auto const callback = [this](mir::graphics::Buffer* new_buffer)
            {
                buffer.store(new_buffer);
            };

        buffer_stream->swap_buffers(buf, callback);
    }

    void paint(int intensity) override
    {
        if (auto const buf = buffer.exchange(nullptr))
        {
            auto const format = buffer_stream->pixel_format();
            auto const sz = buf->size().height.as_int() *
                            buf->size().width.as_int() * MIR_BYTES_PER_PIXEL(format);
            std::vector<unsigned char> pixels(sz, intensity);
            buf->write(pixels.data(), sz);
            swap_buffers(buf);
        }
    }

    std::shared_ptr<mir::frontend::BufferStream> const buffer_stream;
    std::atomic<mir::graphics::Buffer*> buffer;
};

struct CanonicalWindowManagementPolicyData::AllocatingPainter
    : CanonicalWindowManagementPolicyData::StreamPainter
{
    AllocatingPainter(std::shared_ptr<mir::frontend::BufferStream> const& buffer_stream, Size size) :
        buffer_stream(buffer_stream),
        properties({
                       size,
                       buffer_stream->pixel_format(),
                       mir::graphics::BufferUsage::software
                   }),
        front_buffer(buffer_stream->allocate_buffer(properties)),
        back_buffer(buffer_stream->allocate_buffer(properties))
    {
    }

    void paint(int intensity) override
    {
        buffer_stream->with_buffer(back_buffer,
            [this, intensity](mir::graphics::Buffer& buffer)
            {
                auto const format = buffer.pixel_format();
                auto const sz = buffer.size().height.as_int() * buffer.size().width.as_int() * MIR_BYTES_PER_PIXEL(format);
                std::vector<unsigned char> pixels(sz, intensity);
                buffer.write(pixels.data(), sz);
                buffer_stream->swap_buffers(&buffer, [](mir::graphics::Buffer*){});
            });
        std::swap(front_buffer, back_buffer);
    }

    ~AllocatingPainter()
    {
        buffer_stream->remove_buffer(front_buffer);
        buffer_stream->remove_buffer(back_buffer);
    }

    std::shared_ptr<mir::frontend::BufferStream> const buffer_stream;
    mir::graphics::BufferProperties properties;
    mir::graphics::BufferID front_buffer;
    mir::graphics::BufferID back_buffer;
};

void CanonicalWindowManagementPolicyData::paint_titlebar(int intensity)
{
    if (!stream_painter)
    {
        auto stream = std::shared_ptr<mir::scene::Surface>(window)->primary_buffer_stream();
        try
        {
            stream_painter = std::make_shared<AllocatingPainter>(stream, window.size());
        }
        catch (...)
        {
            stream_painter = std::make_shared<SwappingPainter>(stream);
        }
    }

    stream_painter->paint(intensity);
}
