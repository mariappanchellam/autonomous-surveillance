// main.qml - Main QML interface for Autonomous Mission System

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 1200
    height: 800
    title: "ArduPilot Autonomous Mission - " + missionBridge.executionMode

    color: "#f0f0f0"

    // C++ Bridge connection
    Connections {
        target: missionBridge
        function onMissionStateChanged() {
            statusPanel.updateStatus()
        }
        function onWaypointChanged() {
            mapView.updateDronePosition()
        }
        function onWaypointsUpdated() {
            mapView.updateWaypoints()
        }
        function onErrorOccurred(error) {
            errorDialog.showError(error)
        }
        function onDronePositionUpdated(lat, lon, alt) {
            mapView.setDronePosition(lat, lon, alt)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // Header
        Rectangle {
            color: "#2196F3"
            height: 50
            radius: 5
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15

                Text {
                    text: "Autonomous Mission Planner"
                    color: "white"
                    font.pixelSize: 18
                    font.bold: true
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "Mode: " + missionBridge.executionMode
                    color: "white"
                    font.pixelSize: 12
                }

                ComboBox {
                    model: ["Simulate", "Real"]
                    currentIndex: missionBridge.executionMode === "Simulate" ? 0 : 1
                    onCurrentIndexChanged: {
                        missionBridge.setMode(currentIndex === 0)
                    }
                }
            }
        }

        // Main content area
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10

            // Left Panel - Controls
            Rectangle {
                color: "white"
                radius: 5
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                border.color: "#ddd"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    // Control Panel
                    GroupBox {
                        title: "Mission Control"
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 8

                            Button {
                                text: "Initialize Mission"
                                Layout.fillWidth: true
                                onClicked: {
                                    let mode = missionBridge.executionMode === "Simulate"
                                    missionBridge.initializeMission(mode)
                                }
                            }

                            Button {
                                text: "Generate Mission"
                                Layout.fillWidth: true
                                onClicked: missionBridge.generateMission()
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Button {
                                    text: "Start"
                                    Layout.fillWidth: true
                                    onClicked: missionBridge.startMission()
                                }
                                Button {
                                    text: "Pause"
                                    Layout.fillWidth: true
                                    onClicked: missionBridge.pauseMission()
                                }
                            }

                            Button {
                                text: "Resume"
                                Layout.fillWidth: true
                                onClicked: missionBridge.resumeMission()
                            }

                            Button {
                                text: "Cancel"
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: "#f44336"
                                    radius: 3
                                }
                                onClicked: missionBridge.cancelMission()
                            }
                        }
                    }

                    // Perimeter Input
                    GroupBox {
                        title: "Perimeter (Square)"
                        Layout.fillWidth: true

                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 5

                            TextField {
                                id: lat1Input
                                placeholderText: "Corner 1 Lat"
                                Layout.fillWidth: true
                                text: "37.7749"
                            }
                            TextField {
                                id: lon1Input
                                placeholderText: "Corner 1 Lon"
                                Layout.fillWidth: true
                                text: "-122.4194"
                            }

                            TextField {
                                id: lat2Input
                                placeholderText: "Corner 2 Lat"
                                Layout.fillWidth: true
                                text: "37.7760"
                            }
                            TextField {
                                id: lon2Input
                                placeholderText: "Corner 2 Lon"
                                Layout.fillWidth: true
                                text: "-122.4194"
                            }

                            TextField {
                                id: lat3Input
                                placeholderText: "Corner 3 Lat"
                                Layout.fillWidth: true
                                text: "37.7760"
                            }
                            TextField {
                                id: lon3Input
                                placeholderText: "Corner 3 Lon"
                                Layout.fillWidth: true
                                text: "-122.4185"
                            }

                            TextField {
                                id: lat4Input
                                placeholderText: "Corner 4 Lat"
                                Layout.fillWidth: true
                                text: "37.7749"
                            }
                            TextField {
                                id: lon4Input
                                placeholderText: "Corner 4 Lon"
                                Layout.fillWidth: true
                                text: "-122.4185"
                            }

                            Button {
                                text: "Set Perimeter"
                                Layout.fillWidth: true
                                onClicked: {
                                    missionBridge.setPerimeter(
                                        parseFloat(lat1Input.text),
                                        parseFloat(lon1Input.text),
                                        parseFloat(lat2Input.text),
                                        parseFloat(lon2Input.text),
                                        parseFloat(lat3Input.text),
                                        parseFloat(lon3Input.text),
                                        parseFloat(lat4Input.text),
                                        parseFloat(lon4Input.text)
                                    )
                                }
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }

            // Right Panel - Map and Info
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                // Map
                Rectangle {
                    color: "white"
                    radius: 5
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    border.color: "#ddd"
                    border.width: 1

                    MissionMapView {
                        id: mapView
                        anchors.fill: parent
                    }
                }

                // Status Panel
                MissionStatusPanel {
                    id: statusPanel
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                }
            }
        }
    }

    // Error Dialog
    Dialog {
        id: errorDialog
        modal: true
        anchors.centerIn: parent
        width: 400

        function showError(message) {
            errorText.text = message
            open()
        }

        ColumnLayout {
            anchors.fill: parent
            Text {
                id: errorText
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }
            Button {
                text: "OK"
                Layout.fillWidth: true
                onClicked: errorDialog.close()
            }
        }
    }
}
