.. highlight:: python
    :linenothreshold: 10

.. toctree::
    :maxdepth: 3

.. _chapter_generators:

***************************
Generators
***************************

Python `Generators <https://docs.python.org/3/glossary.html#term-generator>`_ are a secret weapon in Pythons toolbox.
If you don't believe me then ask David Beazley who has done some very fine and informative presentations on
`Generators <https://www.dabeaz.com/generators/>`_

So how do we write a Generator in C?

The concept is actually fairly straight forward:

- You have some object that contains some data.
- You have some iterator that traverses the data.

That iterator:

- Has a strong reference to the originating object and its data.
  This strong reference keeps the originating object alive as long as the iterator is alive.
- Has a notion of *state*, in other words 'where I was before and where I go next'.

