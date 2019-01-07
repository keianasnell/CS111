#### PART 1 README QUESTIONS

 **2.1.1 - causing conflicts:**

*Why does it take many iterations before errors are seen?*
There is a higher chance for race conditions to occur when the number of iterations increase. The higher the number of threads/iterations = the higher the chances of these collisions increase =  increase in the number of errors.

*Why does a significantly smaller number of iterations so seldom fail?*
Smaller numbers of iterations means that the chance the two threads reaching a race condition during execution is low. While this is low parallelization, it's more likely that a thread finish its job and exit before the next one reaches this code. The increase in iteration size is what increases the chance of these collisions.



**QUESTION 2.1.2 - cost of yielding:**

*Why are the --yield runs so much slower?*
--yield runs are much slower because the program voluntarily gives up its cycle, forcing a context switch.  These context switch switches threads, which is very costly in terms of time.

*Where is the additional time going?*
This additional time is going to the cost of the context switch, which includes: removing first thread's stack and heap off the main stack, saving its registers and program counters, and replacing them with the second thread's. 

*Is it possible to get valid per-operation timings if we are using the --yield option? If so, explain how. If not, explain why not.*
No, because the cost of context switching is way more than that of an add operation. This makes the add operation negligible; when you try and determine per-operation timing, it wouldn't be accurate anymore because the elapsed run time wouldn't be related to the actual operation.



**QUESTION 2.1.3 - measurement errors:**

*Why does the average cost per operation drop with increasing iterations?*
With increasing iterations, the initial costs (like creating and joining threads) are amortized by the amount of operations to actually perform. Thus, the average cost per operation drops.


*If the cost per iteration is a function of the number of iterations, how do we know how many iterations to run (or what the "correct" cost is)?*
If you run as many iterations as possible, you would determine when the initial costs become negligible enough that the run time is truly from executing the operation.  



**QUESTION 2.1.4 - costs of serialization:**

*Why do all of the options perform similarly for low numbers of threads?*
There's less chance for collisions and race conditions when you have low numbers of threads. There's now less time for the threads to be competing for locks and now are spending more time on the actual operations, thus equalizing the overhead cost.


*Why do the three protected operations slow down as the number of threads rises?*
The three protected operations slow down as the number of threads rises because  the chance of collisions and race conditions rises with the increase in number of threads. The threads will have to wait for locks more often (instead of actually performing operations), so it slows everything down.



#### PART 2 README QUESTIONS

**QUESTION 2.2.1 - scalability of Mutex**:

*Compare the variation in time per mutex-protected operation vs the number of threads in Part-1 (adds) and Part-2 (sorted lists).*
As the number of threads increases in both graphs, so does the time per operation. It then plateaus at a certain pointin which the increase in time per operation levels out.

*Comment on the general shapes of the curves, and explain why they have this shape.*
At some point, both curves seem to level off. With more threads, the amount of time waiting for the lock increases. At a certain point, this increase in overhead becomes negligible, so additional threads do not add overhead past this point.

*Comment on the relative rates of increase and differences in the shapes of the curves, and offer an explanation for these differences.*
The rate of increase in time is greater for the list curve because its operations being performed are more complicated. The point where the curve stabilizes is also later with increased numbers of threads for a similar reason.



**QUESTION 2.2.2 - scalability of spin locks:**

***Compare the variation in time per protected operation vs the number of threads for list operations protected by Mutex vs Spin locks.* 
The spin locks do not level off and decrease in increase rate; instead, it increases directly relative to the number of threads.


*Comment on the general shapes of the curves, and explain why they have this shape.*
As the number mutex threads increase, they don't as much overhead per thread, so the time per operation levels off. In the beginning, the spin lock is cheaper in terms of time per operation, but slowly passes mutex as the thread number increases. The spin lock increases the time per operation because it adds overhead.

*Comment on the relative rates of increase and differences in the shapes of the curves, and offer an explanation for these differences*
Mutex rates of increase eventually decrease and flatten out. Spin lock continues to increase in time, and even slowly increases in rate. Spin locks will cause threads to spin and waste processing cycles if they need to wait for the locks. 
Spin locks are less expensive in the beginning because they do not need to be initialized; however, the mutex lock needs to be first initialized and that adds overhead. 

