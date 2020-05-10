import QtQuick 2.12
import QtQuick.Controls 2.12

import "qrc:///styles/"

ListView {
    id: control

    implicitWidth: 180
    clip: true
    focus: true
    highlight: highlight
    model: mediaPlayer.playlist
    currentIndex: mediaPlayer.playlist.currentIndex
    delegate: ItemDelegate {
        text: model.title
        width: parent.width
        onDoubleClicked: {
            mediaPlayer.playlist.currentIndex = index;
            mediaPlayer.play();
        }
    }

    Component {
        id: highlight

        Rectangle {
            color: AppStyle.accentColor
            y: control.currentItem.y
            Behavior on y {
            SpringAnimation {
                spring: 3
                damping: 0.2
                }
            }
        }
    }
}
