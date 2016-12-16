# CDMA Chip-sequence decoder

In the example text files are 1023 number entries. This the signal that has been created by multiple layered CDMA 
chip sequences. Those have been created by multiple GPS satellites.

The task is to create a software decoder, that receives the following information from the signal:

* Which GPS satellite is responsible for sending the signal.
* Which bit has been sent from the satellite.
* What is the delta of the information.

### Additional information
A GPS chip-sequence contains 1023 chips. Each satellite sent exactly one bit in the recorded time frame.
The GPS signals reach the receiver in and unordered fashion (asynchronous CDMA). This results in the chipsequences not beginning at the beginning of the time frame, but in the middle. In this task it can be expected, that the satellite sent the same bit before and afterward, so the sequence can be completed by reading from the beginning (appending it to the end)

### Output
The program's output is something like this:

```
Satellite  8 has sent bit 0 (delta = 72)
Satellite  9 has sent bit 1 (delta = 449)
Satellite 18 has sent bit 0 (delta = 345)
Satellite 22 has sent bit 1 (delta = 157)
```