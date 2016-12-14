killall judge
make judge
echo compile_judge_complete
make search
echo compile_search_complete
rm -rf log_ju.txt log_p1.txt log_p2.txt
./judge 7125 10 > log_ju.txt &
sleep 1
./search_progressivepruning 127.0.0.1 7125 > log_p1.txt &
./search_uct 127.0.0.1 7125 > log_p2.txt &
