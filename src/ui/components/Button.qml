import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import QtQuick.Controls.Material.impl 2.12

import "qrc:///styles/"

Button {
    id: control

    property bool rounded: false // Applies a more prominent border-radius for a squared shape button
    property bool loading: false
    property int duration: 1200
    property bool labeled: false // no icon
    property bool noCaps: false // void turning label text into caps
    property bool outline: false
    property bool ripple: true
    property bool iconRight: false
    property color color: AppStyle.defaultColor
    property color textColor: AppStyle.foregroundColor
    property string tooltip: ""

    property real __origWidth: control.implicitWidth
    property real __origHeight: control.implicitHeight
    Component.onCompleted: {  __origWidth = control.width; __origHeight = control.height}

    ToolTip.visible: hovered && tooltip.length > 0
    ToolTip.text: tooltip
    Material.elevation: flat ? control.down || control.hovered ? 2 : 0
                             : control.down ? 8 : 2
    Material.background: flat && enabled ? "transparent" : color

    contentItem: GridLayout {
        columns: 2
        columnSpacing: 2
        layoutDirection: iconRight ? Qt.RightToLeft : Qt.LeftToRight
        Text {
            id: buttonIcon
            Layout.alignment: loading ? Qt.AlignCenter : Qt.AlignVCenter
            visible: control.icon.name.length > 0 || control.display === AbstractButton.IconOnly
            text: loading ? AppIcons.mdi_loading : control.icon.name
            color: !control.enabled ? control.Material.hintTextColor : control.textColor
            font.family: AppIcons.family
            font.pixelSize: control.icon.width / 1.2
            RotationAnimation on rotation {
                running: control.loading
                from: 0
                to: 360
                loops: Animation.Infinite
                duration: control.duration
                onStopped: buttonIcon.rotation = 0
            }
        }
        Text { id: noCapsFont; visible: false }
        Text {
            id: buttonLabel
            rightPadding: 4
            Layout.alignment: control.loading ? Qt.AlignJustify : Qt.AlignVCenter | Qt.AlignLeft
            visible: loading ? false : control.display !== AbstractButton.IconOnly && control.text.length > 0
            text: control.text
            font: noCaps ? noCapsFont.font : control.font
            color: !control.enabled ? control.Material.hintTextColor : control.textColor
        }
    } // contentItem

    background: Rectangle {
        implicitWidth: control.loading ? control.__origWidth : control.Material.buttonHeight
        implicitHeight: control.Material.buttonHeight
        radius: control.rounded ? control.Material.buttonHeight : 2
        color: !control.enabled ? control.Material.buttonDisabledColor
                                : control.highlighted ? control.Material.highlightedButtonColor
                                                      : control.Material.buttonColor

        layer.enabled: control.enabled && control.Material.buttonColor.a > 0
        layer.effect: ElevationEffect {
            elevation: control.Material.elevation
        }

        Rectangle {
            id: outlineRect
            visible: outline
            width: parent.width
            height: parent.height
            color: "transparent"
            border.width: 1
            border.color: !control.enabled ? control.Material.buttonDisabledColor
                                           : control.textColor
        }

        //@disabled warning M300
        Ripple {
            id: ripple

            readonly property color _rippleColor: control.flat ? control.textColor : "white"
            readonly property real _opacity: control.flat ? 0.18 : 0.3

            visible: control.ripple
            clipRadius:control.rounded ? parent.height / 2 : 2
            width: parent.width
            height: parent.height
            pressed: control.pressed
            anchor: control
            active: control.hovered || control.down || control.visualFocus
            color: Qt.rgba(_rippleColor.r, _rippleColor.g,
                           _rippleColor.b, _opacity)
        }
    } // background
}
