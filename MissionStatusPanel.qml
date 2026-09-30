// MissionStatusPanel.qml - Status and statistics display

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: statusPanel
    color: "white"
    radius: 5
    border.color: "#ddd"
    border.width: 1

    function updateStatus() {
        statusText.text = missionBridge.getStatusString()
        progressBar.value = missionBridge.getMissionProgress()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // Title and status
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Mission Status"
                font.bold: true
                font.pixelSize: 14
            }

            Text {
                id: statusText
                text: "Idle"
                color: "#1976D2"
                font.pixelSize: 12
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
            }
        }

        // Progress bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "Progress:"
                font.pixelSize: 11
            }

            ProgressBar {
                id: progressBar
                Layout.fillWidth: true
                value: 0.0
                from: 0
                to: 100
            }

            Text {
                id: progressText
                text: "0%"
                font.pixelSize: 11
                width: 30
            }
        }

        // Statistics grid
        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 15
            rowSpacing: 8

            // Column 1
            Text {
                text: "Total Distance:"
                font.pixelSize: 10
                color: "#666"
            }
            Text {
                id: distanceText
                text: (missionBridge.totalDistance / 1000.0).toFixed(2) + " km"
                font.pixelSize: 10
                font.bold: true
            }

            // Column 2
            Text {
                text: "Est. Time:"
                font.pixelSize: 10
                color: "#666"
            }
            Text {
                id: timeText
                text: {
                    var minutes = Math.floor(missionBridge.estimatedTime / 60)
                    var seconds = Math.floor(missionBridge.estimatedTime % 60)
                    return minutes + "m " + seconds + "s"
                }
                font.pixelSize: 10
                font.bold: true
            }

            // Column 3
            Text {
                text: "Waypoints:"
                font.pixelSize: 10
                color: "#666"
            }
            Text {
                id: waypointsText
                text: missionBridge.totalWaypoints
                font.pixelSize: 10
                font.bold: true
            }

            // Column 4
            Text {
                text: "Area:"
                font.pixelSize: 10
                color: "#666"
            }
            Text {
                id: areaText
                text: (missionBridge.perimeterArea / 1000000.0).toFixed(2) + " km²"
                font.pixelSize: 10
                font.bold: true
            }
        }

        Item { Layout.fillHeight: true }

        // Export/Import buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Export Mission"
                Layout.fillWidth: true
                onClicked: {
                    // TODO: Open file dialog
                    missionBridge.exportMission("/tmp/mission.dat")
                }
            }

            Button {
                text: "Import Mission"
                Layout.fillWidth: true
                onClicked: {
                    // TODO: Open file dialog
                    missionBridge.importMission("/tmp/mission.dat")
                }
            }
        }
    }

    Connections {
        target: missionBridge
        function onStatisticsUpdated() {
            distanceText.text = (missionBridge.totalDistance / 1000.0).toFixed(2) + " km"
            timeText.text = {
                var minutes = Math.floor(missionBridge.estimatedTime / 60)
                var seconds = Math.floor(missionBridge.estimatedTime % 60)
                return minutes + "m " + seconds + "s"
            }
            waypointsText.text = missionBridge.totalWaypoints
            areaText.text = (missionBridge.perimeterArea / 1000000.0).toFixed(2) + " km²"
        }
        function onMissionProgress(progress) {
            progressBar.value = progress
            progressText.text = progress.toFixed(0) + "%"
        }
    }
}
