.. _apireference:

.. warning::

    Please be advised that the reference documentation discussing ReLe
    internals is currently incomplete. Please refer to the previous sections
    and the ReLe header files for the nitty gritty details.


API Reference
######################

.. highlight:: c++

All functions and classes provided by the C++ Format library reside
in namespace ``ReLe``.


Policy Representations
======================

.. doxygenclass:: ReLe::Policy
   :members:

.. doxygenclass:: ReLe::ParametricPolicy
   :members:

.. doxygenclass:: ReLe::DifferentiablePolicy
   :members:

Normal Policies
-----------------------

.. doxygenclass:: ReLe::GenericMVNPolicy

.. doxygenclass:: ReLe::GenericMVNDiagonalPolicy

.. doxygenclass:: ReLe::GenericMVNStateDependantStddevPolicy