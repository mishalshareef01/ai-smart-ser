#!/bin/sh
# Compiles and diffs every tests/*.in against tests/*.expected
g++ -O2 -std=c++17 -o report report.cpp || exit 1
pass=0; fail=0
for f in tests/*.in; do
  if ./report < "$f" | diff -q - "${f%.in}.expected" >/dev/null; then pass=$((pass+1)); echo "PASS $f"; else fail=$((fail+1)); echo "FAIL $f"; fi
done
echo "$pass passed, $fail failed"
