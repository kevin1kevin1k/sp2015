1. Resources

I refered to PJ's slides, and also googled some tips of non-blocking IO.

In these days I also discussed with my classmates about 
some common problems and the solutions.


2. Some thoughts

First, select is a bit tricky to use (after many trials and errors), 
especially when I have to deal with connections from new clients 
as well as new inputs from already-connected clients.

On the other hand, it's difficult to design the control flow, 
since read_server and write_server require different actions.
Thus, sadly I wrote some "goto"s, which is not a good practice.

Finally, the lock part is more interesting for me and also
easier than the select part.


3. Appreciation

Thanks for preparing so much for us, and thanks for reading. :)
