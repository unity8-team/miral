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

#include "qtmir_test.h"

#include <QLoggingCategory>
#include <QTest>
#include <QSignalSpy>

#include "mirqtconversion.h"
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


class WindowModelTest : public ::testing::Test
{
public:
    WindowModelTest()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }

    miral::WindowInfo createMirALWindowInfo(QPoint position = {160, 320}, QSize size = {100, 200})
    {
        const miral::Application app{stubSession};
        const miral::Window window{app, stubSurface};

        ms::SurfaceCreationParameters windowSpec;
        windowSpec.of_size(toMirSize(size));
        windowSpec.of_position(toMirPoint(position));
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

/*
 * Test: add 2 windows, remove first, add another window - ensure model order correct
 */
TEST_F(WindowModelTest, Add2WindowsRemoveFirstAddAnotherResultsInCorrectModel)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    auto mirWindowInfo3 = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);
    notifier.removeWindow(mirWindowInfo1);

    notifier.addWindow(mirWindowInfo3);

    ASSERT_EQ(2, model.count());
    auto miralWindow2 = getMirALWindowFromModel(model, 0);
    ASSERT_EQ(mirWindowInfo2.window(), miralWindow2);
    auto miralWindow3 = getMirALWindowFromModel(model, 1);
    ASSERT_EQ(mirWindowInfo3.window(), miralWindow3);
}

/*
 * Test: add 3 windows, remove second - ensure model order correct
 */
TEST_F(WindowModelTest, Add3WindowsRemoveSecondResultsInCorrectModel)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    auto mirWindowInfo3 = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);
    notifier.addWindow(mirWindowInfo3);

    notifier.removeWindow(mirWindowInfo2);

    ASSERT_EQ(2, model.count());
    auto miralWindow1 = getMirALWindowFromModel(model, 0);
    ASSERT_EQ(mirWindowInfo1.window(), miralWindow1);
    auto miralWindow3 = getMirALWindowFromModel(model, 1);
    ASSERT_EQ(mirWindowInfo3.window(), miralWindow3);
}

/*
 * Test: with 1 window, raise does nothing
 */
TEST_F(WindowModelTest, Raising1WindowDoesNothing)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);

    // Raise first window
    notifier.raiseWindows({mirWindowInfo1.window()});

    ASSERT_EQ(1, model.count());
    auto topWindow = getMirALWindowFromModel(model, 0);
    ASSERT_EQ(mirWindowInfo1.window(), topWindow);
}

/*
 * Test: with 2 window, raising top window does nothing
 */
TEST_F(WindowModelTest, RaisingTopWindowDoesNothing)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    // Raise second window (currently on top)
    notifier.raiseWindows({mirWindowInfo2.window()});

    // Check second window still on top
    ASSERT_EQ(2, model.count());
    auto topWindow = getMirALWindowFromModel(model, 1);
    ASSERT_EQ(mirWindowInfo2.window(), topWindow);
}

/*
 * Test: with 2 window, raising bottom window brings it to the top
 */
TEST_F(WindowModelTest, DISABLED_RaisingBottomWindowBringsItToTheTop)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    // Raise first window (currently at bottom)
    notifier.raiseWindows({mirWindowInfo1.window()});

    // Check first window now on top
    ASSERT_EQ(2, model.count());
    auto topWindow = getMirALWindowFromModel(model, 1);
    ASSERT_EQ(mirWindowInfo1.window(), topWindow);
}

/*
 * Test: MirSurface has inital position set correctly from miral::WindowInfo
 */
TEST_F(WindowModelTest, DISABLED_MirSurfacePositionSetCorrectlyAtCreation)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QPoint position(100, 200);

    auto mirWindowInfo = createMirALWindowInfo(position);
    notifier.addWindow(mirWindowInfo);

    auto surface = getMirSurfaceFromModel(model, 0);
    ASSERT_EQ(position, surface->position());
}

/*
 * Test: Mir moving a window updates MirSurface position
 */
TEST_F(WindowModelTest, WindowMoveUpdatesMirSurface)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QPoint oldPosition(100, 200),
           newPosition(150, 220);

    auto mirWindowInfo = createMirALWindowInfo(oldPosition);
    notifier.addWindow(mirWindowInfo);

    auto surface = getMirSurfaceFromModel(model, 0);

    // Move window, check new position set
    notifier.moveWindow(mirWindowInfo, toMirPoint(newPosition));

    ASSERT_EQ(newPosition, surface->position());
}

/*
 * Test: with 2 windows, ensure window move impacts the correct MirSurface
 */
TEST_F(WindowModelTest, WindowMoveUpdatesCorrectMirSurface)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QPoint oldPosition(100, 200),
           newPosition(150, 220);

    auto mirWindowInfo1 = createMirALWindowInfo(oldPosition);
    auto mirWindowInfo2 = createMirALWindowInfo(QPoint(300, 400));
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    auto surface = getMirSurfaceFromModel(model, 0); // will be MirSurface for mirWindowInfo1

    // Move window, check new position set
    notifier.moveWindow(mirWindowInfo1, toMirPoint(newPosition));

    ASSERT_EQ(newPosition, surface->position());
}

/*
 * Test: with 2 windows, ensure window move does not impact other MirSurfaces
 */
TEST_F(WindowModelTest, DISABLED_WindowMoveDoesNotTouchOtherMirSurfaces)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QPoint fixedPosition(300, 400);

    auto mirWindowInfo1 = createMirALWindowInfo(QPoint(100, 200));
    auto mirWindowInfo2 = createMirALWindowInfo(fixedPosition);
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    auto surface = getMirSurfaceFromModel(model, 1); // will be MirSurface for mirWindowInfo2

    // Move window, check new position set
    notifier.moveWindow(mirWindowInfo1, toMirPoint(QPoint(350, 420)));

    // Ensure other window untouched
    ASSERT_EQ(fixedPosition, surface->position());
}

/*
 * Test: MirSurface has inital size set correctly from miral::WindowInfo
 */
TEST_F(WindowModelTest, DISABLED_MirSurfaceSizeSetCorrectlyAtCreation)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QSize size(300, 200);

    auto mirWindowInfo1 = createMirALWindowInfo(QPoint(), size);
    notifier.addWindow(mirWindowInfo1);

    auto surface = getMirSurfaceFromModel(model, 0);
    ASSERT_EQ(size, surface->size());
}

/*
 * Test: Mir resizing a window updates MirSurface size
 */
TEST_F(WindowModelTest, WindowResizeUpdatesMirSurface)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QSize newSize(150, 220);

    auto mirWindowInfo1 = createMirALWindowInfo(QPoint(), QSize(300, 200));
    notifier.addWindow(mirWindowInfo1);

    auto surface = getMirSurfaceFromModel(model, 0);

    // Move window, check new position set
    notifier.resizeWindow(mirWindowInfo1, toMirSize(newSize));

    ASSERT_EQ(newSize, surface->size());
}

/*
 * Test: with 2 windows, ensure window resize impacts the correct MirSurface
 */
TEST_F(WindowModelTest, WindowResizeUpdatesCorrectMirSurface)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QSize newSize(150, 220);

    auto mirWindowInfo1 = createMirALWindowInfo(QPoint(), QSize(100, 200));
    auto mirWindowInfo2 = createMirALWindowInfo(QPoint(), QSize(300, 400));
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    auto surface = getMirSurfaceFromModel(model, 0);

    // Move window, check new position set
    notifier.resizeWindow(mirWindowInfo1, toMirSize(newSize));

    ASSERT_EQ(newSize, surface->size());
}

/*
 * Test: with 2 windows, ensure window resize does not impact other MirSurfaces
 */
TEST_F(WindowModelTest, DISABLED_WindowResizeDoesNotTouchOtherMirSurfaces)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    QSize fixedPosition(300, 400);

    auto mirWindowInfo1 = createMirALWindowInfo(QPoint(), QSize(100, 200));
    auto mirWindowInfo2 = createMirALWindowInfo(QPoint(), fixedPosition);
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    auto surface = getMirSurfaceFromModel(model, 1);

    // Move window
    notifier.resizeWindow(mirWindowInfo1, toMirSize(QSize(150, 220)));

    // Ensure other window untouched
    ASSERT_EQ(fixedPosition, surface->size());
}
