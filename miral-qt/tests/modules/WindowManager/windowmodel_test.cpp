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

#include <mir/test/doubles/stub_surface.h>
#include <mir/test/doubles/stub_session.h>

#include <mir/scene/surface_creation_parameters.h>

using namespace qtmir;

namespace ms = mir::scene;
namespace mg = mir::graphics;
using StubSurface = mir::test::doubles::StubSurface;
using StubSession = mir::test::doubles::StubSession;
using namespace testing;
using namespace mir::geometry;


class WindowModelTest : public ::testing::Test
{
public:
    WindowModelTest()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }

    const std::shared_ptr<StubSession> stubSession{std::make_shared<StubSession>()};
    const std::shared_ptr<StubSurface> stubSurface{std::make_shared<StubSurface>()};
    const miral::Application app{stubSession};
    const miral::Window windowA{app, stubSurface};

    WindowModelNotifier m_notifier;
};

TEST_F(WindowModelTest, AddWindowSucceeds)
{
    WindowModel model(&m_notifier, nullptr); // no need for controller in this testcase

    ms::SurfaceCreationParameters windowASpec;
    windowASpec.of_size(Size{Width{1024}, Height{768}});
    miral::WindowInfo mirWindowInfo{windowA, windowASpec};

    QSignalSpy spyCountChanged(&model, SIGNAL(countChanged()));

    m_notifier.addWindow(mirWindowInfo);

    ASSERT_EQ(model.count(), 1);
    EXPECT_EQ(spyCountChanged.count(), 1);
}
