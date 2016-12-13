killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
rm -rf log_ju.txt log_p1.txt log_p2.txt
./judge 7123 4 > log_ju.txt &
./search_uct 127.0.0.1 7123 > log_p1.txt &
./search 127.0.0.1 7123 > log_p2.txt &