// MissionMapView.qml - Map visualization for mission

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtLocation
import QtPositioning

Rectangle {
    id: mapContainer
    color: "#f5f5f5"

    property var waypoints: []
    property var perimeter: []
    property double droneLatitude: 37.7749
    property double droneLongitude: -122.4194
    property double droneAltitude: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5
        spacing: 5

        // Canvas for drawing map
        Canvas {
            id: mapCanvas
            Layout.fillWidth: true
            Layout.fillHeight: true

            property double minLat: 37.7740
            property double maxLat: 37.7770
            property double minLon: -122.4210
            property double maxLon: -122.4170
            property double scaleLat: height / (maxLat - minLat)
            property double scaleLon: width / (maxLon - minLon)

            onPaint: {
                var ctx = getContext("2d")

                // Clear canvas
                ctx.fillStyle = "#ffffff"
                ctx.fillRect(0, 0, width, height)

                // Draw grid
                ctx.strokeStyle = "#e0e0e0"
                ctx.lineWidth = 1
                var gridSpacing = 50
                for (var i = 0; i < width; i += gridSpacing) {
                    ctx.beginPath()
                    ctx.moveTo(i, 0)
                    ctx.lineTo(i, height)
                    ctx.stroke()
                }
                for (var j = 0; j < height; j += gridSpacing) {
                    ctx.beginPath()
                    ctx.moveTo(0, j)
                    ctx.lineTo(width, j)
                    ctx.stroke()
                }

                // Draw perimeter
                if (perimeter.length > 0) {
                    ctx.strokeStyle = "#FF5722"
                    ctx.lineWidth = 2
                    ctx.beginPath()

                    for (var i = 0; i < perimeter.length; i++) {
                        var lat = perimeter[i].latitude
                        var lon = perimeter[i].longitude
                        var x = (lon - mapCanvas.minLon) * mapCanvas.scaleLon
                        var y = (mapCanvas.maxLat - lat) * mapCanvas.scaleLat

                        if (i === 0) {
                            ctx.moveTo(x, y)
                        } else {
                            ctx.lineTo(x, y)
                        }
                    }

                    // Close the perimeter
                    if (perimeter.length > 0) {
                        var lat = perimeter[0].latitude
                        var lon = perimeter[0].longitude
                        var x = (lon - mapCanvas.minLon) * mapCanvas.scaleLon
                        var y = (mapCanvas.maxLat - lat) * mapCanvas.scaleLat
                        ctx.lineTo(x, y)
                    }

                    ctx.stroke()

                    // Draw corner markers
                    ctx.fillStyle = "#FF5722"
                    for (var i = 0; i < perimeter.length; i++) {
                        var lat = perimeter[i].latitude
                        var lon = perimeter[i].longitude
                        var x = (lon - mapCanvas.minLon) * mapCanvas.scaleLon
                        var y = (mapCanvas.maxLat - lat) * mapCanvas.scaleLat

                        ctx.fillRect(x - 4, y - 4, 8, 8)

                        // Draw index
                        ctx.fillStyle = "#000000"
                        ctx.font = "10px Arial"
                        ctx.fillText(i + 1, x + 8, y + 3)
                        ctx.fillStyle = "#FF5722"
                    }
                }

                // Draw waypoints
                if (waypoints.length > 0) {
                    ctx.strokeStyle = "#2196F3"
                    ctx.lineWidth = 1
                    ctx.beginPath()

                    for (var i = 0; i < waypoints.length; i++) {
                        var lat = waypoints[i].latitude
                        var lon = waypoints[i].longitude
                        var x = (lon - mapCanvas.minLon) * mapCanvas.scaleLon
                        var y = (mapCanvas.maxLat - lat) * mapCanvas.scaleLat

                        if (i === 0) {
                            ctx.moveTo(x, y)
                        } else {
                            ctx.lineTo(x, y)
                        }
                    }

                    ctx.stroke()

                    // Draw waypoint markers
                    ctx.fillStyle = "#2196F3"
                    for (var i = 0; i < waypoints.length; i++) {
                        var lat = waypoints[i].latitude
                        var lon = waypoints[i].longitude
                        var x = (lon - mapCanvas.minLon) * mapCanvas.scaleLon
                        var y = (mapCanvas.maxLat - lat) * mapCanvas.scaleLat

                        ctx.beginPath()
                        ctx.arc(x, y, 3, 0, 2 * Math.PI)
                        ctx.fill()
                    }
                }

                // Draw drone position
                var droneLon = droneLongitude
                var droneLat = droneLatitude
                var droneX = (droneLon - mapCanvas.minLon) * mapCanvas.scaleLon
                var droneY = (mapCanvas.maxLat - droneLat) * mapCanvas.scaleLat

                // Draw drone marker (green circle)
                ctx.fillStyle = "#4CAF50"
                ctx.beginPath()
                ctx.arc(droneX, droneY, 6, 0, 2 * Math.PI)
                ctx.fill()

                // Draw drone outline
                ctx.strokeStyle = "#2E7D32"
                ctx.lineWidth = 2
                ctx.stroke()

                // Draw altitude text
                ctx.fillStyle = "#000000"
                ctx.font = "11px Arial"
                ctx.fillText("Alt: " + droneAltitude.toFixed(1) + "m", droneX + 10, droneY - 5)
            }

            function updateWaypoints() {
                waypoints = missionBridge.getWaypoints()
                perimeter = missionBridge.getPerimeterCorners()
                requestPaint()
            }

            function setDronePosition(lat, lon, alt) {
                droneLatitude = lat
                droneLongitude = lon
                droneAltitude = alt
                requestPaint()
            }

            function updateDronePosition() {
                requestPaint()
            }
        }

        // Info panel
        Rectangle {
            color: "#f9f9f9"
            border.color: "#ddd"
            border.width: 1
            Layout.fillWidth: true
            Layout.preferredHeight: 60

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 15

                Text {
                    text: "Drone: " + mapContainer.droneLatitude.toFixed(6) + ", " +
                          mapContainer.droneLongitude.toFixed(6)
                    font.pixelSize: 11
                }

                Text {
                    text: "Altitude: " + mapContainer.droneAltitude.toFixed(1) + " m"
                    font.pixelSize: 11
                }

                Text {
                    text: "Waypoints: " + (waypoints ? waypoints.length : 0)
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                Button {
                    text: "Fit Map"
                    onClicked: {
                        // Auto-fit map bounds to perimeter
                        mapCanvas.requestPaint()
                    }
                }

                Button {
                    text: "Reset View"
                    onClicked: {
                        mapCanvas.minLat = 37.7740
                        mapCanvas.maxLat = 37.7770
                        mapCanvas.minLon = -122.4210
                        mapCanvas.maxLon = -122.4170
                        mapCanvas.requestPaint()
                    }
                }
            }
        }
    }
}
