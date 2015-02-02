import QtQuick 1.1

Rectangle {
    id: aboutPageRoot

    Column {
        anchors {
            fill: parent
            margins: 20
        }
        spacing: 20

        Row {
            width: parent.width
            spacing: 20
            Column {
                Image {
                    source: "CamTest64.png"
                    sourceSize.width: 64
                    sourceSize.height: 64
                }
            }
            Column {
                Label {
                    width: parent.width
                    text: qsTr("CamTest 1");
                    font.pixelSize: platformStyle.fontSizeLarge
                }

                Label {
                    width: parent.width
                    text: qsTr("Version") + ": " + version_number;
                    font.pixelSize: platformStyle.fontSizeMedium
                }

                Label {
                    width: parent.width
                    text: qsTr("Compiled on") + ": " + compiled_on
                    font.pixelSize: platformStyle.fontSizeSmall
                }
            }
        }

        Label {
            width: parent.width
            property string openUrl: "http://twitter.com/riussi"
            textFormat: Text.RichText
            text: qsTr("Author") + ': Juha Ristolainen (<a href="http://twitter.com/riussi/">@Riussi</a>)'
            font.pixelSize: platformStyle.fontSizeMedium
            onLinkActivated: {
                Qt.openUrlExternally(openUrl);
            }
        }

        Label {
            width: parent.width
            text: qsTr("You can use this code freely for whatever purpose you want.");
            wrapMode: Text.Wrap
            font.pixelSize: platformStyle.fontSizeMedium
        }
    }
}
