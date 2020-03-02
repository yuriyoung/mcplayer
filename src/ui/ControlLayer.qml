import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import McPlayer 1.0
import "qrc:///styles/"
import "qrc:///components/" as COM
import "qrc:///player/" as PLAYER
import "qrc:///libraries/functions.js" as FUN

Item {
    id: control

    property int bottomMargin: 16

    Label {
        id: mediaTitle

        anchors.left: playbackProgress.left
        anchors.bottom: playbackProgress.top
        font.pixelSize: 12
        width: playbackProgress.width
        elide: Text.ElideRight
        text: {
            var result = "Unknown Title"; // todo: show the media file name ?
            if(mediaMetaData) {
                if(mediaMetaData.title)
                    result += mediaMetaData.title
                if(mediaMetaData.author)
                    result += " - " + mediaMetaData.author
            }

            return result;
        }
    }

    RowLayout {
        id: tickTime

        width: playbackProgress.width
        anchors.left: playbackProgress.left
        anchors.top: playbackProgress.bottom
        Label {
            id: timeLable
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            text: mediaPlayer.time.toString()
            font.pixelSize: 10
        }

        Item { Layout.fillWidth: true }

        Label {
            id: durationLable
            Layout.alignment: Qt.AlignRight |  Qt.AlignTop
            text: mediaPlayer.duration.toString()
            font.pixelSize: 10
        }
    }

    PLAYER.ProgressControl {
        id: playbackProgress

        anchors.bottom: parent.bottom
        anchors.bottomMargin: control.bottomMargin + 12
    }

    PLAYER.PlayControl {
        id: playButton

        anchors.bottom: parent.bottom
        anchors.bottomMargin: control.bottomMargin
        x: parent.width - 140
    }
    COM.RoundButton {
        id: previousButton

        anchors.right: playButton.left
        y: playButton.y + playButton.height / 2 - previousButton.height / 2
        icon.name: AppIcons.mdi_skip_previous
        size: AppStyle.sm
        textColor: "white"
        onClicked: mediaPlayer.previous()
    }
    COM.RoundButton {
        id: nextButton

        anchors.left: playButton.right
        y: playButton.y + playButton.height / 2 - nextButton.height / 2
        icon.name: AppIcons.mdi_skip_next
        size: AppStyle.sm
        textColor: "white"
        onClicked: mediaPlayer.next()
    }
    PLAYER.VolumeControl {
        anchors.left: nextButton.right
        y: nextButton.y + nextButton.height / 2 - nextButton.height / 2
    }
}
