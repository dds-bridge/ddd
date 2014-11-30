
Double Dummy Driver (DDD)
=========================

Copyright PM Cronje  June 2006

pmcronje@yahoo.com

This software is released under the GPL,
see the files COPYING and LICENSE.

Description
-----------

DDD is a driver for Bo Haglund's Double Dummy Solver (DDS),
released separately under the GPL,
at http://web.telia.com/~u88910365/.

This program is a command line interface for testing and
using the DDS.

It has four useful stand-alone options:
1. Option -playdd, to play any deal double dummy.
2. Option -giblib. For any deal, to get maximum trick values
   for all possible contracts and leaders
   (these are the 20 trick values as for giblib).
3. Option -tricks. For specified deal, to get maximum trick values
   for all possible contracts and leaders
   (these are the 20 trick values as for giblib).
4. Option -gen. This is to generate deals,
   without or with up to 20 possible trick values.

Input 'GIB' files
-----------------

Please read the documentation provided at the top of file giblib.h.
Two sample files are provided --- test.gib and deals.gib

You can get the huge giblib file from the gibware site,
Note that it must be the text version, not the binary version.

Compile and Link
----------------

This program has been tested to run on Linux with the GNU g++ compiler,
as well as in the MingW environment on Windows XP.

To compile and link, see instructions near the top of ddd.cpp,
for Linux/Windows respectively.

Running the program
-------------------

Type ./ddd on Linux or ddd.exe on Windows,
or whatever you named the executable.
The following help is displayed:

  DDD usage:
        ddd_executable file [opts]
  where:
                  file : path for 'giblib' input file
  optional arguments [opts] are one or more of:
                    -v : verbose where applicable
             -target=d : (default -1), see dll reference
                -sol=d : solution 1/2/3 (default 3), see dll reference
               -mode=d : 0/1 (default 1), see dll reference
             -trumps=t : s/h/d/c/n, this overrides the file (default=n)
             -leader=l : w/n/e/s, this overrides the file (default=w)
                         but used only when no cards have been played
               -deal=d : 1/2/... deal number in giblib file
                         only one of -deal or -name should be specified
             -name=str : deal with 'name=str' in giblib file
                         only one of -deal or -name should be specified
               -playdd : play deal choosing between DDS alternatives
              -timeall : time all deals in file for sol=1/2/3, print stats
            -timeg=xcn : x - hex digit, total tricks by n-s
                         c - contract s/h/d/c/n
                         n - number of deals
                         time the first n deals in the giblib file,
                         having total tricks x at contract c,
                         for target=-1 sol=1 mode=1
                         and for the specified/default leader,
                         each deal is validated
   -giblib=d1-d2[-all] : validate all deals from d1 to d2 in giblib file
                         for target=-1 sol=1 mode=1
                         1. if '-all' is given, this is done for all of
                            the 20 trick values even if some of them are '-'
                         2. if '-all' is not given, this is done only for
                            those trick values which are not '-'
               -tricks : like -giblib, but for single deal specified
                         by -name=str -deal=d or option
   generate deals:
                -gen=n : (required) n=number of deals to generate
                         output is written to a file (see below)
            -genseed=s : (default 0) seed for random generator
           -gencards=c : (default=52) number of cards generated per deal,
                         must be multiple of 4
          -gentricks=t : 0,1,...,20 (default 1), number of trick values
                         to set randomly
   generate output is written to a file:
      gen-'genseed'-'ndeal'-'gencards'-'gentricks'.txt
  
Note that all options must start with a '-',
only the the file name starts without one.
Make sure that you start all options with '-',
else you will get a weird error message that
the file cannot be found.

================================================================================
 single calulation
================================================================================

The defaults are target=-1, sol=3, mode=1 trumps=n leader=w:

Do the deal with option name=gin in the file test.gib
  ./ddd test.gib -name=gin
Do the 6-th deal in the file test.gib
  ./ddd test.gib -sol=1 -deal=6 -trumps=s -leader=e

================================================================================
 -playdd: play a deal double dummy
================================================================================

You can play any deal double dummy, selecting between alternatives
suggested by DDS

  ./ddd deals.gib -leader=w -playdd

At the prompt, type 'h' for help, to see the options available.
They are 'q' for quit, 'h' for help, 'u' for unplay card,
and 'sc' (suit-card) to play (e.g. 'd8', 'ht', 's3', ...).

A good test is to start as follows:
  ./ddd deals.gib -leader=w -playdd -name=cpt-47
and then play the following 15 cards:
 w:  c8 c9 ct ck
 s:  h7 h2 hk hj
 n:  d3 d8 dk d5
 s:  h9 h3 ha
the best play for east must then be to discard da. This is the position
for the deal with option name=cpt-47-3t+3c in the same file
(after 3 tricks and 3 cards).

Also try option 'u' to unplay (take back) one or more cards.

================================================================================
 -timeall: benchmark
================================================================================

Do all deals in file test.gib for sol=1/2/3,
time it, collect and print the stats
  ./ddd test.gib -timeall

Note: compare the output with that at the bottom of file ddd.cpp

================================================================================
 -timeg: validate giblib deals for given contract and leader
================================================================================

Notes:
  1. the giblib file must be a giblib text file
     (see the file giblib.h for a description).
  2. The following are used: target=-1 sol=1 mode=1

Do the first ten 3NT (9 tricks) deals in file giblib (this must a giblib file),
for -sol=1:
  ./ddd ../giblib -v -sol=1 -timeg=9n10
compare the output for this at the bottom of file ddd.cpp.

Do the first fifteen 4H (10 tricks = hex a) deals in file giblib with leader
north (the default is -sol=3):
  ./ddd ../giblib -timeg=ah15 -leader=n -v

Note: with many deals specified you may want to omit the option -v (verbose)
which displays progress for each deal only, and not a list for all deals.

================================================================================
 -giblib: validate giblib deals
================================================================================

From version 1.04 there are now two modes
  -giblib=deal1-deal2-all
  -giblib=deal1-deal2

If '-all' is given, this is done for all of
   the 20 tricks even if some of them are '-'
If '-all' is not given, this is done only for
   those tricks which are not '-'

Notes:
  1. See the file giblib.h for a description of the input file.
  2. The following are used: target=-1 sol=1 mode=1
  3. The deal can be any number of tricks (1 to 13),
     it need not be the full number of tricks as for giblib.
     Warning: The number of cards dealt must be a multiple of 4,
              if it is not, the deal is skipped

Normal gib files:
1. When giblib file is in strict giblib format (see the top of file giblib.h)
   --- 67 character deal followed by colon and 20 hex trick values.
   In this case the actual scores are compared with the trick values and
   any differences will be flagged and printed as errors.
   The correct scores are printed at the end of each deal.
2. This option is for testing the DDS solver and for checking a giblib file.

giblib files without trick values:
1. When giblib file contains the deal only, without trick values
   --- 67 character deal only, without colon and 20 hex trick values.
   In this case the correct scores are printed at the end of each deal.
2. This option is used to determine the possible outcomes of a deal
   for all contracts and leaders.

Example, do deals 18 and 19 in the file specified
  ./ddd ../giblib -giblib=18-19 -sol=1

================================================================================
 -tricks: find all 20 trick values for specified files
================================================================================

As for the -giblib option, but only for the deal specified
by either the -name=str or -deal=d options.

./ddd test.gib -tricks -name=gib10
./ddd test.gib -tricks -deal=10

================================================================================
 -gen: generate deals without or with random trick values
================================================================================

       -gen=n : required, this is the main option specifying deal generation,
                with n being the number of deals,
                output is written to a file (see below)
   -genseed=s : (default 0) seed for random generator,
                any unsigned number, this is a unique signature,
                it enables exact repetition of sequences of random numbers
  -gencards=c : (default=52) number of cards generated per deal,
                must be multiple of 4
 -gentricks=t : 0,1,...,20 (default 1), number of trick values
                to set randomly, note that if this is 20 then all
                the trick values are set

Repeating any generation with the same values for
seed/ndeal/cards/tricks is guaranteed to reproduce the same file.

The deal generation output is written to a file:
      gen-'genseed'-'ndeal'-'gencards'-'gentricks'.txt
example
      gen-1234-1000-40-5.txt
is the file for seed=1234, with 1000 deals, 40 cards per deal
and 5 randomly selected trick values

Examples:

Generate 10 deals with no trick values set (seed=0, cards=40)
  ./ddd -gen=10 -gencards=40 -gentricks=0
this goes very fast, of course, with no calls to DDS
the output file is: gen-0-10-40-0.txt

Generate 5 deals with 1 trick value set (seed=0, cards=52)
  ./ddd -gen=5
the output file is: gen-0-5-52-1.txt

Validate the previous step
  ./ddd gen-0-5-52-1.txt -giblib=1-5
compare output from this with that of the previous example

Generate 8 deals with 3 trick values set (seed=10, cards=52)
  ./ddd -gen=8 -genseed=10 -gentricks=3
the output file is: gen-10-8-52-3.txt

