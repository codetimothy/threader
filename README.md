# Threader
> Note:This file can be enabled on <span color="lightblue">Windows</span>, Linux🐧 distributions, Mac OS X🍎 and other operating systems.
---   
This is a multi-threaded extension library, excellent architecture, elegant code, compact package, greatly compatibility become its characteristics!
---
- vantage
- The package is extremely small, single file, and greatly compatible.
- Hundreds of times faster than other multithreaded libraries (using fast sort and "sort" functions).
- Easy to operate, easy to use, doesn't take up memory or anything.
- Pure C++ build.
- drawbacks
- Asynchronous is not supported.
- It can nest functions but functions can't nest it. !important
---
code example
---

```c++

#include <iostream>  
int main() {  
    SimpleThreadPool pool(4);  
    for (int i = 0; i < 10; ++i) {  
        pool.addTask([i]() {  
        std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << std::endl;  
        });  
    }  
    pool.wait();  
    return 0;  
}  
```
