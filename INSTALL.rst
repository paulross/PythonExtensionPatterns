.. moduleauthor:: Paul Ross <apaulross@gmail.com>
.. sectionauthor:: Paul Ross <apaulross@gmail.com>

.. highlight:: python
    :linenothreshold: 20

.. toctree::
    :maxdepth: 3

.. index::
    single: Installation

Installation
========================

This is primarily a documentation project hosted on
`Read the Docs <http://pythonextensionpatterns.readthedocs.org/en/latest/index.html>`_.
However all the example code is buildable and testable so if you want to examine that then here is how to get the
project.

From PyPi
------------------------

.. code-block:: console

    pip install cPyExtPatt

From Source
------------------------

Choose a directory of your choice, in this case: ``~/dev/tmp``.

.. code-block:: console

    mkdir -p ~/dev/tmp
    cd ~/dev/tmp
    git clone https://github.com/paulross/PythonExtensionPatterns.git
    cd PythonExtensionPatterns

Virtual Environment
---------------------

Create a Python environment in the directory of your choice, in this case:
``~/dev/tmp/PythonExtensionPatterns/venv_3.11`` and activate it:

.. code-block:: console

    python3.11 -m venv venv_3.11
    source venv_3.11/bin/activate


Install the Dependencies
---------------------------------

.. code-block:: console

    pip install -r requirements.txt

.. index::
    single: Installation; Testing

Running the Tests
-----------------------

You now should be able to run the following commands successfully in
``~/dev/tmp/PythonExtensionPatterns`` with your environment activated:

.. code-block:: console

    pytest tests/

.. index::
    single: Installation; Documentation

Building the Documentation
----------------------------------

This will build the html and PDF documentation (requires a latex installation):

.. code-block:: console

    cd doc/sphinx
    make html latexpdf


