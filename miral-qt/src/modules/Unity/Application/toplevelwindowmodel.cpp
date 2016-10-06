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

#include "toplevelwindowmodel.h"

#include "application_manager.h"
#include "mirsurface.h"
#include "sessionmanager.h"

#include <mir/scene/surface.h>

// mirserver
#include "nativeinterface.h"

// Qt
#include <QGuiApplication>
#include <QDebug>

Q_LOGGING_CATEGORY(QTMIR_WINDOWMODEL, "qtmir.windowmodel", QtDebugMsg)

#define DEBUG_MSG qCDebug(QTMIR_WINDOWMODEL).nospace().noquote() << __func__

using namespace qtmir;
namespace unityapi = unity::shell::application;

TopLevelWindowModel::TopLevelWindowModel()
{
    auto nativeInterface = dynamic_cast<NativeInterface*>(QGuiApplication::platformNativeInterface());

    if (!nativeInterface) {
        qFatal("ERROR: Unity.Application QML plugin requires use of the 'mirserver' QPA plugin");
    }

    m_windowController = static_cast<WindowControllerInterface*>(nativeInterface->nativeResourceForIntegration("WindowController"));

    auto windowModel = static_cast<WindowModelNotifier*>(nativeInterface->nativeResourceForIntegration("WindowModelNotifier"));
    connectToWindowModelNotifier(windowModel);

    setApplicationManager(ApplicationManager::singleton());

    m_sessionManager = SessionManager::singleton();
}

TopLevelWindowModel::TopLevelWindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller)
    : m_windowController(controller)
{
    connectToWindowModelNotifier(notifier);
}

void TopLevelWindowModel::setApplicationManager(ApplicationManagerInterface* value)
{
    if (m_applicationManager == value) {
        return;
    }

    Q_ASSERT(m_modelState == IdleState);
    m_modelState = ResettingState;

    beginResetModel();

    if (m_applicationManager) {
        m_windowModel.clear();
        disconnect(m_applicationManager, 0, this, 0);
    }

    m_applicationManager = value;

    if (m_applicationManager) {
        connect(m_applicationManager, &QAbstractItemModel::rowsInserted,
                this, [this](const QModelIndex &/*parent*/, int first, int last) {
                    for (int i = first; i <= last; ++i) {
                        auto application = m_applicationManager->get(i);
                        addApplication(application);
                    }
                });

        connect(m_applicationManager, &QAbstractItemModel::rowsAboutToBeRemoved,
                this, [this](const QModelIndex &/*parent*/, int first, int last) {
                    for (int i = first; i <= last; ++i) {
                        auto application = m_applicationManager->get(i);
                        removeApplication(application);
                    }
                });

        for (int i = 0; i < m_applicationManager->rowCount(); ++i) {
            auto application = m_applicationManager->get(i);
            addApplication(application);
        }
    }

    endResetModel();
    m_modelState = IdleState;
}

void TopLevelWindowModel::addApplication(unityapi::ApplicationInfoInterface *application)
{
    DEBUG_MSG << "(" << application->appId() << ")";

    if (application->state() != unityapi::ApplicationInfoInterface::Stopped && application->surfaceList()->count() == 0) {
        appendPlaceholder(application);
    }
}

void TopLevelWindowModel::removeApplication(unityapi::ApplicationInfoInterface *application)
{
    DEBUG_MSG << "(" << application->appId() << ")";

    Q_ASSERT(m_modelState == IdleState);
    m_modelState = RemovingState;

    int i = 0;
    while (i < m_windowModel.count()) {
        if (m_windowModel.at(i).application == application) {
            beginRemoveRows(QModelIndex(), i, i);
            m_windowModel.removeAt(i);
            endRemoveRows();
            Q_EMIT countChanged();
            Q_EMIT listChanged();
        } else {
            ++i;
        }
    }

    m_modelState = IdleState;

    DEBUG_MSG << " after " << toString();
}

void TopLevelWindowModel::appendPlaceholder(unityapi::ApplicationInfoInterface *application)
{
    DEBUG_MSG << "(" << application->appId() << ")";

    appendSurfaceHelper(nullptr, application);
}

void TopLevelWindowModel::appendSurface(MirSurface *surface, unityapi::ApplicationInfoInterface *application)
{
    Q_ASSERT(surface != nullptr);

    bool filledPlaceholder = false;
    for (int i = 0; i < m_windowModel.count() && !filledPlaceholder; ++i) {
        ModelEntry &entry = m_windowModel[i];
        if (entry.application == application && entry.surface == nullptr) {
            entry.surface = surface;
            connectSurface(surface);
            DEBUG_MSG << " appId=" << application->appId() << " surface=" << surface
                      << ", filling out placeholder. after: " << toString();
            Q_EMIT dataChanged(index(i) /* topLeft */, index(i) /* bottomRight */, QVector<int>() << SurfaceRole);
            filledPlaceholder = true;
        }
    }

    if (!filledPlaceholder) {
        DEBUG_MSG << " appId=" << application->appId() << " surface=" << surface << ", adding new row";
        appendSurfaceHelper(surface, application);
    }
}

void TopLevelWindowModel::appendSurfaceHelper(MirSurface *surface, unityapi::ApplicationInfoInterface *application)
{
    if (m_modelState == IdleState) {
        m_modelState = InsertingState;
        beginInsertRows(QModelIndex(), m_windowModel.size() /*first*/, m_windowModel.size() /*last*/);
    } else {
        Q_ASSERT(m_modelState == ResettingState);
        // No point in signaling anything if we're resetting the whole model
    }

    int id = generateId();
    m_windowModel.append(ModelEntry(surface, application, id));
    if (surface) {
        connectSurface(surface);
    }

    if (m_modelState == InsertingState) {
        endInsertRows();
        Q_EMIT countChanged();
        Q_EMIT listChanged();
        m_modelState = IdleState;
    }

    DEBUG_MSG << " after " << toString();
}

void TopLevelWindowModel::connectSurface(MirSurfaceInterface *surface)
{
    connect(surface, &MirSurfaceInterface::liveChanged, this, [this, surface](bool live){
            if (!live) {
                onSurfaceDied(surface);
            }
        });
    connect(surface, &QObject::destroyed, this, [this, surface](){ this->onSurfaceDestroyed(surface); });
}

void TopLevelWindowModel::onSurfaceDied(MirSurfaceInterface *surface)
{
    int i = indexOf(surface);
    if (i == -1) {
        return;
    }

    auto application = m_windowModel[i].application;

    // can't be starting if it already has a surface
    Q_ASSERT(application->state() != unityapi::ApplicationInfoInterface::Starting);

    if (application->state() == unityapi::ApplicationInfoInterface::Running) {
        m_windowModel[i].removeOnceSurfaceDestroyed = true;
    } else {
        // assume it got killed by the out-of-memory daemon.
        //
        // So leave entry in the model and only remove its surface, so shell can display a screenshot
        // in its place.
        m_windowModel[i].removeOnceSurfaceDestroyed = false;
    }
}

void TopLevelWindowModel::onSurfaceDestroyed(MirSurfaceInterface *surface)
{
    int i = indexOf(surface);
    if (i == -1) {
        return;
    }

    if (m_windowModel[i].removeOnceSurfaceDestroyed) {
        removeAt(i);
    } else {
        if (m_windowModel[i].surface == m_focusedSurface) {
            setFocusedSurface(nullptr);
        }
        m_windowModel[i].surface = nullptr;
        Q_EMIT dataChanged(index(i) /* topLeft */, index(i) /* bottomRight */, QVector<int>() << SurfaceRole);
        DEBUG_MSG << " Removed surface from entry. After: " << toString();
    }
}

void TopLevelWindowModel::connectToWindowModelNotifier(WindowModelNotifier *notifier)
{
    connect(notifier, &WindowModelNotifier::windowAdded,        this, &TopLevelWindowModel::onWindowAdded,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowRemoved,      this, &TopLevelWindowModel::onWindowRemoved,      Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowMoved,        this, &TopLevelWindowModel::onWindowMoved,        Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowStateChanged, this, &TopLevelWindowModel::onWindowStateChanged, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowFocusChanged, this, &TopLevelWindowModel::onWindowFocusChanged, Qt::QueuedConnection);
    connect(notifier, &WindowModelNotifier::windowsRaised,      this, &TopLevelWindowModel::onWindowsRaised,      Qt::QueuedConnection);
}

QHash<int, QByteArray> TopLevelWindowModel::roleNames() const
{
    QHash<int, QByteArray> roleNames { {SurfaceRole, "surface"},
                                       {ApplicationRole, "application"},
                                       {IdRole, "id"} };
    return roleNames;
}

void TopLevelWindowModel::onWindowAdded(const NewWindow &window)
{
    auto mirSession = window.windowInfo.window().application();
    SessionInterface* session = m_sessionManager->findSession(mirSession.get());

    auto surface = new MirSurface(window, m_windowController, session);

    if (session)
        session->registerSurface(surface);

    if (window.windowInfo.type() == mir_surface_type_inputmethod) {
        setInputMethodWindow(surface);
    } else {
        unityapi::ApplicationInfoInterface *application = m_applicationManager->findApplicationWithSession(mirSession);
        appendSurface(surface, application);
    }
}

void TopLevelWindowModel::onWindowRemoved(const miral::WindowInfo &windowInfo)
{
    if (windowInfo.type() == mir_surface_type_inputmethod) {
        removeInputMethodWindow();
        return;
    }

    const int index = findIndexOf(windowInfo.window());
    if (index >= 0) {
        m_windowModel[index].surface->setLive(false);
    }
}

void TopLevelWindowModel::removeAt(int index)
{
    if (m_modelState == IdleState) {
        beginRemoveRows(QModelIndex(), index, index);
        m_modelState = RemovingState;
    } else {
        Q_ASSERT(m_modelState == ResettingState);
        // No point in signaling anything if we're resetting the whole model
    }

    if (m_windowModel[index].surface != nullptr && m_windowModel[index].surface == m_focusedSurface) {
        setFocusedSurface(nullptr);
    }

    m_windowModel.removeAt(index);

    if (m_modelState == RemovingState) {
        endRemoveRows();
        Q_EMIT countChanged();
        Q_EMIT listChanged();
        m_modelState = IdleState;
    }

    DEBUG_MSG << " after " << toString();
}

void TopLevelWindowModel::onWindowMoved(const miral::WindowInfo &windowInfo, const QPoint topLeft)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setPosition(topLeft);
    }
}

void TopLevelWindowModel::onWindowFocusChanged(const miral::WindowInfo &windowInfo, bool focused)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->setFocused(focused);

        if (focused) {
            setFocusedSurface(mirSurface);
        } else if (mirSurface == m_focusedSurface) {
            setFocusedSurface(nullptr);
        }
    }
}

void TopLevelWindowModel::onWindowStateChanged(const miral::WindowInfo &windowInfo, Mir::State state)
{
    if (auto mirSurface = find(windowInfo)) {
        mirSurface->updateState(state);
    }
}

void TopLevelWindowModel::setInputMethodWindow(MirSurface *surface)
{
    if (m_inputMethodSurface) {
        qDebug("Multiple Input Method Surfaces created, removing the old one!");
        delete m_inputMethodSurface;
    }
    m_inputMethodSurface = surface;
    Q_EMIT inputMethodSurfaceChanged(m_inputMethodSurface);
}

void TopLevelWindowModel::removeInputMethodWindow()
{
    if (m_inputMethodSurface) {
        delete m_inputMethodSurface;
        m_inputMethodSurface = nullptr;
        Q_EMIT inputMethodSurfaceChanged(m_inputMethodSurface);
    }
}

void TopLevelWindowModel::onWindowsRaised(const std::vector<miral::Window> &windows)
{
    // Reminder: last item in the "windows" list should end up at the top of the model
    const int modelCount = m_windowModel.count();
    const int raiseCount = windows.size();

    // Assumption: no NO-OPs are in this list - Qt will crash on endMoveRows() if you try NO-OPs!!!
    // A NO-OP is if
    //    1. "indices" is an empty list
    //    2. "indices" of the form (..., modelCount - 2, modelCount - 1) which results in an unchanged list

    // Precompute the list of indices of Windows/Surfaces to raise, including the offsets due to
    // indices which have already been moved.
    QVector<QPair<int /*from*/, int /*to*/>> moveList;

    for (int i=raiseCount-1; i>=0; i--) {
        int from = findIndexOf(windows[i]);
        const int to = modelCount - raiseCount + i;

        int moveCount = 0;
        // how many list items under "index" have been moved so far, correct "from" to suit
        for (int j=raiseCount-1; j>i; j--) {
            if (findIndexOf(windows[j]) < from) {
                moveCount++;
            }
        }
        from -= moveCount;

        if (from == to) {
            // is NO-OP, would result in moving element to itself
        } else {
            moveList.prepend({from, to});
        }
    }

    // Perform the moving, trusting the moveList is correct for each iteration.
    QModelIndex parent;
    for (int i=moveList.count()-1; i>=0; i--) {
        const int from = moveList[i].first;
        const int to = moveList[i].second;

        beginMoveRows(parent, from, from, parent, to+1);
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
        const auto &window = m_windowModel.takeAt(from);
        m_windowModel.insert(to, window);
#else
        m_windowModel.move(from, to);
#endif

        endMoveRows();
    }
}

int TopLevelWindowModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_windowModel.count();
}

QVariant TopLevelWindowModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_windowModel.size())
        return QVariant();

    if (role == SurfaceRole) {
        unityapi::MirSurfaceInterface *surface = m_windowModel.at(index.row()).surface;
        return QVariant::fromValue(surface);
    } else if (role == ApplicationRole) {
        return QVariant::fromValue(m_windowModel.at(index.row()).application);
    } else if (role == IdRole) {
        return QVariant::fromValue(m_windowModel.at(index.row()).id);
    } else {
        return QVariant();
    }
}

MirSurface *TopLevelWindowModel::find(const miral::WindowInfo &needle) const
{
    auto window = needle.window();
    Q_FOREACH(const auto entry, m_windowModel) {
        if (entry.surface && entry.surface->window() == window) {
            return entry.surface;
        }
    }
    return nullptr;
}

int TopLevelWindowModel::findIndexOf(const miral::Window &needle) const
{
    for (int i=0; i<m_windowModel.count(); i++) {
        if (m_windowModel[i].surface && m_windowModel[i].surface->window() == needle) {
            return i;
        }
    }
    return -1;
}

int TopLevelWindowModel::generateId()
{
    int id = m_nextId;
    m_nextId = nextFreeId(m_nextId + 1);
    return id;
}

int TopLevelWindowModel::nextFreeId(int candidateId)
{
    if (candidateId > m_maxId) {
        return nextFreeId(1);
    } else {
        if (indexForId(candidateId) == -1) {
            // it's indeed free
            return candidateId;
        } else {
            return nextFreeId(candidateId + 1);
        }
    }
}

QString TopLevelWindowModel::toString()
{
    QString str;
    for (int i = 0; i < m_windowModel.count(); ++i) {
        auto item = m_windowModel.at(i);

        QString itemStr = QString("(index=%1,appId=%2,surface=0x%3,id=%4)")
            .arg(i)
            .arg(item.application->appId())
            .arg((qintptr)item.surface, 0, 16)
            .arg(item.id);

        if (i > 0) {
            str.append(",");
        }
        str.append(itemStr);
    }
    return str;
}

int TopLevelWindowModel::indexOf(MirSurfaceInterface *surface)
{
    for (int i = 0; i < m_windowModel.count(); ++i) {
        if (m_windowModel.at(i).surface == surface) {
            return i;
        }
    }
    return -1;
}

int TopLevelWindowModel::indexForId(int id) const
{
    for (int i = 0; i < m_windowModel.count(); ++i) {
        if (m_windowModel[i].id == id) {
            return i;
        }
    }
    return -1;
}

unityapi::MirSurfaceInterface *TopLevelWindowModel::surfaceAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].surface;
    } else {
        return nullptr;
    }
}

unityapi::ApplicationInfoInterface *TopLevelWindowModel::applicationAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].application;
    } else {
        return nullptr;
    }
}

int TopLevelWindowModel::idAt(int index) const
{
    if (index >=0 && index < m_windowModel.count()) {
        return m_windowModel[index].id;
    } else {
        return 0;
    }
}

void TopLevelWindowModel::raiseId(int id)
{
    if (m_modelState == IdleState) {
        DEBUG_MSG << "(id=" << id << ") - do it now.";
        doRaiseId(id);
    } else {
        DEBUG_MSG << "(id=" << id << ") - Model busy (modelState=" << m_modelState << "). Try again in the next event loop.";
        // The model has just signalled some change. If we have a Repeater responding to this update, it will get nuts
        // if we perform yet another model change straight away.
        //
        // A bad sympton of this problem is a Repeater.itemAt(index) call returning null event though Repeater.count says
        // the index is definitely within bounds.
        QMetaObject::invokeMethod(this, "raiseId", Qt::QueuedConnection, Q_ARG(int, id));
    }
}

void TopLevelWindowModel::doRaiseId(int id)
{
    int fromIndex = indexForId(id);
    if (fromIndex != -1) {
        auto surface = m_windowModel[fromIndex].surface;
        if (surface) {
            m_windowController->raise(surface->window());
        } else {
            // move it ourselves. Since there's no mir::scene::Surface/miral::Window, there's nothing
            // miral can do about it.
            move(fromIndex, m_windowModel.count() - 1);
        }
    }
}

void TopLevelWindowModel::setFocusedSurface(MirSurface *surface)
{
    if (surface != m_focusedSurface) {
        DEBUG_MSG << "(" << surface << ")";
        m_focusedSurface = surface;
        Q_EMIT focusedSurfaceChanged(m_focusedSurface);
    }
}

unityapi::MirSurfaceInterface* TopLevelWindowModel::focusedSurface() const
{
    return m_focusedSurface;
}

void TopLevelWindowModel::move(int from, int to)
{
    if (from == to) return;
    DEBUG_MSG << " from=" << from << " to=" << to;

    if (from >= 0 && from < m_windowModel.size() && to >= 0 && to < m_windowModel.size()) {
        QModelIndex parent;
        /* When moving an item down, the destination index needs to be incremented
           by one, as explained in the documentation:
           http://qt-project.org/doc/qt-5.0/qtcore/qabstractitemmodel.html#beginMoveRows */

        Q_ASSERT(m_modelState == IdleState);
        m_modelState = MovingState;

        beginMoveRows(parent, from, from, parent, to + (to > from ? 1 : 0));
        m_windowModel.move(from, to);
        endMoveRows();

        m_modelState = IdleState;

        DEBUG_MSG << " after " << toString();
    }
}
