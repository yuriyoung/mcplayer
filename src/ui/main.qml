import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Window 2.12

import org.mcplayer 1.0
import "qrc:///styles/"

Window {
    id: mainwindow

    visible: true
    width: 640
    height: 360
    minimumWidth: 160
    minimumHeight: 200
    title: qsTr("McPlayer 0.0.1-dev")

    MouseArea {
        property point pressedPos: "1,1"

        anchors.fill: parent
        onPressed: {
            pressedPos = Qt.point(mouse.x, mouse.y);
        }
        onPositionChanged: {
            var delta = Qt.point(mouse.x - pressedPos.x, mouse.y - pressedPos.y);
            Window.window.x += delta.x;
            Window.window.y += delta.y;
        }
    }

    Rectangle {
        id: settingsButton

        height: 40
        width: 12
        color: AppStyle.accentColor
        z: 6
        anchors.top: parent.top
        anchors.right: parent.right
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                console.debug('open settings page')
            }
        }
    }

    MediaPlayer {
        id: mediaPlayer

        playlist: MediaPlaylist {
            id: playlist

            currentIndex: 0
            playbackMode: MediaPlaylist.Loop

            MediaItem { source: "file:///E:/Users/Yuri/Music/浜崎あゆみ(滨崎步)-Dearest.flac" }
            MediaItem { source: "file:///E:/Users/Yuri/Music/蔡琴-被遗忘的时光.wav" }
            MediaItem { source: "file:///E:/Users/Yuri/Music/邓紫棋-回忆的沙漏.wav" }
            MediaItem { source: "file:///E:/Users/Yuri/Music/小岩井ことり - ピカピカなのん.mp3" }
        }
    }

    BackgroundLayer {
        id: backgroundLayer

        z: 0
        anchors.fill: parent
    }

    DataLayer {
        id: dataLayer

        z: 1
        anchors.fill: parent
    }

    ControlLayer {
        id: controlLayer

        z: 5
        anchors.fill: parent
    }
}
