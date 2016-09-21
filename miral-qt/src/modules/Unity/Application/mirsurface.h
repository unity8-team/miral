/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#ifndef QTMIR_MIRSURFACE_H
#define QTMIR_MIRSURFACE_H

#include "mirsurfaceinterface.h"
#include "mirsurfacelistmodel.h"

// Qt
#include <QCursor>
#include <QMutex>
#include <QPointer>
#include <QRect>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QSet>
#include <QTimer>

#include "mirbuffersgtexture.h"
#include "windowcontrollerinterface.h"
#include "windowmodelnotifier.h"

// mir
#include <mir_toolkit/common.h>


class SurfaceObserver;

namespace qtmir {

class AbstractTimer;
class SessionInterface;

class MirSurface : public MirSurfaceInterface
{
    Q_OBJECT

public:
    MirSurface(NewWindow windowInfo,
               WindowControllerInterface *controller,
               SessionInterface *session = nullptr);
    virtual ~MirSurface();

    ////
    // unity::shell::application::MirSurfaceInterface

    Mir::Type type() const override;

    QString name() const override;

    QString persistentId() const override;

    QSize size() const override;
    void resize(int width, int height) override;
    Q_INVOKABLE void resize(const QSize &size) override { resize(size.width(), size.height()); }

    QPoint position() const override;
    Q_INVOKABLE void requestPosition(const QPoint newPosition) override;

    Mir::State state() const override;
    void setState(Mir::State qmlState) override; // To remove from unity-api
    void requestState(Mir::State qmlState) override;

    bool live() const override;

    bool visible() const override;

    Mir::OrientationAngle orientationAngle() const override;
    void setOrientationAngle(Mir::OrientationAngle angle) override;

    int minimumWidth() const override;
    int minimumHeight() const override;
    int maximumWidth() const override;
    int maximumHeight() const override;
    int widthIncrement() const override;
    int heightIncrement() const override;

    bool focused() const override;
    QRect inputBounds() const override;

    Q_INVOKABLE void requestFocus() override;
    Q_INVOKABLE void close() override;
    Q_INVOKABLE void raise() override;

    ////
    // qtmir::MirSurfaceInterface

    void setLive(bool value) override;

    bool isFirstFrameDrawn() const override { return m_firstFrameDrawn; }

    void stopFrameDropper() override;
    void startFrameDropper() override;

    bool isBeingDisplayed() const override;

    void registerView(qintptr viewId) override;
    void unregisterView(qintptr viewId) override;
    void setViewExposure(qintptr viewId, bool exposed) override;

    // methods called from the rendering (scene graph) thread:
    QSharedPointer<QSGTexture> texture() override;
    QSGTexture *weakTexture() const override { return m_texture.data(); }
    bool updateTexture() override;
    unsigned int currentFrameNumber() const override;
    bool numBuffersReadyForCompositor() override;
    // end of methods called from the rendering (scene graph) thread

    void setFocused(bool focus) override;

    void setViewActiveFocus(qintptr viewId, bool value) override;
    bool activeFocus() const override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void touchEvent(Qt::KeyboardModifiers qmods,
            const QList<QTouchEvent::TouchPoint> &qtTouchPoints,
            Qt::TouchPointStates qtTouchPointStates,
            ulong qtTimestamp) override;

    QString appId() const override;

    QCursor cursor() const override;

    void setKeymap(const QString &) override;
    QString keymap() const override;

    Mir::ShellChrome shellChrome() const override;

    SessionInterface* session() override { return m_session.data(); }

    bool inputAreaContains(const QPoint &) const override;

    ////
    // Own API
    void setPosition(const QPoint newPosition);
    void updateWindowInfo(const miral::WindowInfo &windowInfo);
    void updateState(MirSurfaceState state);

    // useful for tests
    void setCloseTimer(AbstractTimer *timer);
    std::shared_ptr<SurfaceObserver> surfaceObserver() const;
    miral::Window window() const { return m_windowInfo.window(); }

public Q_SLOTS:
    void onCompositorSwappedBuffers() override;

    void setShellChrome(Mir::ShellChrome shellChrome) override;

private Q_SLOTS:
    void dropPendingBuffer();
    void onAttributeChanged(const MirSurfaceAttrib, const int);
    void onFramesPostedObserved();
    void onSessionDestroyed();
    void emitSizeChanged();
    void setCursor(const QCursor &cursor);
    void onCloseTimedOut();
    void setInputBounds(const QRect &rect);

private:
    void syncSurfaceSizeWithItemSize();
    bool clientIsRunning() const;
    void updateExposure();
    void applyKeymap();
    void updateActiveFocus();

    miral::WindowInfo m_windowInfo;
    std::shared_ptr<mir::scene::Surface> m_surface; // keep copy of the Surface for lifecycle
    QPointer<SessionInterface> m_session;
    WindowControllerInterface *const m_controller;
    QString m_persistentId;
    bool m_firstFrameDrawn;

    //FIXME -  have to save the state as Mir has no getter for it (bug:1357429)
    Mir::OrientationAngle m_orientationAngle;

    QTimer m_frameDropperTimer;

    mutable QMutex m_mutex;

    // Lives in the rendering (scene graph) thread
    QWeakPointer<QSGTexture> m_texture;
    bool m_textureUpdated;
    unsigned int m_currentFrameNumber;

    bool m_live;
    struct View {
        bool exposed;
    };
    QHash<qintptr, View> m_views;

    QSet<qintptr> m_activelyFocusedViews;
    bool m_neverSetSurfaceFocus{true};

    std::shared_ptr<SurfaceObserver> m_surfaceObserver;

    QPoint m_position;
    QSize m_size;
    QString m_keymap;

    QCursor m_cursor;
    Mir::ShellChrome m_shellChrome;

    QRect m_inputBounds;

    bool m_focused{false};

    enum ClosingState {
        NotClosing = 0,
        Closing = 1,
        CloseOverdue = 2
    };
    ClosingState m_closingState{NotClosing};
    AbstractTimer *m_closeTimer{nullptr};
};

} // namespace qtmir

#endif // QTMIR_MIRSURFACE_H
