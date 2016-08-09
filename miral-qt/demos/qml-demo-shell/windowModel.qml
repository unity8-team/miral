import QtQuick 2.4
import Unity.Application 0.1

FocusScope {
    id: root
    focus: true

    WindowModel {
        id: windowModel;
    }

    Item {
        id: windowViewContainer
        anchors.fill: parent

        Repeater {
            model: windowModel

            delegate: MirSurfaceItem {
                id: surfaceItem
                surface: model.surface
                consumesInput: true // QUESTION: why is this non-default?
                x: surface.position.x
                y: surface.position.y
                width: surface.size.width
                height: surface.size.height
                focus: surface.focused

                Rectangle {
                    anchors { top: parent.bottom; right: parent.right }
                    width: childrenRect.width
                    height: childrenRect.height
                    color: surface.focused ? "red" : "lightsteelblue"
                    opacity: 0.8
                    Text {
                        text: surface.position.x + "," + surface.position.y + " " + surface.size.width + "x" + surface.size.height
                        font.pixelSize: 10
                    }
                }
            }
        }
    }

    Button {
        anchors { right: parent.right; bottom: parent.bottom }
        height: 30
        width: 80
        text: "Quit"
        onClicked: Qt.quit()
    }

    Text {
        anchors { left: parent.left; bottom: parent.bottom }
        text: "Move window: Ctrl+click"
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        property variant window: null
        property int initialWindowXPosition
        property int initialWindowYPosition
        property int initialMouseXPosition
        property int initialMouseYPosition

        function moveWindowBy(window, delta) {
            window.surface.requestPosition(Qt.point(initialWindowXPosition + delta.x,
                                                    initialWindowYPosition + delta.y))
        }

        onPressed: {
            if (mouse.modifiers & Qt.ControlModifier) {
                window = windowViewContainer.childAt(mouse.x, mouse.y)
                if (!window) return;
                initialWindowXPosition = window.surface.position.x
                initialWindowYPosition = window.surface.position.y
                initialMouseXPosition = mouse.x
                initialMouseYPosition = mouse.y
            } else {
                mouse.accepted = false
            }
        }

        onPositionChanged: {
            if (!window) {
                mouse.accepted = false
                return
            }
            moveWindowBy(window, Qt.point(mouse.x - initialMouseXPosition, mouse.y - initialMouseYPosition))
        }

        onReleased: {
            if (!window) {
                mouse.accepted = false
                return
            }
            moveWindowBy(window, Qt.point(mouse.x - initialMouseXPosition, mouse.y - initialMouseYPosition))
            window = null;
        }
    }
}
