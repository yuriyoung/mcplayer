import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import org.mcplayer 1.0
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
            var result = mediaPlayer.metadata.Title || ""
            if(mediaPlayer.metadata.author)
                result += " - " + mediaPlayer.metadata.Author
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
            text: mediaPlayer.metadata.Duration || mediaPlayer.duration.toString()
            font.pixelSize: 10
        }
    }

    PLAYER.ProgressControl {
        id: playbackProgress

        anchors.bottom: parent.bottom
        anchors.bottomMargin: control.bottomMargin + 12
        value: mediaPlayer.position
        onPressedChanged: {
            if (mediaPlayer.playbackState !== MediaPlayer.StoppedState) {
                if (pressed) {
                    mediaPlayer.pause()
                } else {
                    mediaPlayer.seek(value)
                    mediaPlayer.resume()
                }
            }
        }
//    onMoved: {
//        if(mediaPlayer.playbackState !== MediaPlayer.StoppedState)
//            mediaPlayer.seek(valueAt(position))
//    }
    }

    PLAYER.PlayControl {
        id: playButton

        x: parent.width - 140
        anchors.bottom: parent.bottom
        anchors.bottomMargin: control.bottomMargin
        icon.name: mediaPlayer.playbackState === MediaPlayer.PlayingState
                   ? AppIcons.mdi_pause
                   : AppIcons.mdi_play
        onClicked: {
            mediaPlayer.playbackState === MediaPlayer.PlayingState
                    ? mediaPlayer.pause()
                    : mediaPlayer.play();
        }
    }

    COM.RoundButton {
        id: previousButton

        anchors.right: playButton.left
        y: playButton.y + playButton.height / 2 - previousButton.height / 2
        icon.name: AppIcons.mdi_skip_previous
        size: AppStyle.sm
        textColor: "white"
        onClicked: {
            mediaPlayer.previous()
            if(mediaPlayer.playbackState !== MediaPlayer.PlayingState)
                mediaPlayer.play()
        }
    }

    COM.RoundButton {
        id: nextButton

        anchors.left: playButton.right
        y: playButton.y + playButton.height / 2 - nextButton.height / 2
        icon.name: AppIcons.mdi_skip_next
        size: AppStyle.sm
        textColor: "white"
        onClicked: {
            mediaPlayer.next()
            if(mediaPlayer.playbackState !== MediaPlayer.PlayingState)
                mediaPlayer.play()
        }
    }

    PLAYER.VolumeControl {
        id: volumeButton

        anchors.left: nextButton.right
        y: nextButton.y + nextButton.height / 2 - nextButton.height / 2
    }
}
