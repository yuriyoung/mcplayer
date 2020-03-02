import QtQuick 2.0

ListView {
    id: control

    implicitWidth: 180
    clip: true
    model: playlistModel


    Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutBack } }
}
