Questions

\----------------------

QUESTION 2.3.1:

Where do you believe most of the CPU time is spent in the 1 and 2-thread list tests ?

​	Most of the CPU time is spent in the actual list operations with the 1 and 2 thread tests.

Why do you believe these to be the most expensive parts of the code?

​	Because there are so few threads, there isn't the overhead cost to maintain context switching or waiting for locks. The majority of the time is thus spent on actually performing the operation

Where do you believe most of the CPU time is being spent in the high-thread spin-lock tests?

​	For high # of threads, most of the CPU time is spent on waiting for the spin-lock to open.

Where do you believe most of the CPU time is being spent in the high-thread mutex tests?

​	For high # of threads, most of the CPU time is spent on context switches because the mutex lock is not available.  The other threads are put to sleep to wait until lock is open.



QUESTION 2.3.2 - Execution Profiling:

Where (what lines of code) are consuming most of the CPU time when the spin-lock version of the list exerciser is run with a large number of threads?

​            **while** (__sync_lock_test_and_set(&s_lock[index], 1));

Why does this operation become so expensive with large numbers of threads?

​	Because only 1 thread can hold the spin-lock at a time, this operation becomes expensive and CPU time increases. As a higher # of threads have to wait on 1 thread to finish, a large percentage of CPU time is wasted by the other threads spinning.



QUESTION 2.3.3 - Mutex Wait Time:

Why does the average lock-wait time rise so dramatically with the number of contending threads?

​	The average lock-wait time increases dramatically with increased # of threads because the number of competitions between threads for a single lock drastically increases with higher thread #s. The more threads there are, the more threads will be waiting for each lock so the average wait time increases.

Why does the completion time per operation rise (less dramatically) with the number of contending threads?

​	Completion time per operation rises less dramatically with increased # of threads because blocked threads don't add to this time. While there is an inevitable increase in times with any increase in thread #, the completion time only rises slightly--the time spent completing other tasks isn't affected by this thread # increase.



How is it possible for the wait time per operation to go up faster (or higher) than the completion time per operation?

​	Because wait time is proportional to the # of threads, the higher the number of threads, the more threads will be contending for a particular resource at a given moment (increasing wait time). But, the wait time for locks increases a *lot* more because it aggregates from *each* threads blockage, duplicating what would have been a single wait time cost.



QUESTION 2.3.4 - Performance of Partitioned Lists 

Explain the change in performance of the synchronized methods as a function of the number of lists.

​	As the # of lists increases, the performance of synchronized methods increases as well. There is a smaller chance of thread collision if the same # of threads is working on a larger # of lists. The chances of contention are 100% when there is only one list and multiple threads.

Should the throughput continue increasing as the number of lists is further increased? If not, explain why not.

​	The throughput should continue increasing as # of lists increases, but only until each list element has its own sublist. At this point, adding any more lists would be negligible to the total cost of overhead and wouldn't affect contention nor throughput significantly.

It seems reasonable to suggest the throughput of an N-way partitioned list should be equivalent to the throughput of a single list with fewer (1/N)
threads. Does this appear to be true in the above curves? If not, explain why not.

​	This is not true for all cases: while it seems true in the above curves, it isn't a completely invalid approximation.The N-way partitioned list will still suffer from the contentions and context switches associated with having an increased number of threads, while the single list with fewer threads will not. 