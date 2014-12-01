// =============================================================================
/* rng.cpp RNG - random number generators

            PMC 14-jun-2005
            PMC 10-jul-2005
            PMC 13-Jul-2006 updated, default is now MT

   Copyright 2005-2006 P.M.Cronje

   RNG is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   RNG is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with RNG; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

*/
// =============================================================================

#ifndef RNG_H
#define RNG_H

typedef unsigned long long uint64;

// -----------------------------------------------------------------------------
// Entropy test
// -----------------------------------------------------------------------------

void initEnt(bool binmode);
void addEnt(unsigned char * buf, int buflen);
void endEnt(double * r_ent, double * r_chisq, double * r_mean,
            double * r_montepicalc, double * r_scc);
void prtEnt(double r_chisq, double r_mean,
            double r_montepicalc, double r_scc);

// -----------------------------------------------------------------------------
// Available RNG's
// -----------------------------------------------------------------------------

enum eRNG
{
  eRNG_QD1 = 0, // 'Quick and dirty' LCG
  eRNG_MT = 1, // Mersenne Twister MT19937_02 (default)
  eRNG_MTHR = 2, // 'Mother of all RNG' (Marsaglia)
  eRNG_WELL = 3, // WELL1024a, single seed with run-in

  eRNG_COUNT = 4

}; // enum eRNG
// -----------------------------------------------------------------------------
// Description
// -----------------------------------------------------------------------------
//
// The QD1 is fast, but is only as good as an LCG can be,
// it should only be used where true randomness is not required.
// LCG = linear congruential generator.
//
// The other (mt/mthr/well) have been selected after many tests
// using the TestU01 software of Ecuyer and Simard, and pass most
// of the stringent tests for random number generators, failing only
// occasionally at a significance level of a few parts in 0.001.
// They all have extremely long periods and are recommended for any
// serious simulation work requiring many millions of random numbers.
//
// The default generator is MT19937_02.
//
// The acronym WELL means 'Well Equidistributed Long-period Linear'
// The generator WELL1024a has been modified to:
// - initialize with a single seed
// - generate equidistributed initial values
// - run for 50,000 numbers to escape from the initial setup
//
// The generators mt/mthr are initialized from a single seed using a WELL.
//
// Speed (relative elapsed time for 10,000,000 numbers)
// qd1 1.00
// mt 1.25
// mthr 1.90
// well 1.41
//
// -----------------------------------------------------------------------------

extern const char * pszRNGGen[eRNG_COUNT];
extern const char * pszRNGGenList;
extern const char * pszRNGGenDefault;
enum eRNG const eRNGGenDefault = eRNG_MT;

// -----------------------------------------------------------------------------
// forward declaration of derived classes
// -----------------------------------------------------------------------------

class cRNG_QD1;
class cRNG_WELL;
class cRNG_Mother;
class cRNG_MT19937;

// -----------------------------------------------------------------------------
// cRNG - base class
// -----------------------------------------------------------------------------

class cRNG
{
  public:
    virtual ~cRNG()
    { }

    // set seed
    virtual void set(unsigned int useed) = 0;

    // generator name
    virtual char * getszRandom() = 0;

    // next random number
    virtual unsigned int random() = 0;

    // return random unsigned int, where 0 <= unsigned int < uirange
    virtual unsigned int randomUint(unsigned int urange);

    // creates an instance of cRNG,
    // specifying either eRNG or a string identifier pszrng,
    // if pszgen incorrectly specified creates a cRNG_MT
    //
    // erng pszrng
    // --------- --------
    // eRNG_QD1 "qd1"
    // eRNG_MT "mt"
    // eRNG_MTHR "mthr"
    // eRNG_WELL "well"
    //
    // note: the object must be deleted after use

    static cRNG * createRNG(eRNG erng, unsigned int useed);
    static cRNG * createRNG(char * pszrng, unsigned int useed);

    enum eRNG getRNG()
    {
      return RNG;
    }

    const char * getpszGen()
    {
      return static_cast<const char *>(szGen);
    }

  protected:
    eRNG RNG;
    char szGen[16];
    unsigned int uSeed;

}; // cRNG
// -----------------------------------------------------------------------------

class cRNG_QD1 : public cRNG
{
  public:
    cRNG_QD1(unsigned int useed = 0)
    {
      set(useed);
    }

    // set
    virtual void set(unsigned int useed);

    // generator name
    virtual char * getszRandom()
    {
      return strdup("QD1");
    }

    // next random number
    virtual
    unsigned int random();

    unsigned int getSeed()
    {
      return uSeed;
    }

  protected:

}; // cRNG_QD1
// -----------------------------------------------------------------------------

class cRNG_WELL : public cRNG
{
  public:
    cRNG_WELL(unsigned int useed = 0)
    {
      set(useed);
    }

    // set
    virtual void set(unsigned int useed);

    // generator name
    virtual char * getszRandom()
    {
      return strdup("WELL1024u");
    }

    // next random number
    virtual
    unsigned int random();

  protected:
    unsigned int state_i, STATE[32], z0, z1, z2;

}; // cRNG_WELL
// -----------------------------------------------------------------------------

class cRNG_Mother : public cRNG
{
  public:
    cRNG_Mother(unsigned int useed = 0)
    {
      set(useed);
    }

    // set
    virtual void set(unsigned int useed);

    // generator name
    virtual char * getszRandom()
    {
      return strdup("Mother");
    }

    // next random number
    virtual
    unsigned int random();

  protected:
    unsigned int smthr[4];
    uint64 xm4, xm3, xm2, xm1, mcarry;

}; // cRNG_Mother
// -----------------------------------------------------------------------------

class cRNG_MT19937 : public cRNG
{
  public:
    cRNG_MT19937(unsigned int useed = 0)
    {
      set(useed);
    }

    // set
    virtual void set(unsigned int useed);

    // generator name
    virtual char * getszRandom()
    {
      return strdup("MT19937");
    }

    // next random number
    virtual
    unsigned int random();

  protected:
    unsigned int mti, U[624];
    unsigned long mag01[2];

    void initBySeed(unsigned int useed);
    void initByArray(unsigned int lenkey, unsigned int key[]);


}; // cRNG_MT19937
// -----------------------------------------------------------------------------
#endif
