import QtQuick 2.12
import QtQuick.Controls 2.12

import McPlayer 1.0
import "qrc:///components/" as COM
import "qrc:///styles/"

COM.RoundButton {
    id: control

    icon.name: mediaPlayer.playbackState === MediaPlayer.PlayingState
               ? AppIcons.mdi_pause
               : AppIcons.mdi_play
    textColor: "white"
    size: AppStyle.md
    color: AppStyle.accentColor
    onClicked: {
        mediaPlayer.playbackState === MediaPlayer.PlayingState
                ? mediaPlayer.pause()
                : mediaPlayer.play();
    }

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
