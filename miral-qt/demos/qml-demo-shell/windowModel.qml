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

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton

        onPressed: {
            var window = windowViewContainer.childAt(mouse.x, mouse.y)
            print("GERRY", window)
            window.surfaceWidth = 400
            window.surfaceHeight = 150
        }
    }
}
