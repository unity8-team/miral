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

#include <mir/shell/surface_specification.h>
#include <mir/scene/surface_creation_parameters.h>
#include "miral/window_specification.h"

struct miral::WindowSpecification::Self
{
    Self() = default;
    Self(Self const&) = default;
    Self(mir::shell::SurfaceSpecification const& spec);
    Self(mir::scene::SurfaceCreationParameters const& params);
    void update(mir::shell::SurfaceSpecification& spec) const;
    void update(mir::scene::SurfaceCreationParameters& params) const;

    mir::optional_value<Point> top_left;
    mir::optional_value<Size> size;
    mir::optional_value<MirPixelFormat> pixel_format;
    mir::optional_value<BufferUsage> buffer_usage;
    mir::optional_value<std::string> name;
    mir::optional_value<int> output_id;
    mir::optional_value<MirSurfaceType> type;
    mir::optional_value<MirSurfaceState> state;
    mir::optional_value<MirOrientationMode> preferred_orientation;
    mir::optional_value<Rectangle> aux_rect;
    mir::optional_value<MirEdgeAttachment> edge_attachment;
    mir::optional_value<Width> min_width;
    mir::optional_value<Height> min_height;
    mir::optional_value<Width> max_width;
    mir::optional_value<Height> max_height;
    mir::optional_value<DeltaX> width_inc;
    mir::optional_value<DeltaY> height_inc;
    mir::optional_value<AspectRatio> min_aspect;
    mir::optional_value<AspectRatio> max_aspect;
    mir::optional_value<std::vector<StreamSpecification>> streams;
    mir::optional_value<std::weak_ptr<mir::scene::Surface>> parent;
    mir::optional_value<std::vector<Rectangle>> input_shape;
    mir::optional_value<InputReceptionMode> input_mode;
    mir::optional_value<MirShellChrome> shell_chrome;
};

miral::WindowSpecification::Self::Self(mir::shell::SurfaceSpecification const& spec) :
    top_left(),
    size(),
    pixel_format(spec.pixel_format),
    buffer_usage(),
    name(spec.name),
    output_id(),
    type(spec.type),
    state(spec.state),
    preferred_orientation(spec.preferred_orientation),
    aux_rect(spec.aux_rect),
    edge_attachment(spec.edge_attachment),
    min_width(spec.min_width),
    min_height(spec.min_height),
    max_width(spec.max_width),
    max_height(spec.max_height),
    width_inc(spec.width_inc),
    height_inc(spec.height_inc),
    min_aspect(),
    max_aspect(),
    streams(),
    parent(spec.parent),
    input_shape(spec.input_shape),
    input_mode(),
    shell_chrome()
{
    if (spec.width.is_set() && spec.height.is_set())
        size = Size(spec.width.value(), spec.height.value());

    if (spec.buffer_usage.is_set())
        buffer_usage = BufferUsage(spec.buffer_usage.value());

    if (spec.output_id.is_set())
        output_id = spec.output_id.value().as_value();

    if (spec.min_aspect.is_set())
        min_aspect = AspectRatio{spec.min_aspect.value().width, spec.min_aspect.value().height};

    if (spec.max_aspect.is_set())
        max_aspect = AspectRatio{spec.max_aspect.value().width, spec.max_aspect.value().height};

    if (spec.streams.is_set())
    {
        auto const& source = spec.streams.value();
        std::vector<StreamSpecification> dest;
        dest.reserve(source.size());

        for (auto const& stream : source)
            dest.push_back(StreamSpecification{BufferStreamId{stream.stream_id.as_value()}, stream.displacement});
    }
}

namespace
{
template<typename Dest, typename Source>
void copy_if_set(Dest& dest, mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = source.value();
}

template<typename Dest, typename Source>
void copy_if_set(mir::optional_value<Dest>& dest, mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = source;
}

template<typename Source>
void copy_if_set(mir::optional_value<mir::graphics::BufferUsage>& dest, mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = static_cast<mir::graphics::BufferUsage>(source.value());
}

template<typename Source>
void copy_if_set(mir::graphics::BufferUsage& dest, mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = static_cast<mir::graphics::BufferUsage>(source.value());
}

template<typename Source>
void copy_if_set(
    mir::optional_value<mir::graphics::DisplayConfigurationOutputId>& dest,
    mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = static_cast<mir::graphics::DisplayConfigurationOutputId>(source.value());
}

template<typename Source>
void copy_if_set(
    mir::graphics::DisplayConfigurationOutputId& dest,
    mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = static_cast<mir::graphics::DisplayConfigurationOutputId>(source.value());
}

template<typename Source>
void copy_if_set(
    mir::optional_value<mir::input::InputReceptionMode>& dest,
    mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = static_cast<mir::input::InputReceptionMode>(source.value());
}

template<typename Source>
void copy_if_set(
    mir::input::InputReceptionMode& dest,
    mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = static_cast<mir::input::InputReceptionMode>(source.value());
}

template<typename Source>
void copy_if_set(
    mir::optional_value<mir::shell::SurfaceAspectRatio>& dest,
    mir::optional_value<Source> const& source)
{
    if (source.is_set()) dest = mir::shell::SurfaceAspectRatio{source.value().width, source.value().height};
}
}

void miral::WindowSpecification::Self::update(mir::shell::SurfaceSpecification& spec) const
{
//     top_left
    
    if (size.is_set())
    {
        spec.width = size.value().width;
        spec.height = size.value().height;
    }

    copy_if_set(spec.pixel_format, pixel_format);
    copy_if_set(spec.buffer_usage, buffer_usage);
    copy_if_set(spec.name, name);
    copy_if_set(spec.output_id, output_id);
    copy_if_set(spec.type, type);
    copy_if_set(spec.state, state);
    copy_if_set(spec.preferred_orientation, preferred_orientation);
    copy_if_set(spec.aux_rect, aux_rect);
    copy_if_set(spec.edge_attachment, edge_attachment);
    copy_if_set(spec.min_width, min_width);
    copy_if_set(spec.min_height, min_height);
    copy_if_set(spec.max_width, max_width);
    copy_if_set(spec.max_height, max_height);
    copy_if_set(spec.width_inc, width_inc);
    copy_if_set(spec.height_inc, height_inc);
    copy_if_set(spec.min_aspect, min_aspect);
    copy_if_set(spec.max_aspect, max_aspect);

    if (streams.is_set())
    {
        auto const& source = streams.value();
        std::vector<mir::shell::StreamSpecification> dest;
        dest.reserve(source.size());

        for (auto const& stream : source)
            dest.push_back(mir::shell::StreamSpecification{mir::frontend::BufferStreamId{stream.stream_id.as_value()}, stream.displacement});

        spec.streams = std::move(dest);
    }

    copy_if_set(spec.parent, parent);
    copy_if_set(spec.input_shape, input_shape);

//    input_mode
//    shell_chrome
}

miral::WindowSpecification::Self::Self(mir::scene::SurfaceCreationParameters const& params) :
    top_left(params.top_left),
    size(params.size),
    pixel_format(params.pixel_format),
    buffer_usage(static_cast<BufferUsage>(params.buffer_usage)),
    name(params.name),
    output_id(params.output_id.as_value()),
    type(params.type),
    state(params.state),
    preferred_orientation(params.preferred_orientation),
    aux_rect(params.aux_rect),
    edge_attachment(params.edge_attachment),
    min_width(params.min_width),
    min_height(params.min_height),
    max_width(params.max_width),
    max_height(params.max_height),
    width_inc(params.width_inc),
    height_inc(params.height_inc),
    min_aspect(),
    max_aspect(),
    streams(),
    parent(params.parent),
    input_shape(params.input_shape),
    input_mode(static_cast<InputReceptionMode>(params.input_mode)),
    shell_chrome()
{
    if (params.min_aspect.is_set())
        min_aspect = AspectRatio{params.min_aspect.value().width, params.min_aspect.value().height};

    if (params.max_aspect.is_set())
        max_aspect = AspectRatio{params.max_aspect.value().width, params.max_aspect.value().height};
}

void miral::WindowSpecification::Self::update(mir::scene::SurfaceCreationParameters& params) const
{
    copy_if_set(params.top_left, top_left);
    copy_if_set(params.size, size);
    copy_if_set(params.pixel_format, pixel_format);
    copy_if_set(params.buffer_usage, buffer_usage);
    copy_if_set(params.name, name);
    copy_if_set(params.output_id, output_id);
    copy_if_set(params.type, type);
    copy_if_set(params.state, state);
    copy_if_set(params.preferred_orientation, preferred_orientation);
    copy_if_set(params.aux_rect, aux_rect);
    copy_if_set(params.edge_attachment, edge_attachment);
    copy_if_set(params.min_width, min_width);
    copy_if_set(params.min_height, min_height);
    copy_if_set(params.max_width, max_width);
    copy_if_set(params.max_height, max_height);
    copy_if_set(params.width_inc, width_inc);
    copy_if_set(params.height_inc, height_inc);
    copy_if_set(params.min_aspect, min_aspect);
    copy_if_set(params.max_aspect, max_aspect);
//    streams
    copy_if_set(params.parent, parent);
    copy_if_set(params.input_shape, input_shape);
    copy_if_set(params.input_mode, input_mode);
    copy_if_set(params.shell_chrome, shell_chrome);
}

miral::WindowSpecification::WindowSpecification() :
    self{std::make_unique<Self>()}
{
}

miral::WindowSpecification::WindowSpecification(mir::shell::SurfaceSpecification const& spec) :
    self{std::make_unique<Self>(spec)}
{
}

auto miral::WindowSpecification::operator=(mir::shell::SurfaceSpecification const& spec) -> WindowSpecification&
{
    self = std::make_unique<Self>(spec);
    return *this;
}

void miral::WindowSpecification::update(mir::shell::SurfaceSpecification& spec) const
{
    self->update(spec);
}

miral::WindowSpecification::WindowSpecification(mir::scene::SurfaceCreationParameters const& params) :
    self{std::make_unique<Self>(params)}
{
}

miral::WindowSpecification::WindowSpecification(WindowSpecification const& that) :
    self{std::make_unique<Self>(*that.self)}
{
}

auto miral::WindowSpecification::operator=(WindowSpecification const& that) -> WindowSpecification&
{
    self = std::make_unique<Self>(*that.self);
    return *this;
}

miral::WindowSpecification::~WindowSpecification() = default;

auto miral::WindowSpecification::operator=(mir::scene::SurfaceCreationParameters const& spec) -> WindowSpecification&
{
    self = std::make_unique<Self>(spec);
    return *this;
}

void miral::WindowSpecification::update(mir::scene::SurfaceCreationParameters& params) const
{
    self->update(params);
}

auto miral::WindowSpecification::top_left() const -> mir::optional_value<Point> const&
{
    return self->top_left;
}

auto miral::WindowSpecification::size() const -> mir::optional_value<Size> const&
{
    return self->size;
}

auto miral::WindowSpecification::pixel_format() const -> mir::optional_value<MirPixelFormat> const&
{
    return self->pixel_format;
}

auto miral::WindowSpecification::buffer_usage() const -> mir::optional_value<BufferUsage> const&
{
    return self->buffer_usage;
}

auto miral::WindowSpecification::name() const -> mir::optional_value<std::string> const&
{
    return self->name;
}

auto miral::WindowSpecification::output_id() const -> mir::optional_value<int> const&
{
    return self->output_id;
}

auto miral::WindowSpecification::type() const -> mir::optional_value<MirSurfaceType> const&
{
    return self->type;
}

auto miral::WindowSpecification::state() const -> mir::optional_value<MirSurfaceState> const&
{
    return self->state;
}

auto miral::WindowSpecification::preferred_orientation() const -> mir::optional_value<MirOrientationMode> const&
{
    return self->preferred_orientation;
}

auto miral::WindowSpecification::aux_rect() const -> mir::optional_value<Rectangle> const&
{
    return self->aux_rect;
}

auto miral::WindowSpecification::edge_attachment() const -> mir::optional_value<MirEdgeAttachment> const&
{
    return self->edge_attachment;
}

auto miral::WindowSpecification::min_width() const -> mir::optional_value<Width> const&
{
    return self->min_width;
}

auto miral::WindowSpecification::min_height() const -> mir::optional_value<Height> const&
{
    return self->min_height;
}

auto miral::WindowSpecification::max_width() const -> mir::optional_value<Width> const&
{
    return self->max_width;
}

auto miral::WindowSpecification::max_height() const -> mir::optional_value<Height> const&
{
    return self->max_height;
}

auto miral::WindowSpecification::width_inc() const -> mir::optional_value<DeltaX> const&
{
    return self->width_inc;
}

auto miral::WindowSpecification::height_inc() const -> mir::optional_value<DeltaY> const&
{
    return self->height_inc;
}

auto miral::WindowSpecification::min_aspect() const -> mir::optional_value<AspectRatio> const&
{
    return self->min_aspect;
}

auto miral::WindowSpecification::max_aspect() const -> mir::optional_value<AspectRatio> const&
{
    return self->max_aspect;
}

auto miral::WindowSpecification::streams() const -> mir::optional_value<std::vector<StreamSpecification>> const&
{
    return self->streams;
}

auto miral::WindowSpecification::parent() const -> mir::optional_value<std::weak_ptr<mir::scene::Surface>> const&
{
    return self->parent;
}

auto miral::WindowSpecification::input_shape() const -> mir::optional_value<std::vector<Rectangle>> const&
{
    return self->input_shape;
}

auto miral::WindowSpecification::input_mode() const -> mir::optional_value<InputReceptionMode> const&
{
    return self->input_mode;
}

auto miral::WindowSpecification::shell_chrome() const -> mir::optional_value<MirShellChrome> const&
{
    return self->shell_chrome;
}

auto miral::WindowSpecification::top_left() -> mir::optional_value<Point>&
{
    return self->top_left;
}

auto miral::WindowSpecification::size() -> mir::optional_value<Size>&
{
    return self->size;
}

auto miral::WindowSpecification::pixel_format() -> mir::optional_value<MirPixelFormat>&
{
    return self->pixel_format;
}

auto miral::WindowSpecification::buffer_usage() -> mir::optional_value<BufferUsage>&
{
    return self->buffer_usage;
}

auto miral::WindowSpecification::name() -> mir::optional_value<std::string>&
{
    return self->name;
}

auto miral::WindowSpecification::output_id() -> mir::optional_value<int>&
{
    return self->output_id;
}

auto miral::WindowSpecification::type() -> mir::optional_value<MirSurfaceType>&
{
    return self->type;
}

auto miral::WindowSpecification::state() -> mir::optional_value<MirSurfaceState>&
{
    return self->state;
}

auto miral::WindowSpecification::preferred_orientation() -> mir::optional_value<MirOrientationMode>&
{
    return self->preferred_orientation;
}

auto miral::WindowSpecification::aux_rect() -> mir::optional_value<Rectangle>&
{
    return self->aux_rect;
}

auto miral::WindowSpecification::edge_attachment() -> mir::optional_value<MirEdgeAttachment>&
{
    return self->edge_attachment;
}

auto miral::WindowSpecification::min_width() -> mir::optional_value<Width>&
{
    return self->min_width;
}

auto miral::WindowSpecification::min_height() -> mir::optional_value<Height>&
{
    return self->min_height;
}

auto miral::WindowSpecification::max_width() -> mir::optional_value<Width>&
{
    return self->max_width;
}

auto miral::WindowSpecification::max_height() -> mir::optional_value<Height>&
{
    return self->max_height;
}

auto miral::WindowSpecification::width_inc() -> mir::optional_value<DeltaX>&
{
    return self->width_inc;
}

auto miral::WindowSpecification::height_inc() -> mir::optional_value<DeltaY>&
{
    return self->height_inc;
}

auto miral::WindowSpecification::min_aspect() -> mir::optional_value<AspectRatio>&
{
    return self->min_aspect;
}

auto miral::WindowSpecification::max_aspect() -> mir::optional_value<AspectRatio>&
{
    return self->max_aspect;
}

auto miral::WindowSpecification::streams() -> mir::optional_value<std::vector<StreamSpecification>>&
{
    return self->streams;
}

auto miral::WindowSpecification::parent() -> mir::optional_value<std::weak_ptr<mir::scene::Surface>>&
{
    return self->parent;
}

auto miral::WindowSpecification::input_shape() -> mir::optional_value<std::vector<Rectangle>>&
{
    return self->input_shape;
}

auto miral::WindowSpecification::input_mode() -> mir::optional_value<InputReceptionMode>&
{
    return self->input_mode;
}

auto miral::WindowSpecification::shell_chrome() -> mir::optional_value<MirShellChrome>&
{
    return self->shell_chrome;
}
