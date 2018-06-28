CC=g++
CFLAG=-Wall -g -o
all:HW5 EvaluationProgram
HW5:HW5.cpp
	$(CC) HW5.cpp $(CFLAG) HW5
EvaluationProgram:EvaluationProgram.cpp
	$(CC) EvaluationProgram.cpp $(CFLAG) EvaluationProgram 
clean:
	rm EvaluationProgram HW4 result.txt Interpolated-Recall-Precision-curve.txt NDCG.txt Mean-Average-Precision.txt
