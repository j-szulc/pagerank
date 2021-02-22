./utils/profileProgram.sh ./tests/pageRankPerformanceTest output2.gdb
./utils/generateFlameChartSvg.sh output2.gdb result2.svg
./utils/profileProgram.sh "./tests/e2eTest 8" output.gdb tests/e2eScenario.txt
./utils/generateFlameChartSvg.sh output.gdb result.svg