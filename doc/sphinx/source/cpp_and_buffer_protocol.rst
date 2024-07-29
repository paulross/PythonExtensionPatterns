.. toctree::
    :maxdepth: 2

.. _cpp_and_buffer_protocol:


====================================
C++ and the Python Buffer Protocol
====================================

Python's buffer protocol is a general purpose wrapper around data structures that contain
homogeneous types with a regular structure.
Examples are numpy ``ndarrays``, PIL images and Python types such as ``bytes``, ``bytearray`` and ``array.array`` types.

The buffer protocol is described in `PEP 3118 <https://www.python.org/dev/peps/pep-3118/>`_.

The great advantage of this is that it uses a shared memory model so that the data can be passed between Python or C++
without copying.

It is fairly straightforward to create a C++ wrapper class around the buffer protocol.


.. todo::

    Complete the Buffer Protocol chapter with examples from RaPiVot and the C++ wrapper code.

-----------
References:
-----------

* Python documentation on objects that support the
  `Buffer protocol <https://docs.python.org/3/c-api/buffer.html#bufferobjects>`_.
* Python standard library for the `array module <https://docs.python.org/3/library/array.html#module-array>`_.


