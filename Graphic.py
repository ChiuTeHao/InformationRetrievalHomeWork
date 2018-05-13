import matplotlib.pyplot as plt
import numpy as np
irpcfile=open("Interpolated-Recall-Precision-curve.txt","r")
y1=[]
for val in irpcfile.read().split():
    y1.append(float(val))
x1=np.linspace(0,100,11)
fig1=plt.figure()
fig1.suptitle('Interpolated Recall-Precision Curve')
plt.xlim(0,100)
plt.ylim(0,100)
plt.xlabel('Recall')
plt.ylabel('Precision')
plt.plot(x1,y1)
irpcfile.close()

ndcgfile=open("NDCG.txt","r")
y2=[]
for val in ndcgfile.read().split():
    y2.append(float(val))
fig2=plt.figure()
x2=np.linspace(0,2500,2265)
fig2.suptitle("Normalized Discounted Cumulated Gain")
plt.xlim(0,2500)
plt.ylim(0,1.0)
plt.plot(x2,y2)
ndcgfile.close()

plt.show()
