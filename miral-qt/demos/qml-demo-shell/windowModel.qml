import QtQuick 2.4
import Unity.Application 0.1

FocusScope {
    focus: true

    WindowModel {
        id: windowModel;
    }

    Repeater {
        anchors.fill: parent
        model: windowModel

        delegate: MirSurfaceItem {
            surface: model.surface
            consumesInput: true // QUESTION: why is this non-default?
            x: surface.position.x
            y: surface.position.y
            focus: true //surface.focused is not working yet
        }
    }
}
