"""Hall Attendance Reconciliation - O(n) expected, using hash sets / dicts.

Usage:  python report.py [input_file]     (reads stdin if no file given)

Input lines:
  <Hall> <n> id1 .. idn   roster; position in list = seat number
  ABSENT id ...           declared absent IDs
  SCAN <Hall> <id>        scan event (rosters must come first)
  NOTE <id> <text>        malpractice note
"""
import sys


def reconcile(lines):
    halls = []                 # list of (name, [ids])  -> per-hall arrays
    where = {}                 # global map: ID -> (hall index, seat)
    first_scan_hall = {}       # ID -> hall of first scan
    scanned_own = set()        # IDs scanned in their assigned hall
    absent_order = []          # ABSENT ids in input order
    anomalies = []
    notes = []                 # (id, text)
    scan_mode = False

    for line in lines:
        parts = line.split()
        if not parts:
            continue
        tok = parts[0]

        if tok == "ABSENT":
            absent_order.extend(parts[1:])

        elif tok == "SCAN":
            if len(parts) < 3:
                anomalies.append("BAD_LINE " + line.strip())
                continue
            h, sid = parts[1], parts[2]
            scan_mode = True
            first = first_scan_hall.get(sid)
            if first is None:
                first_scan_hall[sid] = h
            elif first == h:
                anomalies.append(f"DUPLICATE_SCAN id={sid} hall={h}")
            else:
                anomalies.append(f"MULTI_HALL id={sid} halls={first},{h}")
            w = where.get(sid)
            if w is None:
                anomalies.append(f"UNKNOWN_ID id={sid} hall={h}")
            elif halls[w[0]][0] != h:
                anomalies.append(f"WRONG_HALL id={sid} scanned={h} assigned={halls[w[0]][0]}")
            else:
                scanned_own.add(sid)

        elif tok == "NOTE":
            if len(parts) < 2:
                anomalies.append("BAD_LINE " + line.strip())
                continue
            text = line.split(None, 2)[2].strip() if len(parts) > 2 else ""
            notes.append((parts[1], text))

        else:  # roster line
            try:
                n = int(parts[1])
            except (IndexError, ValueError):
                anomalies.append("BAD_LINE " + line.strip())
                continue
            ids = parts[2:]
            if len(ids) != n:
                anomalies.append(f"COUNT_MISMATCH hall={tok} declared={n} actual={len(ids)}")
            hi = len(halls)
            for i, sid in enumerate(ids):
                prev = where.setdefault(sid, (hi, i + 1))
                if prev != (hi, i + 1):
                    if prev[0] == hi:
                        anomalies.append(f"DUPLICATE_IN_ROSTER id={sid} hall={tok}")
                    else:
                        anomalies.append(f"ROSTER_CONFLICT id={sid} halls={halls[prev[0]][0]},{tok}")
            halls.append((tok, ids))

    declared_absent = set()
    for sid in absent_order:
        if sid not in where:
            anomalies.append(f"UNKNOWN_ABSENT id={sid}")
        elif sid in declared_absent:
            anomalies.append(f"DUPLICATE_ABSENT id={sid}")
        else:
            declared_absent.add(sid)

    out = []
    tot_p = tot_a = 0
    for h, (name, ids) in enumerate(halls):
        present = 0
        absent_list = []
        for i, sid in enumerate(ids):
            if where[sid] != (h, i + 1):
                continue  # skip duplicate / conflicting entries
            if sid in declared_absent or (scan_mode and sid not in scanned_own):
                absent_list.append(f"{sid}@S{i + 1}")
            else:
                present += 1
        out.append(f"{name} present={present} absent={len(absent_list)} [{', '.join(absent_list)}]")
        tot_p += present
        tot_a += len(absent_list)
    out.append(f"TOTAL present={tot_p} absent={tot_a}")

    if anomalies:
        out.append("ANOMALIES")
        out.extend("  " + a for a in anomalies)
    if notes:
        out.append("NOTES")
        for sid, text in notes:
            w = where.get(sid)
            loc = f"({halls[w[0]][0]}@S{w[1]})" if w else "(unassigned)"
            out.append(f"  NOTE {sid} {loc}: {text}")
    return "\n".join(out) + "\n"


def main():
    if len(sys.argv) > 1:
        with open(sys.argv[1]) as f:
            sys.stdout.write(reconcile(f))
    else:
        sys.stdout.write(reconcile(sys.stdin))


if __name__ == "__main__":
    main()
