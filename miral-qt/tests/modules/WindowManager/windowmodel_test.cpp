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
 * increment model count
 */
TEST_F(WindowModelTest, WhenAddWindowNotifiedModelCountIncrements)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo);

    EXPECT_EQ(1, model.count());
}

/*
 * Test: that the WindowModelNotifier.addWindow causes the Qt-side WindowModel to
 * emit the countChanged signal.
 */
TEST_F(WindowModelTest, WhenAddWindowNotifiedModelEmitsCountChangedSignal)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();

    QSignalSpy spyCountChanged(&model, SIGNAL(countChanged()));

    notifier.addWindow(mirWindowInfo);

    EXPECT_EQ(1, spyCountChanged.count());
}

/*
 * Test: that the WindowModelNotifier.addWindow causes the Qt-side WindowModel to
 * gain an entry which has the correct miral::Window
 */
TEST_F(WindowModelTest, WhenAddWindowNotifiedNewModelEntryHasCorrectWindow)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo);

    auto miralWindow = getMirALWindowFromModel(model, 0);
    EXPECT_EQ(mirWindowInfo.window(), miralWindow);
}

/*
 * Test: that the WindowModelNotifier.removeWindow causes the Qt-side WindowModel to
 * remove the Window from the model, and emit the countChanged signal.
 */
TEST_F(WindowModelTest, WhenRemoveWindowNotifiedModelCountDecrements)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo);

    // Test removing the window
    notifier.removeWindow(mirWindowInfo);

    EXPECT_EQ(0, model.count());
}

/*
 * Test: that the WindowModelNotifier.removeWindow causes the Qt-side WindowModel to
 * emit the countChanged signal.
 */
TEST_F(WindowModelTest, WhenRemoveWindowNotifiedModelEmitsCountChangedSignal)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo);

    // Test removing the window
    QSignalSpy spyCountChanged(&model, SIGNAL(countChanged()));

    notifier.removeWindow(mirWindowInfo);

    EXPECT_EQ(1, spyCountChanged.count());
}

/*
 * Test: that calling WindowModelNotifier.addWindow causes Qt-side WindowModel to
 * have 2 windows in the correct order.
 */
TEST_F(WindowModelTest, WhenAddingTwoWindowsModelHasCorrectOrder)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();

    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    ASSERT_EQ(2, model.count());
    auto miralWindow1 = getMirALWindowFromModel(model, 0);
    EXPECT_EQ(mirWindowInfo1.window(), miralWindow1);
    auto miralWindow2 = getMirALWindowFromModel(model, 1);
    EXPECT_EQ(mirWindowInfo2.window(), miralWindow2);
}

/*
 * Test: that adding 2 windows, then removing the second, leaves the first.
 */
TEST_F(WindowModelTest, WhenAddingTwoWindowsAndRemoveSecondModelPreservesFirst)
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
    EXPECT_EQ(mirWindowInfo1.window(), miralWindow);
}

/*
 * Test: that adding 2 windows, then removing the first, leaves the second.
 */
TEST_F(WindowModelTest, WhenAddingTwoWindowsAndRemoveFirstModelPreservesSecond)
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
    EXPECT_EQ(mirWindowInfo2.window(), miralWindow);
}

/*
 * Test: add 2 windows, remove first, add another window - ensure model order correct
 */
TEST_F(WindowModelTest, WhenAddingTwoWindowsRemoveFirstAddAnotherResultsInCorrectModel)
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
    EXPECT_EQ(mirWindowInfo2.window(), miralWindow2);
    auto miralWindow3 = getMirALWindowFromModel(model, 1);
    EXPECT_EQ(mirWindowInfo3.window(), miralWindow3);
}

/*
 * Test: add 3 windows, remove second - ensure model order correct
 */
TEST_F(WindowModelTest, WhenAddingThreeWindowsRemoveSecondResultsInCorrectModel)
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
    EXPECT_EQ(mirWindowInfo1.window(), miralWindow1);
    auto miralWindow3 = getMirALWindowFromModel(model, 1);
    EXPECT_EQ(mirWindowInfo3.window(), miralWindow3);
}

/*
 * Test: with 1 window, raise does nothing
 */
TEST_F(WindowModelTest, RaisingOneWindowDoesNothing)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);

    // Raise first window
    notifier.raiseWindows({mirWindowInfo1.window()});

    ASSERT_EQ(1, model.count());
    auto topWindow = getMirALWindowFromModel(model, 0);
    EXPECT_EQ(mirWindowInfo1.window(), topWindow);
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
    EXPECT_EQ(mirWindowInfo2.window(), topWindow);
}

/*
 * Test: with 2 window, raising bottom window brings it to the top
 */
TEST_F(WindowModelTest, RaisingBottomWindowBringsItToTheTop)
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
    EXPECT_EQ(mirWindowInfo1.window(), topWindow);
}

/*
 * Test: with 3 windows, raising bottom 2 windows brings them to the top in order
 */
TEST_F(WindowModelTest, Raising2BottomWindowsBringsThemToTheTop)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    auto mirWindowInfo3 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);
    notifier.addWindow(mirWindowInfo3);

    // Current model state
    // 2:   Window3
    // 1:   Window2
    // 0:   Window1

    // Raise windows 1 & 2 (currently at bottom)
    notifier.raiseWindows({mirWindowInfo1.window(), mirWindowInfo2.window()});

    // Model should now be like this:
    // 2:   Window1
    // 1:   Window2
    // 0:   Window3
    ASSERT_EQ(3, model.count());
    auto topWindow = getMirALWindowFromModel(model, 2);
    EXPECT_EQ(mirWindowInfo1.window(), topWindow);
    auto middleWindow = getMirALWindowFromModel(model, 1);
    EXPECT_EQ(mirWindowInfo2.window(), middleWindow);
    auto bottomWindow = getMirALWindowFromModel(model, 0);
    EXPECT_EQ(mirWindowInfo3.window(), bottomWindow);
}

/*
 * Test: with 2 window, raise the 2 windows in swapped order reorders the model
 */
TEST_F(WindowModelTest, Raising2BottomWindowsInSwappedOrderReordersTheModel)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);

    // Current model state
    // 1:   Window2
    // 0:   Window1

    // Raise windows 1 & 2 (bottom two, but in opposite order)
    notifier.raiseWindows({mirWindowInfo1.window(), mirWindowInfo2.window()});

    // Model should now be like this:
    // 1:   Window1
    // 0:   Window2
    ASSERT_EQ(2, model.count());
    auto topWindow = getMirALWindowFromModel(model, 1);
    EXPECT_EQ(mirWindowInfo1.window(), topWindow);
    auto bottomWindow = getMirALWindowFromModel(model, 0);
    EXPECT_EQ(mirWindowInfo2.window(), bottomWindow);
}

/*
 * Test: with 3 windows, raise the bottom 2 windows in swapped order reorders the model
 * so that the bottom window is at the top, and middle window remains in place.
 */
TEST_F(WindowModelTest, With3WindowsRaising2BottomWindowsInSwappedOrderReordersTheModel)
{
    WindowModelNotifier notifier;
    WindowModel model(&notifier, nullptr); // no need for controller in this testcase

    auto mirWindowInfo1 = createMirALWindowInfo();
    auto mirWindowInfo2 = createMirALWindowInfo();
    auto mirWindowInfo3 = createMirALWindowInfo();
    notifier.addWindow(mirWindowInfo1);
    notifier.addWindow(mirWindowInfo2);
    notifier.addWindow(mirWindowInfo3);

    // Current model state
    // 2:   Window3
    // 1:   Window2
    // 0:   Window1

    // Raise windows 2 & 1 (i.e. bottom two, but in opposite order)
    notifier.raiseWindows({mirWindowInfo2.window(), mirWindowInfo1.window()});

    // Model should now be like this:
    // 2:   Window2
    // 1:   Window1
    // 0:   Window3
    ASSERT_EQ(3, model.count());
    auto topWindow = getMirALWindowFromModel(model, 2);
    EXPECT_EQ(mirWindowInfo2.window(), topWindow);
    auto middleWindow = getMirALWindowFromModel(model, 1);
    EXPECT_EQ(mirWindowInfo1.window(), middleWindow);
    auto bottomWindow = getMirALWindowFromModel(model, 0);
    EXPECT_EQ(mirWindowInfo3.window(), bottomWindow);
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
    EXPECT_EQ(position, surface->position());
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

    EXPECT_EQ(newPosition, surface->position());
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

    EXPECT_EQ(newPosition, surface->position());
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
    EXPECT_EQ(fixedPosition, surface->position());
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
    EXPECT_EQ(size, surface->size());
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

    EXPECT_EQ(newSize, surface->size());
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

    EXPECT_EQ(newSize, surface->size());
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
    EXPECT_EQ(fixedPosition, surface->size());
}
