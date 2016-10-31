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

#include "mirsurface.h"
#include "mirsurfacelistmodel.h"
#include "session_interface.h"
#include "timer.h"
#include "timestamp.h"

// from common dir
#include <debughelpers.h>
#include "mirqtconversion.h"

// mirserver
#include <eventbuilder.h>
#include <surfaceobserver.h>
#include "screen.h"

// Mir
#include <mir/geometry/rectangle.h>
#include <mir/scene/surface.h>

// mirserver
#include <logging.h>

// Qt
#include <QQmlEngine>
#include <QScreen>

// std
#include <limits>

using namespace qtmir;

#define DEBUG_MSG qCDebug(QTMIR_SURFACES).nospace() << "MirSurface[" << (void*)this << "," << appId() << "]::" << __func__
#define WARNING_MSG qCWarning(QTMIR_SURFACES).nospace() << "MirSurface[" << (void*)this << "," << appId() << "]::" << __func__

namespace {

enum class DirtyState {
    Clean = 0,
    Name = 1 << 1,
    Type = 1 << 2,
    State = 1 << 3,
    RestoreRect = 1 << 4,
    Children = 1 << 5,
    MinSize = 1 << 6,
    MaxSize = 1 << 7,
};
Q_DECLARE_FLAGS(DirtyStates, DirtyState)

} // namespace {

MirSurface::MirSurface(NewWindow newWindowInfo,
        WindowControllerInterface* controller,
        SessionInterface *session)
    : MirSurfaceInterface()
    , m_windowInfo(newWindowInfo.windowInfo)
    , m_surface(newWindowInfo.surface)
    , m_session(session)
    , m_controller(controller)
    , m_orientationAngle(Mir::Angle0)
    , m_textureUpdated(false)
    , m_currentFrameNumber(0)
    , m_visible(newWindowInfo.windowInfo.is_visible())
    , m_live(true)
    , m_surfaceObserver(std::make_shared<SurfaceObserver>())
    , m_position(toQPoint(m_window.top_left()))
    , m_size(toQSize(m_window.size()))
    , m_state(toQtState(m_windowInfo.state()))
    , m_shellChrome(Mir::NormalChrome)
{
    DEBUG_MSG << "()";

    SurfaceObserver::registerObserverForSurface(m_surfaceObserver.get(), m_surface.get());
    m_surface->add_observer(m_surfaceObserver);

    //m_shellChrome = creationHints.shellChrome; TODO - where will this come from now?

    connect(m_surfaceObserver.get(), &SurfaceObserver::framesPosted, this, &MirSurface::onFramesPostedObserved);
    connect(m_surfaceObserver.get(), &SurfaceObserver::attributeChanged, this, &MirSurface::onAttributeChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::nameChanged, this, &MirSurface::nameChanged);
    connect(m_surfaceObserver.get(), &SurfaceObserver::cursorChanged, this, &MirSurface::setCursor);
    connect(m_surfaceObserver.get(), &SurfaceObserver::minimumWidthChanged, this, &MirSurface::minimumWidth);
    connect(m_surfaceObserver.get(), &SurfaceObserver::minimumHeightChanged, this, &MirSurface::minimumHeight);
    connect(m_surfaceObserver.get(), &SurfaceObserver::maximumWidthChanged, this, &MirSurface::maximumWidth);
    connect(m_surfaceObserver.get(), &SurfaceObserver::maximumHeightChanged, this, &MirSurface::maximumHeight);
    connect(m_surfaceObserver.get(), &SurfaceObserver::widthIncrementChanged, this, &MirSurface::widthIncrement);
    connect(m_surfaceObserver.get(), &SurfaceObserver::heightIncrementChanged, this, &MirSurface::heightIncrement);
    connect(m_surfaceObserver.get(), &SurfaceObserver::shellChromeChanged, this, [&](MirShellChrome shell_chrome) {
        setShellChrome(static_cast<Mir::ShellChrome>(shell_chrome));
    });
    connect(m_surfaceObserver.get(), &SurfaceObserver::inputBoundsChanged, this, &MirSurface::setInputBounds);
    connect(m_surfaceObserver.get(), &SurfaceObserver::confinesMousePointerChanged, this, &MirSurface::confinesMousePointerChanged);
    m_surfaceObserver->setListener(this);

    //connect(session, &QObject::destroyed, this, &MirSurface::onSessionDestroyed); // TODO try using Shared pointer for lifecycle

    connect(&m_frameDropperTimer, &QTimer::timeout,
            this, &MirSurface::dropPendingBuffer);
    // Rationale behind the frame dropper and its interval value:
    //
    // We want to give ample room for Qt scene graph to have a chance to fetch and render
    // the next pending buffer before we take the drastic action of dropping it (so don't set
    // it anywhere close to our target render interval).
    //
    // We also want to guarantee a minimal frames-per-second (fps) frequency for client applications
    // as they get stuck on swap_buffers() if there's no free buffer to swap to yet (ie, they
    // are all pending consumption by the compositor, us). But on the other hand, we don't want
    // that minimal fps to be too high as that would mean this timer would be triggered way too often
    // for nothing causing unnecessary overhead as actually dropping frames from an app should
    // in practice rarely happen.
    m_frameDropperTimer.setInterval(200);
    m_frameDropperTimer.setSingleShot(false);

    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    setCloseTimer(new Timer);

    m_requestedPosition.rx() = std::numeric_limits<int>::min();
    m_requestedPosition.ry() = std::numeric_limits<int>::min();
}

MirSurface::~MirSurface()
{
    DEBUG_MSG << "() viewCount=" << m_views.count();

    Q_ASSERT(m_views.isEmpty());

    QMutexLocker locker(&m_mutex);
    m_surface->remove_observer(m_surfaceObserver);

    delete m_closeTimer;

    Q_EMIT destroyed(this); // Early warning, while MirSurface methods can still be accessed.
}

void MirSurface::onFramesPostedObserved()
{
    // restart the frame dropper so that items have enough time to render the next frame.
    m_frameDropperTimer.start();

    Q_EMIT framesPosted();
}

void MirSurface::onAttributeChanged(const MirSurfaceAttrib attribute, const int /*value*/)
{
    switch (attribute) {
    case mir_surface_attrib_type:
        DEBUG_MSG << " type = " << mirSurfaceTypeToStr(state());
        Q_EMIT typeChanged(type());
        break;
    default:
        break;
    }
}

Mir::Type MirSurface::type() const
{
    switch (m_windowInfo.type()) {
    case mir_surface_type_normal:
        return Mir::NormalType;

    case mir_surface_type_utility:
        return Mir::UtilityType;

    case mir_surface_type_dialog:
        return Mir::DialogType;

    case mir_surface_type_gloss:
        return Mir::GlossType;

    case mir_surface_type_freestyle:
        return Mir::FreeStyleType;

    case mir_surface_type_menu:
        return Mir::MenuType;

    case mir_surface_type_inputmethod:
        return Mir::InputMethodType;

    case mir_surface_type_satellite:
        return Mir::SatelliteType;

    case mir_surface_type_tip:
        return Mir::TipType;

    default:
        return Mir::UnknownType;
    }
}

void MirSurface::dropPendingBuffer()
{
    QMutexLocker locker(&m_mutex);

    const void* const userId = (void*)123;  // TODO: Multimonitor support

    const int framesPending = m_surface->buffers_ready_for_compositor(userId);
    if (framesPending > 0) {
        m_textureUpdated = false;

        locker.unlock();
        if (updateTexture()) {
            DEBUG_MSG << "() dropped=1 left=" << framesPending-1;
        } else {
            // If we haven't managed to update the texture, don't keep banging away.
            m_frameDropperTimer.stop();
            DEBUG_MSG << "() dropped=0" << " left=" << framesPending << " - failed to upate texture";
        }
        Q_EMIT frameDropped();
    } else {
        // The client can't possibly be blocked in swap buffers if the
        // queue is empty. So we can safely enter deep sleep now. If the
        // client provides any new frames, the timer will get restarted
        // via scheduleTextureUpdate()...
        m_frameDropperTimer.stop();
    }
}

void MirSurface::stopFrameDropper()
{
    DEBUG_MSG << "()";
    m_frameDropperTimer.stop();
}

void MirSurface::startFrameDropper()
{
    DEBUG_MSG << "()";
    if (!m_frameDropperTimer.isActive()) {
        m_frameDropperTimer.start();
    }
}

QSharedPointer<QSGTexture> MirSurface::texture()
{
    QMutexLocker locker(&m_mutex);

    if (!m_texture) {
        QSharedPointer<QSGTexture> texture(new MirBufferSGTexture);
        m_texture = texture.toWeakRef();
        return texture;
    } else {
        return m_texture.toStrongRef();
    }
}

bool MirSurface::updateTexture()
{
    QMutexLocker locker(&m_mutex);

    MirBufferSGTexture *texture = static_cast<MirBufferSGTexture*>(m_texture.data());
    if (!texture) return false;

    if (m_textureUpdated) {
        return texture->hasBuffer();
    }

    const void* const userId = (void*)123;
    auto renderables = m_surface->generate_renderables(userId);

    if (renderables.size() > 0 &&
            (m_surface->buffers_ready_for_compositor(userId) > 0 || !texture->hasBuffer())
        ) {
        // Avoid holding two buffers for the compositor at the same time. Thus free the current
        // before acquiring the next
        texture->freeBuffer();
        texture->setBuffer(renderables[0]->buffer());
        ++m_currentFrameNumber;

        if (texture->textureSize() != size()) {
            m_size = texture->textureSize();
            QMetaObject::invokeMethod(this, "emitSizeChanged", Qt::QueuedConnection);
        }

        m_textureUpdated = true;
    }

    if (m_surface->buffers_ready_for_compositor(userId) > 0) {
        // restart the frame dropper to give MirSurfaceItems enough time to render the next frame.
        // queued since the timer lives in a different thread
        QMetaObject::invokeMethod(&m_frameDropperTimer, "start", Qt::QueuedConnection);
    }

    return texture->hasBuffer();
}

void MirSurface::onCompositorSwappedBuffers()
{
    QMutexLocker locker(&m_mutex);
    m_textureUpdated = false;
}

bool MirSurface::numBuffersReadyForCompositor()
{
    QMutexLocker locker(&m_mutex);
    const void* const userId = (void*)123;
    return m_surface->buffers_ready_for_compositor(userId);
}

void MirSurface::setFocused(bool value)
{
    if (m_focused == value)
        return;

    DEBUG_MSG << "(" << value << ")";

    m_focused = value;
    Q_EMIT focusedChanged(value);
}

void MirSurface::setViewActiveFocus(qintptr viewId, bool value)
{
    if (value && !m_activelyFocusedViews.contains(viewId)) {
        m_activelyFocusedViews.insert(viewId);
        updateActiveFocus();
    } else if (!value && (m_activelyFocusedViews.contains(viewId) || m_neverSetSurfaceFocus)) {
        m_activelyFocusedViews.remove(viewId);
        updateActiveFocus();
    }
}

bool MirSurface::activeFocus() const
{
    return !m_activelyFocusedViews.empty();
}

void MirSurface::updateActiveFocus()
{
    if (!m_session) {
        return;
    }

    // Temporary hotfix for http://pad.lv/1483752
    if (m_session->childSessions()->rowCount() > 0) {
        // has child trusted session, ignore any focus change attempts
        DEBUG_MSG << "() has child trusted session, ignore any focus change attempts";
        return;
    }

    // TODO Figure out what to do here
    /*
    if (m_activelyFocusedViews.isEmpty()) {
        DEBUG_MSG << "() unfocused";
        m_controller->setActiveFocus(m_window, false);
    } else {
        DEBUG_MSG << "() focused";
        m_controller->setActiveFocus(m_window, true);
    }
    */

    m_neverSetSurfaceFocus = false;
}

void MirSurface::updateVisible()
{
    const bool visible = m_windowInfo.is_visible();
    if (m_visible != visible) {
        m_visible = visible;
        Q_EMIT visibleChanged(visible);
    }
}

void MirSurface::close()
{
    if (m_closingState != NotClosing) {
        return;
    }

    DEBUG_MSG << "()";

    m_closingState = Closing;
    Q_EMIT closeRequested();
    m_closeTimer->start();

    if (m_surface) {
        m_surface->request_client_surface_close();
    }
}

void MirSurface::resize(int width, int height)
{
    bool mirSizeIsDifferent = width != m_size.width() || height != m_size.height();

    if (clientIsRunning() && mirSizeIsDifferent) {
        m_controller->resize(m_window, QSize(width, height));
        DEBUG_MSG << " old (" << m_size.width() << "," << m_size.height() << ")"
                  << ", new (" << width << "," << height << ")";
    }
}

QPoint MirSurface::position() const
{
    return m_position;
}

void MirSurface::setPosition(const QPoint newPosition)
{
    if (m_position != newPosition) {
        m_position = newPosition;
        Q_EMIT positionChanged(newPosition);
    }
}

QSize MirSurface::size() const
{
    return m_size;
}

Mir::State MirSurface::state() const
{
    // FIXME: use the commented line below when possible
    // return toQtState(m_windowInfo.state());
    return m_state;
}

Mir::OrientationAngle MirSurface::orientationAngle() const
{
    return m_orientationAngle;
}

void MirSurface::setOrientationAngle(Mir::OrientationAngle angle)
{
    MirOrientation mirOrientation;

    if (angle == m_orientationAngle) {
        return;
    }

    m_orientationAngle = angle;

    switch (angle) {
    case Mir::Angle0:
        mirOrientation = mir_orientation_normal;
        break;
    case Mir::Angle90:
        mirOrientation = mir_orientation_right;
        break;
    case Mir::Angle180:
        mirOrientation = mir_orientation_inverted;
        break;
    case Mir::Angle270:
        mirOrientation = mir_orientation_left;
        break;
    default:
        qCWarning(QTMIR_SURFACES, "Unsupported orientation angle: %d", angle);
        return;
    }

    if (m_surface) {
        m_surface->set_orientation(mirOrientation);
    }

    Q_EMIT orientationAngleChanged(angle);
}

QString MirSurface::name() const
{
    return QString::fromStdString(m_windowInfo.name());
}

QString MirSurface::persistentId() const
{
    return getExtraInfo(m_windowInfo)->persistentId;
}

void MirSurface::requestState(Mir::State state)
{
    DEBUG_MSG << "(" << unityapiMirStateToStr(state) << ")";
    m_controller->requestState(m_window, state);
}

void MirSurface::setLive(bool value)
{
    if (value != m_live) {
        DEBUG_MSG << "(" << value << ")";
        m_live = value;
        Q_EMIT liveChanged(value);
        if (m_views.isEmpty() && !m_live) {
            deleteLater();
        }
    }
}

bool MirSurface::live() const
{
    return m_live;
}

bool MirSurface::visible() const
{
    return m_windowInfo.is_visible();
}
#include <mir_toolkit/event.h>
void MirSurface::mousePressEvent(QMouseEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::mouseMoveEvent(QMouseEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::mouseReleaseEvent(QMouseEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::hoverEnterEvent(QHoverEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::hoverLeaveEvent(QHoverEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::hoverMoveEvent(QHoverEvent *event)
{
    auto ev = EventBuilder::instance()->reconstructMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::wheelEvent(QWheelEvent *event)
{
    auto ev = EventBuilder::instance()->makeMirEvent(event);
    auto ev1 = reinterpret_cast<MirPointerEvent const*>(ev.get());
    m_controller->deliverPointerEvent(m_window, ev1);
    event->accept();
}

void MirSurface::keyPressEvent(QKeyEvent *qtEvent)
{
    auto ev = EventBuilder::instance()->makeMirEvent(qtEvent);
    auto ev1 = reinterpret_cast<MirKeyboardEvent const*>(ev.get());
    m_controller->deliverKeyboardEvent(m_window, ev1);
    qtEvent->accept();
}

void MirSurface::keyReleaseEvent(QKeyEvent *qtEvent)
{
    auto ev = EventBuilder::instance()->makeMirEvent(qtEvent);
    auto ev1 = reinterpret_cast<MirKeyboardEvent const*>(ev.get());
    m_controller->deliverKeyboardEvent(m_window, ev1);
    qtEvent->accept();
}

void MirSurface::touchEvent(Qt::KeyboardModifiers mods,
                            const QList<QTouchEvent::TouchPoint> &touchPoints,
                            Qt::TouchPointStates touchPointStates,
                            ulong timestamp)
{
    auto ev = EventBuilder::instance()->makeMirEvent(mods, touchPoints, touchPointStates, timestamp);
    auto ev1 = reinterpret_cast<MirTouchEvent const*>(ev.get());
    m_controller->deliverTouchEvent(m_window, ev1);
}

bool MirSurface::clientIsRunning() const
{
    return (m_session &&
            (m_session->state() == SessionInterface::State::Running
             || m_session->state() == SessionInterface::State::Starting
             || m_session->state() == SessionInterface::State::Suspending))
        || !m_session;
}

bool MirSurface::isBeingDisplayed() const
{
    return !m_views.isEmpty();
}

void MirSurface::registerView(qintptr viewId)
{
    m_views.insert(viewId, MirSurface::View{false});
    DEBUG_MSG << "(" << viewId << ")" << " after=" << m_views.count();
    if (m_views.count() == 1) {
        Q_EMIT isBeingDisplayedChanged();
    }
}

void MirSurface::unregisterView(qintptr viewId)
{
    m_views.remove(viewId);
    DEBUG_MSG << "(" << viewId << ")" << " after=" << m_views.count() << " live=" << m_live;
    if (m_views.count() == 0) {
        Q_EMIT isBeingDisplayedChanged();
        if (m_session.isNull() || !m_live) {
            deleteLater();
        }
    }
    updateExposure();
    setViewActiveFocus(viewId, false);
}

void MirSurface::setViewExposure(qintptr viewId, bool exposed)
{
    if (!m_views.contains(viewId)) return;

    m_views[viewId].exposed = exposed;
    updateExposure();
}

void MirSurface::updateExposure()
{
    // Only update exposure after client has swapped a frame (aka surface is "ready"). MirAL only considers
    // a surface visible after it has drawn something
    if (!m_ready) {
        return;
    }

    bool newExposed = false;
    QHashIterator<qintptr, View> i(m_views);
    while (i.hasNext()) {
        i.next();
        newExposed |= i.value().exposed;
    }

    const bool oldExposed = (m_surface->query(mir_surface_attrib_visibility) == mir_surface_visibility_exposed);

    if (newExposed != oldExposed) {
        DEBUG_MSG << "(" << newExposed << ")";

        m_surface->configure(mir_surface_attrib_visibility,
                             newExposed ? mir_surface_visibility_exposed : mir_surface_visibility_occluded);
    }
}

unsigned int MirSurface::currentFrameNumber() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentFrameNumber;
}

void MirSurface::onSessionDestroyed()
{
    if (m_views.isEmpty()) {
        deleteLater();
    }
}

void MirSurface::emitSizeChanged()
{
    Q_EMIT sizeChanged(m_size);
}

QString MirSurface::appId() const
{
    QString appId;

    if (m_session && m_session->application()) {
        appId = m_session->application()->appId();
    } else {
        appId.append("-");
    }
    return appId;
}

void MirSurface::setKeymap(const QString &layoutPlusVariant)
{
    if (m_keymap == layoutPlusVariant) {
        return;
    }

    DEBUG_MSG << "(" << layoutPlusVariant << ")";

    m_keymap = layoutPlusVariant;
    Q_EMIT keymapChanged(m_keymap);

    applyKeymap();
}

QString MirSurface::keymap() const
{
    return m_keymap;
}

void MirSurface::applyKeymap()
{
    QStringList stringList = m_keymap.split('+', QString::SkipEmptyParts);

    QString layout = stringList[0];
    QString variant;

    if (stringList.count() > 1) {
        variant = stringList[1];
    }

    if (layout.isEmpty()) {
        WARNING_MSG << "Setting keymap with empty layout is not supported";
        return;
    }

    m_surface->set_keymap(MirInputDeviceId(), "", layout.toStdString(), variant.toStdString(), "");
}

QCursor MirSurface::cursor() const
{
    return m_cursor;
}

Mir::ShellChrome MirSurface::shellChrome() const
{
    return m_shellChrome;
}

void MirSurface::setShellChrome(Mir::ShellChrome shellChrome)
{
    if (m_shellChrome != shellChrome) {
        m_shellChrome = shellChrome;

        Q_EMIT shellChromeChanged(shellChrome);
    }
}

bool MirSurface::inputAreaContains(const QPoint &point) const
{
    bool result;


    // Can't use it due to https://bugs.launchpad.net/mir/+bug/1598936
    // FIXME: Use the line below instead of m_inputBounds once this bug gets fixed.
    //result = m_surface->input_area_contains(mir::geometry::Point(point.x(), point.y()));

    if (m_inputBounds.isNull()) {
        result = true;
    } else {
        result = m_inputBounds.contains(point);
    }

    return result;
}

void MirSurface::updateState(Mir::State newState)
{
    if (newState == m_state) {
        return;
    }
    DEBUG_MSG << "(" << unityapiMirStateToStr(newState) << ")";

    m_state = newState;
    m_windowInfo.state(toMirState(newState));
    Q_EMIT stateChanged(state());

    // Mir determines visibility from the state, it may have changed
    updateVisible();
}

void MirSurface::setReady()
{
    m_ready = true;
    Q_EMIT ready();
    updateVisible();
}

void MirSurface::setCursor(const QCursor &cursor)
{
    DEBUG_MSG << "(" << qtCursorShapeToStr(cursor.shape()) << ")";

    m_cursor = cursor;
    Q_EMIT cursorChanged(m_cursor);
}

int MirSurface::minimumWidth() const
{
    return m_windowInfo.min_width().as_int();
}

int MirSurface::minimumHeight() const
{
    return m_windowInfo.min_height().as_int();
}

int MirSurface::maximumWidth() const
{
    return m_windowInfo.max_width().as_int();
}

int MirSurface::maximumHeight() const
{
    return m_windowInfo.max_height().as_int();
}

int MirSurface::widthIncrement() const
{
    return 0; //TODO
}

int MirSurface::heightIncrement() const
{
    return 0; //TODO
}

bool MirSurface::focused() const
{
    return m_focused;
}

QRect MirSurface::inputBounds() const
{
    return m_inputBounds;
}

bool MirSurface::confinesMousePointer() const
{
    return m_surface->confine_pointer_state() == mir_pointer_confined_to_surface;
}

void MirSurface::requestFocus()
{
    DEBUG_MSG << "()";
    m_controller->activate(m_window);
}

void MirSurface::raise()
{
    DEBUG_MSG << "()";
    Q_EMIT raiseRequested();
}

void MirSurface::onCloseTimedOut()
{
    Q_ASSERT(m_closingState == Closing);

    DEBUG_MSG << "()";

    m_closingState = CloseOverdue;

    m_controller->requestClose(m_window);
}

void MirSurface::setCloseTimer(AbstractTimer *timer)
{
    bool timerWasRunning = false;

    if (m_closeTimer) {
        timerWasRunning = m_closeTimer->isRunning();
        delete m_closeTimer;
    }

    m_closeTimer = timer;
    m_closeTimer->setInterval(3000);
    m_closeTimer->setSingleShot(true);
    connect(m_closeTimer, &AbstractTimer::timeout, this, &MirSurface::onCloseTimedOut);

    if (timerWasRunning) {
        m_closeTimer->start();
    }
}

std::shared_ptr<SurfaceObserver> MirSurface::surfaceObserver() const
{
    return m_surfaceObserver;
}

void MirSurface::setInputBounds(const QRect &rect)
{
    if (m_inputBounds != rect) {
        DEBUG_MSG << "(" << rect << ")";
        m_inputBounds = rect;
        Q_EMIT inputBoundsChanged(m_inputBounds);
    }
}

QPoint MirSurface::requestedPosition() const
{
    return m_requestedPosition;
}

void MirSurface::setRequestedPosition(const QPoint &point)
{
    if (point != m_requestedPosition) {
        m_requestedPosition = point;
        Q_EMIT requestedPositionChanged(m_requestedPosition);
        m_controller->move(m_window, m_requestedPosition);
    }
}
