/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include <QLoggingCategory>
#include <QTest>
#include <QSignalSpy>

#include "windowmodelnotifier.h"
#include "Unity/Application/windowmodel.h"

#include "stub_windowmodelnotifier.h"

#include <mir/test/doubles/stub_surface.h>
#include <mir/test/doubles/stub_session.h>

#include "mirserver/mir/scene/surface_creation_parameters.h"

using namespace qtmir;

namespace ms = mir::scene;
namespace mg = mir::graphics;
namespace geom = mir::geometry;
using StubSurface = mir::test::doubles::StubSurface;
using StubSession = mir::test::doubles::StubSession;
using namespace testing;


class WindowModelTest : public ::testing::Test
{
public:
    WindowModelTest()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }

    std::shared_ptr<StubSession> const stubSession{std::make_shared<StubSession>()};
    std::shared_ptr<StubSurface> const stubSurface{std::make_shared<StubSurface>()};
    miral::Application app{stubSession};
    miral::Window windowA{app, stubSurface};

    StubWindowModelNotifier m_notifier;
};

TEST_F(WindowModelTest, AddWindowSucceeds)
{
    WindowModel model(&m_notifier, nullptr); // no need for controller in this testcase

    ms::SurfaceCreationParameters windowASpec;
    windowASpec.of_size(geom::Size{geom::Width{1024}, geom::Height{768}});
    miral::WindowInfo mirWindowInfo{windowA, windowASpec};

    WindowInfo window{mirWindowInfo};

    QSignalSpy spyCountChanged(&model, SIGNAL(countChanged()));

    m_notifier.emitWindowAdded(window, 0);

    ASSERT_EQ(model.count(), 1);
    EXPECT_EQ(spyCountChanged.count(), 1);
}
