# Segment based allocator

This is a simply allocator uses virtual memory model based upon segments.

The memory allocated usually could not be resized depending on the user's memory needs. In order to increase an allocation's size, the user had to explicitly allocate a larger buffer, copy data from the initial allocation, free it and then continue to keep track of the newer allocation's address. This often lead to lower performance and higher peak memory utilization for applications.

The segmentt based allocator decouple the idea of an address and memory and allow the application to handle them separately. The APIs allow applications to allocate additional chunks of memory from a virtual address range as they see fit.

The source code is published under BSD 2-clause, the license is available [here][license].


[//]: # (LINKS)
[license]: LICENSE

