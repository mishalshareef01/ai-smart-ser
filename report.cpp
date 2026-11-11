// Hall Attendance Reconciliation - O(n) expected, hash sets / hash maps.
// Build: g++ -O2 -std=c++17 -o report report.cpp
// Usage: ./report < input.txt
#include <bits/stdc++.h>
using namespace std;

struct Hall {
    string name;
    vector<string> ids;  // per-hall array of student IDs; index+1 = seat number
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<Hall> halls;
    unordered_map<string, pair<int, int>> where;  // global map: ID -> (hall index, seat)
    unordered_map<string, string> firstScanHall;  // ID -> hall of first scan
    unordered_set<string> scannedOwn;             // IDs scanned in their assigned hall
    unordered_set<string> declaredAbsent;         // IDs from ABSENT lines
    vector<string> absentOrder;                   // ABSENT ids in input order
    vector<string> anomalies;
    vector<pair<string, string>> notes;           // (id, text)
    bool scanMode = false;

    string line;
    while (getline(cin, line)) {
        istringstream ss(line);
        string tok;
        if (!(ss >> tok)) continue;  // blank line

        if (tok == "ABSENT") {
            string id;
            while (ss >> id) absentOrder.push_back(id);
        } else if (tok == "SCAN") {
            string h, id;
            if (!(ss >> h >> id)) { anomalies.push_back("BAD_LINE " + line); continue; }
            scanMode = true;
            auto it = firstScanHall.find(id);
            if (it == firstScanHall.end()) firstScanHall[id] = h;
            else if (it->second == h) anomalies.push_back("DUPLICATE_SCAN id=" + id + " hall=" + h);
            else anomalies.push_back("MULTI_HALL id=" + id + " halls=" + it->second + "," + h);

            auto w = where.find(id);
            if (w == where.end()) {
                anomalies.push_back("UNKNOWN_ID id=" + id + " hall=" + h);
            } else if (halls[w->second.first].name != h) {
                anomalies.push_back("WRONG_HALL id=" + id + " scanned=" + h +
                                    " assigned=" + halls[w->second.first].name);
            } else {
                scannedOwn.insert(id);
            }
        } else if (tok == "NOTE") {
            string id, text;
            ss >> id;
            getline(ss, text);
            size_t p = text.find_first_not_of(' ');
            notes.emplace_back(id, p == string::npos ? "" : text.substr(p));
        } else {  // hall roster line: <hall> <count> id1..idn
            Hall hall;
            hall.name = tok;
            int n;
            if (!(ss >> n)) { anomalies.push_back("BAD_LINE " + line); continue; }
            string id;
            while (ss >> id) hall.ids.push_back(id);
            if ((int)hall.ids.size() != n)
                anomalies.push_back("COUNT_MISMATCH hall=" + hall.name + " declared=" +
                                    to_string(n) + " actual=" + to_string(hall.ids.size()));
            int hi = halls.size();
            for (int i = 0; i < (int)hall.ids.size(); i++) {
                const string& s = hall.ids[i];
                auto r = where.emplace(s, make_pair(hi, i + 1));
                if (!r.second) {
                    int ph = r.first->second.first;
                    if (ph == hi)
                        anomalies.push_back("DUPLICATE_IN_ROSTER id=" + s + " hall=" + hall.name);
                    else
                        anomalies.push_back("ROSTER_CONFLICT id=" + s + " halls=" +
                                            halls[ph].name + "," + hall.name);
                }
            }
            halls.push_back(move(hall));
        }
    }

    // Resolve explicit ABSENT list
    for (const string& id : absentOrder) {
        if (!where.count(id)) { anomalies.push_back("UNKNOWN_ABSENT id=" + id); continue; }
        if (!declaredAbsent.insert(id).second) anomalies.push_back("DUPLICATE_ABSENT id=" + id);
    }

    long long totP = 0, totA = 0;
    for (int h = 0; h < (int)halls.size(); h++) {
        long long present = 0, absent = 0;
        string list;
        for (int i = 0; i < (int)halls[h].ids.size(); i++) {
            const string& id = halls[h].ids[i];
            if (where[id] != make_pair(h, i + 1)) continue;  // skip duplicate/conflicting entries
            bool isAbsent = declaredAbsent.count(id) || (scanMode && !scannedOwn.count(id));
            if (isAbsent) {
                if (absent++) list += ", ";
                list += id + "@S" + to_string(i + 1);
            } else {
                present++;
            }
        }
        cout << halls[h].name << " present=" << present << " absent=" << absent << " [" << list << "]\n";
        totP += present;
        totA += absent;
    }
    cout << "TOTAL present=" << totP << " absent=" << totA << "\n";

    if (!anomalies.empty()) {
        cout << "ANOMALIES\n";
        for (const string& a : anomalies) cout << "  " << a << "\n";
    }
    if (!notes.empty()) {
        cout << "NOTES\n";
        for (auto& [id, text] : notes) {
            cout << "  NOTE " << id;
            auto w = where.find(id);
            if (w != where.end())
                cout << " (" << halls[w->second.first].name << "@S" << w->second.second << ")";
            else
                cout << " (unassigned)";
            cout << ": " << text << "\n";
        }
    }
    return 0;
}
