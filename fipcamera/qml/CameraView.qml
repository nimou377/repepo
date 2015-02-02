/**
 * Copyright (c) 2012 Nokia Corporation.
 * Copyright (c) 2012 Harald Meyer, http://www.meh.at
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import CustomElements 1.0

Item {
    id: cameraView
    focus: true

    property CustomCamera camera
    property string currentDevice

    /*!
      Starts the primary camera.
    */
    function start() {
        var devices = camera.availableDevices;

        // Start primary camera
        currentDevice = devices[0];
        camera.start(currentDevice);
    }

    /*!
      Performs the auto focus.
    */
    function focusToCoordinate(x, y) {
        camera.focusToCoordinate(x, y);
    }

    /*!
      Requests camera to capture a image.
    */
    function captureImage() {
        camera.captureImage();
    }

    MouseArea {
        id: focusArea
        anchors.fill: parent
        onClicked: { cameraView.focusToCoordinate(mouse.x, mouse.y); }
    }
}
