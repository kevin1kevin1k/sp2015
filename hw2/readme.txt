b03902086 李鈺昇


1. Execution

To compile all the *.c source codes, just type "make".
If you want to remove all the execution files, type "make rm".

2. Description

I choose to write the three programs in a bottom-up way. That is, starting with player, then host, and finally bidding_system. It's because player don't have to deal with any score or rank, and it only has to communicate with host.
Player is not too hard and didn't take me much time. However, I had much trouble while writing host, mostly because of the FIFO and some I/O problems. And when it comes to bidding_system, with the experience of select() in hw1, I also finished it without much pain.
The algorithm I used in bonus part is as follows:
If I have higher money than all the other players now, then I output the lowest amount of money which is still higher than others'. Otherwise, I always output 999.

3. Self-examination

The seven criteria are all finished. 1~3 are described above in the bidding_system part, 4 in the host part, and 5 in the player part.
After 1~5 are all finished, 6 is easily checked. And then 7 is trivial.
