CC=g++
CFLAG=-Wall -g -o
all:HW3 EvaluationProgram
HW3:HW3.cpp
	$(CC) HW3.cpp $(CFLAG) HW3
EvaluationProgram:EvaluationProgram.cpp
	$(CC) EvaluationProgram.cpp $(CFLAG) EvaluationProgram 
clean:
	rm EvaluationProgram HW3 result.txt Interpolated-Recall-Precision-curve.txt NDCG.txt Mean-Average-Precision.txt
