ansitee
=======

Duplicate standard input, with the option to strip ANSI control sequences

Motivation
----------

I want to run a command from Vim, with colour-highlighted output, whilst
simultaneously capturing the output for editing or cross-referencing later.

Synopsis
--------

     ansitee [-ais] [file ...]

Description
-----------

The ansitee utility copies standard input to standard output, making a copy in
zero or more files.
The output is unbuffered.

Files can optionally be written stripped of any ANSI control sequences passed
into standard input.

The following options are available:

* `-a` – Append the output to the files rather than overwriting them.
* `-i` – Ignore the SIGINT signal.
* `-s` – Strip ANSI control sequences when writing to files.
  They are still written to standard output.

The following operands are available:

* `file` – A pathname of an output file.

The ansitee utility takes the default action for all signals, except in the
event of the -i option.

Exit status
-----------

The ansitee utility exits 0 on success, and >0 if an error occurs.
