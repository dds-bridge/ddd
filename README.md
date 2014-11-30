DDD is a driver for Bo Haglund's [Double Dummy Solver (DDS)](https://github.com/dds-bridge/dds/), written by PM Cronje and relased under the GPL.

DDD had not really been touched since 2007, and the last readme notes were dated 2006.  Those notes are available in the file [readme2006.txt](https://github.com/dds-bridge/ddd/blob/v1.1.0/readme2006.txt).

This is a light modification in November 2014 to fit with the current structure of DDS, as of v2.8.  It expects the DDS 2.8 distribution to be in the `dds/` subdirectory.  For convenience it is now on GitHub and includes DDS as a [Git "submodule"](http://git-scm.com/book/en/v2/Git-Tools-Submodules).  It should automatically pull down the proper version of DDS if you clone the repository (using `--recurse`).

Change Summary
==============

* The code has been modified slightly to compile with the same compilers in the same way as the main DDS library.  The set-up with Makefiles in a directory is the same.  The code is now compiled against the DLL and is not linked directly with the DDS code.

* Note that DDD only calls `SolveBoard()` and no other DDS functions.  Especially for calculating the DD table for all 20 combinations of declarer and strain, there are now much better functions available.

* DDD does not make use of the batch processing capability in DDS *(as it probably didn't exist with DDD was written)*.

* DDD enables the user to walk the DD "tree" interactively.  At least when walking "downwards" towards fewer cards, the
`AnalysePlay()` functions can be used.  There are excellent visualization tools available, including [DDS Captain](http://www.bridge-captain.com/downloadDD.html), that do this in a graphical way.

Overall, the code would need a fair amount of work to be a good demonstrator of DDS as of v2.8.  For some example programs that show all the new individual functions in DDS, see [DDS's examples directory](https://github.com/dds-bridge/dds/tree/v2.8.0/examples).

Soren Hein, November 2014
