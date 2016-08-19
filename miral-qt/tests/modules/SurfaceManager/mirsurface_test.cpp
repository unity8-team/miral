/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#define MIR_INCLUDE_DEPRECATED_EVENT_HEADER

struct MirEvent {}; // otherwise won't compile otherwise due to incomplete type

#include <gtest/gtest.h>

#include <QLoggingCategory>
#include <QTest>
#include <QSignalSpy>

// src/common
#include "windowmodelinterface.h"

// the test subject
#include <Unity/Application/mirsurface.h>

#include <Unity/Application/timer.h>

// tests/framework
#include <mock_mir_session.h>
#include <mock_surface.h>

// tests/modules/common
#include <surfaceobserver.h>

// miral
#include "window.h"
#include "window_info.h"

using namespace qtmir;

namespace ms = mir::scene;
using namespace testing;

class MirSurfaceTest : public ::testing::Test
{
public:
    MirSurfaceTest()
    {
        // We don't want the logging spam cluttering the test results
        QLoggingCategory::setFilterRules(QStringLiteral("qtmir.surfaces=false"));
    }
};

TEST_F(MirSurfaceTest, UpdateTextureBeforeDraw)
{
    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto const fakeSession = std::make_shared<ms::MockSession>();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();
    miral::Window mockWindow(fakeSession, mockSurface);
    miral::WindowInfo mockWindowInfo(mockWindow, {});
    QScopedPointer<WindowInfo> windowInfo(new WindowInfo(mockWindowInfo));

    EXPECT_CALL(*mockSurface.get(),buffers_ready_for_compositor(_))
        .WillRepeatedly(Return(1));

    MirSurface surface(mockWindowInfo, nullptr);
    surface.surfaceObserver()->frame_posted(1, mir::geometry::Size{1,1});

    QSignalSpy spyFrameDropped(&surface, SIGNAL(frameDropped()));
    QTest::qWait(300);
    ASSERT_TRUE(spyFrameDropped.count() > 0);
}


TEST_F(MirSurfaceTest, DeleteMirSurfaceOnLastNonLiveUnregisterView)
{
    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto const fakeSession = std::make_shared<NiceMock<ms::MockSession>>();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();
    miral::Window mockWindow(fakeSession, mockSurface);
    miral::WindowInfo mockWindowInfo(mockWindow, {});
    QScopedPointer<WindowInfo> windowInfo(new WindowInfo(mockWindowInfo));

    MirSurface surface(mockWindowInfo, nullptr);

    bool surfaceDeleted = false;
    QObject::connect(&surface, &QObject::destroyed, &surface, [&surfaceDeleted](){ surfaceDeleted = true; });

    qintptr view1 = (qintptr)1;
    qintptr view2 = (qintptr)2;

    surface.registerView(view1);
    surface.registerView(view2);
    surface.setLive(false);
    EXPECT_FALSE(surfaceDeleted);

    surface.unregisterView(view1);
    surface.unregisterView(view2);

    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    EXPECT_TRUE(surfaceDeleted);
}


TEST_F(MirSurfaceTest, DoNotDeleteMirSurfaceOnLastLiveUnregisterView)
{
    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto const fakeSession = std::make_shared<ms::MockSession>();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();
    miral::Window mockWindow(fakeSession, mockSurface);
    miral::WindowInfo mockWindowInfo(mockWindow, {});
    QScopedPointer<WindowInfo> windowInfo(new WindowInfo(mockWindowInfo));

    MirSurface surface(mockWindowInfo, nullptr);

    bool surfaceDeleted = false;
    QObject::connect(&surface, &QObject::destroyed, &surface, [&surfaceDeleted](){ surfaceDeleted = true; });

    qintptr view1 = (qintptr)1;
    qintptr view2 = (qintptr)2;

    surface.registerView(view1);
    surface.registerView(view2);

    surface.unregisterView(view1);
    surface.unregisterView(view2);

    QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    EXPECT_FALSE(surfaceDeleted);
}

/*
 * Test that a surface whose client fails to comply with a close request will eventually get destroyed.
 */
TEST_F(MirSurfaceTest, failedSurfaceCloseEventuallyDestroysSurface)
{
    int argc = 0;
    char* argv[0];
    QCoreApplication qtApp(argc, argv); // app for deleteLater event

    auto const mockSession = std::make_shared<NiceMock<ms::MockSession>>();
    auto mockSurface = std::make_shared<NiceMock<ms::MockSurface>>();
    miral::Window mockWindow(mockSession, mockSurface);
    miral::WindowInfo mockWindowInfo(mockWindow, {});
    QScopedPointer<WindowInfo> windowInfo(new WindowInfo(mockWindowInfo));

    MirSurface surface(mockWindowInfo, nullptr);

    bool surfaceDeleted = false;
    QObject::connect(&surface, &QObject::destroyed, &surface, [&surfaceDeleted](){ surfaceDeleted = true; });

    QSharedPointer<FakeTimeSource> fakeTimeSource(new FakeTimeSource);
    QPointer<FakeTimer> fakeTimer(new FakeTimer(fakeTimeSource));
    surface.setCloseTimer(fakeTimer.data()); // surface takes ownership of the timer

    qintptr view = (qintptr)1;
    surface.registerView(view);

    EXPECT_CALL(*mockSession.get(), destroy_surface(An<const std::weak_ptr<ms::Surface> &>()))
        .Times(1);

    surface.close();

    if (fakeTimer->isRunning()) {
        // Simulate that closeTimer has timed out.
        fakeTimeSource->m_msecsSinceReference = fakeTimer->nextTimeoutTime() + 1;
        fakeTimer->update();
    }

    // clean up
    surface.setLive(false);
    surface.unregisterView(view);
}
