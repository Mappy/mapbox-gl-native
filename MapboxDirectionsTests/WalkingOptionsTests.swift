import XCTest
import MapboxDirections

class WalkingOptionsTests: XCTestCase {
    
    func testURLQueryParams() {
        
        let waypoints = [
            Waypoint(coordinate: CLLocationCoordinate2D(latitude: 0, longitude: 1)),
            Waypoint(coordinate: CLLocationCoordinate2D(latitude: 2, longitude: 3))
        ]
        
        let options = WalkingOptions(waypoints: waypoints)
        var queryItems = options.urlQueryItems
        
        XCTAssertEqual(queryItems.filter { $0.name == "alley_bias" }.first?.value, "0.0")
        XCTAssertEqual(queryItems.filter { $0.name == "walkway_bias" }.first?.value, "0.0")
        XCTAssertEqual(queryItems.filter { $0.name == "walking_speed" }.first?.value, "1.42")
        
        options.alleyBias = 0.4
        options.walkwayBias = 0.5
        options.walkingSpeed = 5.2
        
        queryItems = options.urlQueryItems
        
        XCTAssertEqual(queryItems.filter { $0.name == "alley_bias" }.first?.value, "0.4")
        XCTAssertEqual(queryItems.filter { $0.name == "walkway_bias" }.first?.value, "0.5")
        XCTAssertEqual(queryItems.filter { $0.name == "walking_speed" }.first?.value, "5.2")
    }
}
