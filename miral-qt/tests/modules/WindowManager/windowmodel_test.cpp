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
#include "Unity/Application/mirsurface.h"
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

    miral::WindowInfo createMirALWindowInfo(int width = 200, int height = 300)
    {
        const miral::Application app{stubSession};
        const miral::Window window{app, stubSurface};

        ms::SurfaceCreationParameters windowSpec;
        windowSpec.of_size(Size{Width{width}, Height{height}});
        return miral::WindowInfo{window, windowSpec};
    }

    MirSurface *getMirSurfaceFromModel(const WindowModel &model, int index)
    {
        return model.data(model.index(index, 0), WindowModel::SurfaceRole).value<MirSurface*>();
    }

    miral::Window getMirALWindowFromModel(const WindowModel &model, int index)
    {
        return getMirSurfaceFromModel(model, index)->windowInfo().window;
    }

    const std::shared_ptr<StubSession> stubSession{std::make_shared<StubSession>()};
    const std::shared_ptr<StubSurface> stubSurface{std::make_shared<StubSurface>()};
};

/*
 * Test: that the WindowModelNotifier.addWindow causes the Qt-side WindowModel to
 * add a new Window, and emit the countChanged signal.
 */
TEST_F(WindowModelTest, AddWindowSucceeds)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();

    QSignalSpy spyCountChanged(&model, SIGNAL(countChanged()));

    notifier.addWindow(mirWindowInfo);

    ASSERT_EQ(1, model.count());
    EXPECT_EQ(1, spyCountChanged.count());
}

/*
 * Test: that the WindowModelNotifier.removeWindow causes the Qt-side WindowModel to
 * remove the Window from the model, and emit the countChanged signal.
 */
TEST_F(WindowModelTest, RemoveWindowSucceeds)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo);

    // Test removing the window
    QSignalSpy spyCountChanged(&model, SIGNAL(countChanged()));

    notifier.removeWindow(mirWindowInfo);

    ASSERT_EQ(0, model.count());
    EXPECT_EQ(1, spyCountChanged.count());
}

/*
 * Test: that calling WindowModelNotifier.addWindow causes Qt-side WindowModel to
 * have 2 windows in the correct order.
 */
TEST_F(WindowModelTest, Add2Windows)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    ASSERT_EQ(2, model.count());
    auto miralWindow1 = getMirALWindowFromModel(model, 0);
    ASSERT_EQ(mirWindowInfo1.window(), miralWindow1);
    auto miralWindow2 = getMirALWindowFromModel(model, 1);
    ASSERT_EQ(mirWindowInfo2.window(), miralWindow2);
}

/*
 * Test: that adding 2 windows, then removing the second, leaves the first.
 */
TEST_F(WindowModelTest, Add2WindowsAndRemoveSecondPreservesFirst)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    // Remove second window
    notifier.removeWindow(mirWindowInfo2);

    ASSERT_EQ(1, model.count());
    auto miralWindow = getMirALWindowFromModel(model, 0);
    ASSERT_EQ(mirWindowInfo1.window(), miralWindow);
}

/*
 * Test: that adding 2 windows, then removing the first, leaves the second.
 */
TEST_F(WindowModelTest, Add2WindowsAndRemoveFirstPreservesSecond)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    // Remove first window
    notifier.removeWindow(mirWindowInfo1);

    ASSERT_EQ(1, model.count());
    auto miralWindow = getMirALWindowFromModel(model, 0);
    ASSERT_EQ(mirWindowInfo2.window(), miralWindow);
}
