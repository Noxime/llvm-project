===========================
lld |release| Release Notes
===========================

.. contents::
    :local:

.. only:: PreRelease

  .. warning::
     These are in-progress notes for the upcoming LLVM |release| release.
     Release notes for previous releases can be found on
     `the Download Page <https://releases.llvm.org/download.html>`_.

Introduction
============

This document contains the release notes for the lld linker, release |release|.
Here we describe the status of lld, including major improvements
from the previous release. All lld releases may be downloaded
from the `LLVM releases web site <https://llvm.org/releases/>`_.

Non-comprehensive list of changes in this release
=================================================

ELF Improvements
----------------

* ``--remap-inputs=`` and ``--remap-inputs-file=`` are added to remap input files.
  (`D148859 <https://reviews.llvm.org/D148859>`_)
* ``PT_RISCV_ATTRIBUTES`` is added to include the SHT_RISCV_ATTRIBUTES section.
  (`D152065 <https://reviews.llvm.org/D152065>`_)
* Link speed improved greatly compared with lld 15.0. Notably input section
  initialization and relocation scanning are now parallel.
  (`D130810 <https://reviews.llvm.org/D130810>`_)
  (`D133003 <https://reviews.llvm.org/D133003>`_)
* ``ELFCOMPRESS_ZSTD`` compressed input sections are now supported.
  (`D129406 <https://reviews.llvm.org/D129406>`_)
* ``--compress-debug-sections=zstd`` is now available to compress debug
  sections with zstd (``ELFCOMPRESS_ZSTD``).
  (`D133548 <https://reviews.llvm.org/D133548>`_)
* ``--no-warnings``/``-w`` is now available to suppress warnings.
  (`D136569 <https://reviews.llvm.org/D136569>`_)
* ``DT_RISCV_VARIANT_CC`` is now produced if at least one ``R_RISCV_JUMP_SLOT``
  relocation references a symbol with the ``STO_RISCV_VARIANT_CC`` bit.
  (`D107951 <https://reviews.llvm.org/D107951>`_)
* ``DT_STATIC_TLS`` is now set for AArch64/PPC32/PPC64 initial-exec TLS models
  when producing a shared object.
* ``--no-undefined-version`` is now the default; symbols named in version
  scripts that have no matching symbol in the output will be reported. Use
  ``--undefined-version`` to revert to the old behavior.
  (`D135402 <https://reviews.llvm.org/D135402>`_)
* ``-V`` is now an alias for ``-v`` to support ``gcc -fuse-ld=lld -v`` on many targets.
* ``-r`` no longer defines ``__global_pointer$`` or ``_TLS_MODULE_BASE_``.
* A corner case of mixed GCC and Clang object files (``STB_WEAK`` and
  ``STB_GNU_UNIQUE`` in different COMDATs) is now supported.
  (`D136381 <https://reviews.llvm.org/D136381>`_)
* The output ``SHT_RISCV_ATTRIBUTES`` section now merges all input components
  instead of picking the first input component.
  (`D138550 <https://reviews.llvm.org/D138550>`_)
* For x86-32, ``-fno-plt`` GD/LD TLS models ``call *___tls_get_addr@GOT(%reg)``
  are now supported. Previous output might have runtime crash.
* Armv4(T) thunks are now supported.
  (`D139888 <https://reviews.llvm.org/D139888>`_)
  (`D141272 <https://reviews.llvm.org/D141272>`_)

Breaking changes
----------------

COFF Improvements
-----------------

MinGW Improvements
------------------

MachO Improvements
------------------

WebAssembly Improvements
------------------------

Fixes
#####

* Arm exception index tables (.ARM.exidx sections) are now output
  correctly when they are at a non zero offset within their output
  section. (`D148033 <https://reviews.llvm.org/D148033>`_)
