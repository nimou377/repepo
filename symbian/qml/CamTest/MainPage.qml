import QtQuick 1.1

import Codemancers 1.0

Rectangle {
    id: mainPage
  //  orientationLock: PageOrientation.LockPortrait

    property int frameCount: 0
    property int processedCount: 0

    Column {
        spacing: 24
        anchors {
            fill: parent
            topMargin: 12
            bottomMargin: 12
        }

        Label {
            width: parent.width
            text: qsTr("CamTest")
            font.pixelSize: 38
            color: "lightblue"
        }

        ViewFinder
        {
            id: viewFinder
            width: parent.width
            height: width*3/4
            Component.onCompleted: {
                console.debug("ViewFinder QML-component loaded");
            }
            onCameraErrorChanged: {
                console.debug("Camera error: " + error);
                switch (error) {
                case 0:
                    errorLabel.text = qsTr("No error");
                    break;
                case 1:
                    errorLabel.text = qsTr("Camera error");
                    break;
                case 2:
                    errorLabel.text = qsTr("Invalid request");
                    break;
                case 3:
                    errorLabel.text = qsTr("Camera service not available");
                    break;
                case 4:
                    errorLabel.text = qsTr("Feature not supported");
                    break;
                default:
                    break;
                }
            }

            onCameraStatusChanged: {
                console.debug("Camera status changed: " + status);
                switch (status) {
                case 0:
                    statusLabel.text = qsTr("Camera unavailable");
                    break;
                case 1:
                    statusLabel.text = qsTr("Camera unloaded");
                    break;
                case 2:
                    statusLabel.text = qsTr("Loading camera");
                    break;
                case 3:
                    statusLabel.text = qsTr("Camera loaded");
                    break;
                case 4:
                    statusLabel.text = qsTr("Camera on standby");
                    break;
                case 5:
                    statusLabel.text = qsTr("Starting camera");
                    break;
                case 6:
                    statusLabel.text = qsTr("Camera active");
                    break;
                default:
                    statusLabel.text = "";
                    break;
                }
            }

            onFrameCountChanged: {
                mainPage.frameCount = count;
            }
            onProcessedCountChanged: {
                mainPage.processedCount = count;
            }

            Label {
                id: statusLabel
                anchors.centerIn: parent
                text: ""
                font.pixelSize: 32
                color: "white"
                visible: viewFinder.cameraStatus <= 6
            }
            Label {
                id: errorLabel
                anchors.centerIn: parent
                text: ""
                font.pixelSize: 32
                color: "red"
                visible: viewFinder.cameraError !== 0
            }
        }

        Label {
            id: frameCounterLabel
            text: frameCount + " received, " + processedCount + " processed"
        }

        Button {
            id: cameraButton
            width: parent.width
            text: viewFinder.cameraStatus < 2 ? qsTr("Start ViewFinder") : qsTr("Stop ViewFinder");
            onClicked: viewFinder.cameraStatus < 2 ? viewFinder.startCamera() : viewFinder.stopCamera();
            enabled: viewFinder.cameraStatus <= 2 || viewFinder.cameraStatus === 6
        }
    }
}
