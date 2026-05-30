# Library functions

#### 1. fflush <stdio.h>
-  a library function that operates within user space.  It forces the C standard library to write all buffered data for a specified output stream to the underlying file or device.

- It writes immediately whatever is in the buffer.

#### 2. getenv
- The getenv() function searches the environment list to find the
       environment variable name, and returns a pointer to the
       corresponding value string.

- The secure_getenv() function is just like getenv() except that it
       returns NULL in cases where "secure execution" is required.