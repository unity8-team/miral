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

#ifndef TOPLEVELWINDOWMODEL_H
#define TOPLEVELWINDOWMODEL_H

#include <QAbstractListModel>
#include <QLoggingCategory>

#include "mirsurface.h"
#include "windowmodelnotifier.h"

Q_DECLARE_LOGGING_CATEGORY(QTMIR_TOPLEVELWINDOWMODEL)

namespace unity {
    namespace shell {
        namespace application {
            class ApplicationInfoInterface;
            class ApplicationManagerInterface;
            class MirSurfaceInterface;
        }
    }
}

namespace qtmir {

class ApplicationManagerInterface;
class SessionManager;
class WindowControllerInterface;

// TODO: Define an interface in unityapi
class TopLevelWindowModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int count READ count NOTIFY countChanged)

    Q_PROPERTY(unity::shell::application::MirSurfaceInterface* inputMethodSurface READ inputMethodSurface NOTIFY inputMethodSurfaceChanged)

    Q_PROPERTY(unity::shell::application::MirSurfaceInterface* focusedSurface READ focusedSurface
                                                                              NOTIFY focusedSurfaceChanged)

public:
    /**
     * @brief The Roles supported by the model
     *
     * SurfaceRole - A MirSurfaceInterface. It will be null if the application is still starting up
     * ApplicationRole - An ApplicationInfoInterface
     * IdRole - A unique identifier for this entry. Useful to unambiguosly track elements as they move around in the list
     */
    enum Roles {
        SurfaceRole = Qt::UserRole,
        ApplicationRole = Qt::UserRole + 1,
        IdRole = Qt::UserRole + 2,
    };

    TopLevelWindowModel();
    explicit TopLevelWindowModel(WindowModelNotifier *notifier,
                         WindowControllerInterface *controller); // For testing

    // QAbstractItemModel methods
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;

    int count() const { return rowCount(); }

    unity::shell::application::MirSurfaceInterface* inputMethodSurface() const { return m_inputMethodSurface; }

    unity::shell::application::MirSurfaceInterface* focusedSurface() const;

    void setApplicationManager(ApplicationManagerInterface*);

public Q_SLOTS:
    /**
     * @brief Returns the surface at the given index
     *
     * It will be a nullptr if the application is still starting up and thus hasn't yet created
     * and drawn into a surface.
     */
    unity::shell::application::MirSurfaceInterface *surfaceAt(int index) const;

    /**
     * @brief Returns the application at the given index
     */
    unity::shell::application::ApplicationInfoInterface *applicationAt(int index) const;

    /**
     * @brief Returns the unique id of the element at the given index
     */
    int idAt(int index) const;

    /**
     * @brief Returns the index where the row with the given id is located
     *
     * Returns -1 if there's no row with the given id.
     */
    int indexForId(int id) const;

    /**
     * @brief Raises the row with the given id to the top of the window stack (index == count-1)
     */
    void raiseId(int id);

Q_SIGNALS:
    void countChanged();
    void inputMethodSurfaceChanged(unity::shell::application::MirSurfaceInterface* inputMethodSurface);

    /**
     * @brief Emitted when the list changes
     *
     * Emitted when model gains an element, loses an element or when elements exchange positions.
     */
    void listChanged();

    void focusedSurfaceChanged(unity::shell::application::MirSurfaceInterface *focusedSurface);

private Q_SLOTS:
    void onWindowAdded(const qtmir::NewWindow &windowInfo);
    void onWindowRemoved(const miral::WindowInfo &window);
    void onWindowMoved(const miral::WindowInfo &window, const QPoint topLeft);
    void onWindowStateChanged(const miral::WindowInfo &windowInfo, MirSurfaceState state);
    void onWindowFocusChanged(const miral::WindowInfo &window, bool focused);
    void onWindowsRaised(const std::vector<miral::Window> &windows);

private:
    void doRaiseId(int id);
    int generateId();
    int nextFreeId(int candidateId);
    void connectToWindowModelNotifier(WindowModelNotifier *notifier);
    QString toString();
    int indexOf(MirSurfaceInterface *surface);

    void setInputMethodWindow(MirSurface *surface);
    void setFocusedSurface(MirSurface *surface);
    void removeInputMethodWindow();
    MirSurface* find(const miral::WindowInfo &needle) const;
    int findIndexOf(const miral::Window &needle) const;
    void removeAt(int index);

    void addApplication(unity::shell::application::ApplicationInfoInterface *application);
    void removeApplication(unity::shell::application::ApplicationInfoInterface *application);

    void appendPlaceholder(unity::shell::application::ApplicationInfoInterface *application);
    void appendSurface(MirSurface *surface,
                       unity::shell::application::ApplicationInfoInterface *application);
    void appendSurfaceHelper(MirSurface *surface,
                             unity::shell::application::ApplicationInfoInterface *application);

    void connectSurface(MirSurfaceInterface *surface);

    void onSurfaceDied(MirSurfaceInterface *surface);
    void onSurfaceDestroyed(MirSurfaceInterface *surface);

    struct ModelEntry {
        ModelEntry() {}
        ModelEntry(MirSurface *surface,
                   unity::shell::application::ApplicationInfoInterface *application,
                   int id)
            : surface(surface), application(application), id(id) {}
        MirSurface *surface{nullptr};
        unity::shell::application::ApplicationInfoInterface *application{nullptr};
        int id{-1};
        bool removeOnceSurfaceDestroyed{false};
    };

    QVector<ModelEntry> m_windowModel;
    WindowControllerInterface *m_windowController;
    SessionManager* m_sessionManager;
    MirSurface* m_inputMethodSurface{nullptr};
    MirSurface* m_focusedSurface{nullptr};
    int m_nextId{1};
    // Just something big enough that we don't risk running out of unused id numbers.
    // Not sure if QML int type supports something close to std::numeric_limits<int>::max() and
    // there's no reason to try out its limits.
    static const int m_maxId{1000000};

    ApplicationManagerInterface* m_applicationManager{nullptr};

    enum ModelState {
        IdleState,
        InsertingState,
        RemovingState,
        MovingState,
        ResettingState
    };
    ModelState m_modelState{IdleState};
};

} // namespace qtmir
#endif // TOPLEVELWINDOWMODEL_H
