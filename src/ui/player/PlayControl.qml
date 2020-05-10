import QtQuick 2.12
import QtQuick.Controls 2.12

import "qrc:///components/" as COM
import "qrc:///styles/"

COM.RoundButton {
    id: control

    textColor: "white"
    size: AppStyle.md
    color: AppStyle.accentColor
    states: [
        State {
            name: "SMALL"
            when: mainwindow.width < AppStyle.bp_xs
            PropertyChanges {
                target: control
                anchors.leftMargin: 24
                anchors.topMargin: 24

            }
            AnchorChanges {
                target: control
                anchors.bottom: undefined
                anchors.top: playbackProgress.bottom
                anchors.left: playbackProgress.left
            }
        }
    ]

    Behavior on x { NumberAnimation { duration: 300; easing.type: Easing.OutBack } }
}
