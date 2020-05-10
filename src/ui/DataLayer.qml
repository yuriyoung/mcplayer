import QtQuick 2.12
import QtQuick.Controls 2.12

import org.mcplayer 1.0
import "qrc:///styles/"
import "qrc:///components/" as COM
import "qrc:///player/" as PLAYER

Item {
    id: control

    PLAYER.Playlist {
        id: playlistView

        anchors {
           top: parent.top; left: parent.left; bottom: parent.bottom;
        }
    }

    DropArea {
        id: dropArea

        property bool dropable: false
        anchors.fill: parent

        onEntered: {
            dropable = true;
            drag.accepted = true;
        }
        onExited: {
            dropable = false;
            drag.accepted = false;
        }
        onDropped: {
            if (drop.proposedAction == Qt.MoveAction || drop.proposedAction == Qt.CopyAction)
            {
                if(drop.hasUrls) {
                    mediaPlayer.playlist.addItems(drop.urls);
                    dropable = false;
                }
            }
        }
    }
}
