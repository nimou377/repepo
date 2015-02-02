/**
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

import QtQuick 2.0
import QtQuick.Window 2.0

import CustomElements 1.0

Window {
    objectName: "mainWindow"
    id: mainWindow


    signal exitApp();

    Rectangle {
        id: mainPage
        objectName: "mainPage"
        anchors.fill: parent


        Component.onCompleted: {
            // Start camera and set threshold slider position
            cameraView.start();
//            sldThreshold.value = camera.effectThreshold;
        }

        // Black background
        Rectangle {
            id: bgRectangle
            anchors.fill: parent
            color: "#000000"
        }

        CustomCamera {
            id: camera
            anchors.fill: parent


        }

        // The CameraView provides convenience methods for accessing the camera.
        CameraView {
            id: cameraView
            anchors.fill: parent
            camera: camera
        }
    }
}

