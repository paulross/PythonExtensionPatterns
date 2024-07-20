.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_generators:

***************************
Iterators and Generators
***************************

===========================
Iterators
===========================

How do we write an iterator in C?

The concept is actually fairly straight forward:

- You have some object that contains some data.
- You have some iterator that traverses the data.

That iterator:

- Has a strong reference to the originating object and its data.
  This strong reference keeps the originating object alive as long as the iterator is alive.
- It has a notion of *state*, in other words 'where I was before and where I go next'.

The strong reference to the underlying data structure keeps it alive but what happens if the underlying structure
is altered *during* iteration?
Here is an example:

.. code-block:: python

    lst = list(range(8))
    for i, value in enumerate(lst):
        print(f'i={i} value={value}')
        del lst[i]

Running this results in the irregular sequence:

.. code-block:: bash

    i=0 value=0
    i=1 value=2
    i=2 value=4
    i=3 value=6



===========================
Generators
===========================

Iterators are a very powerful requirement for `Generators <https://docs.python.org/3/glossary.html#term-generator>`_,
the secret weapon in Pythons toolbox.
If you don't believe me then ask David Beazley who has done some very fine and informative presentations on
`Generators <https://www.dabeaz.com/generators/>`_

