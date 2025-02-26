#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <vector>
#include <iomanip>

long double calculateDistance(long double lat1, long double lon1, long double lat2, long double lon2) {
    const long double earthRadius = 6372.795;
    long double angularDistance;
    
    angularDistance = 2 * asin(sqrt(sin((lat2 - lat1) / 2) * sin((lat2 - lat1) / 2) +
        cos(lat1) * cos(lat2) * sin(fabs(lon1 - lon2) / 2) * sin(fabs(lon1 - lon2) / 2)));

    return angularDistance * earthRadius;
}

struct Point {
    long double lat;
    long double lon;
};

vector<Point> parseKML(const string& filename) {
    vector<Point> track;
    ifstream file(filename);
    string line;
    string startTag = "<coordinates>";
    string endTag = "</coordinates>";
    bool calc = false;
    
    while (getline(file, line)) {
        size_t startPos = line.find(startTag);
        if (startPos != string::npos) {
            line = line.substr(startPos + startTag.size());
            calc = true;
        }

        size_t endPos = line.find(endTag);
        bool stop = false;
        if (endPos != string::npos) {
            line = line.substr(0, endPos);
            stop = true;
        }

        if (calc) {
            istringstream ss(line);
            long double lon, lat;
            char c;
            while (ss >> lon >> c >> lat) {
                lat = lat * M_PI / 180.0;
                lon = lon * M_PI / 180.0;
                track.push_back({lat, lon});
            }
        }

        if (stop) {
            calc = false;
        }
    }
    
    return track;
}

void writeKML(const vector<Point>& filteredTrack, const string& filename) {
    ofstream file(filename);
    file << R"(<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://earth.google.com/kml/2.2">
    <Document>
        <name>FilteredTrack</name>
        <Style>
        <PolyStyle>
            <fill>0</fill>
            <outline>1</outline>
        </PolyStyle>
        </Style>
    <Placemark>
    <name>1</name>
    <MultiGeometry>
    <LineString>
        <coordinates>
    )";
    bool fl = true;
    for (const auto& p : filteredTrack) {
        if (!fl) {
            file << " ";
        }
        file << fixed << setprecision(9) << p.lon * 180.0 / M_PI << "," << p.lat * 180.0 / M_PI;
        fl = false;
    }

    file << R"(
        </coordinates>
    </LineString>
    </MultiGeometry>
</Placemark>
</Document>
</kml>
    )";
}

vector<Point> filterTrack(const vector<Point>& track, long double distance) {
    vector<Point> filteredTrack;
    if (track.empty()) {
        return filteredTrack;
    }
    
    filteredTrack.push_back(track[0]);
    for (size_t i = 1; i < track.size(); i++) {
        long double s = calculateDistance(filteredTrack.back().lat, filteredTrack.back().lon, track[i].lat, track[i].lon);

        if (s >= distance) {
            filteredTrack.push_back(track[i]);
        }
    }

    return filteredTrack;
}

int main(int argc, char* argv[]) {

    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <input KML file> <output KML file> <min distance>" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];
    long double distance = atof(argv[3]);

    vector<Point> track = parseKML(inputFile);
    vector<Point> filteredTrack = filterTrack(track, distance);
    writeKML(filteredTrack, outputFile);

    cout << "Number of points in the original track: " << track.size() << endl;
    cout << "Number of points in the filtered track: " << filteredTrack.size() << endl;
    cout << "Minimum distance between points: " << distance << " km" << endl;

    return 0;
}
