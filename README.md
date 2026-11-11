# Hall Attendance Reconciliation

Streams exam-hall attendance, reports absentees with seat positions per hall, and prints a department total.
Detects duplicate scans, a student scanned in two halls, and a student scanned in a hall other than the assigned one.
Expected O(n) time using hash sets / hash maps (one pass over input, one pass over rosters).

## Build & run
    g++ -O2 -std=c++17 -o report report.cpp
    ./report < tests/t1_sample.in
    ./run_tests.sh          # runs all 6 test cases

## Input (one record per line)
| Line | Meaning |
|---|---|
| `<Hall> <n> id1 .. idn` | Roster. Position in the list = seat number (S1..Sn). |
| `ABSENT id ...` | IDs declared absent (may repeat across lines). |
| `SCAN <Hall> <id>` | Scan event (8-hour extension). Rosters must appear before scans. |
| `NOTE <id> <text>` | Malpractice note attached to a student. |

## Output
    H1 present=3 absent=2 [102@S2, 104@S4]
    H2 present=2 absent=1 [202@S2]
    TOTAL present=5 absent=3
followed by optional `ANOMALIES` and `NOTES` sections (nothing extra printed if none, so the sample output matches exactly).

## Data structures
- Per-hall `vector<string>` of IDs (seat = index + 1)
- Global `unordered_map<id, (hall, seat)>` : ID -> hall (and seat)
- `unordered_set` of declared-absent IDs and of IDs scanned in their own hall
- `unordered_map` of first-scan hall per ID, for duplicate / multi-hall detection

## Rules and assumptions
- Present = roster seats - absent. Absent = IDs in `ABSENT`, plus (only if the input contains any `SCAN` line) roster students with no scan in their assigned hall.
- A scan in a wrong hall is flagged `WRONG_HALL` and does **not** count as present in the assigned hall.
- Anomaly codes: `DUPLICATE_SCAN`, `MULTI_HALL`, `WRONG_HALL`, `UNKNOWN_ID`, `UNKNOWN_ABSENT`, `DUPLICATE_ABSENT`, `ROSTER_CONFLICT` (ID on two rosters; first hall wins), `DUPLICATE_IN_ROSTER`, `COUNT_MISMATCH` (actual count used), `BAD_LINE`.
- IDs are treated as strings.

## Tests
| File | Covers |
|---|---|
| t1_sample | Sample from the problem statement |
| t2_no_absent | No ABSENT line, empty absent list |
| t3_dup_unknown_absent | Repeated and unknown absent IDs |
| t4_roster_conflict | Student on two rosters |
| t5_scan_anomalies | SCAN mode: duplicate, wrong hall, multi-hall, unknown ID, NOTE |
| t6_empty_absent_and_count_mismatch | Empty `ABSENT`, declared count != actual |

Performance: 10^6 random SCAN events over 10^6 students (many duplicate-scan anomalies reported) finished in about 3 s here; time is linear in input size.
