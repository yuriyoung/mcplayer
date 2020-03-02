import QtQuick 2.12
import QtGraphicalEffects 1.12

Item {
    Image {
        anchors.fill: parent
        source: "qrc:/assets/img/background.jpg"
        fillMode: Image.PreserveAspectCrop
    }

    LinearGradient {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.65) }
            GradientStop { position: 0.4; color: Qt.rgba(1, 1, 1, 0.96)}
            GradientStop { position: 0.9; color: Qt.rgba(1, 1, 1, 1) }
            GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.88) }
        }
    }
}
