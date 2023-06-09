#ifdef CH_LANG_CC
/*
 *      _______              __
 *     / ___/ /  ___  __ _  / /  ___
 *    / /__/ _ \/ _ \/  V \/ _ \/ _ \
 *    \___/_//_/\___/_/_/_/_.__/\___/
 *    Please refer to Copyright.txt, in Chombo's root directory.
 */
#endif

#include "DisjointBoxLayout.H"
#include "LevelData.H"
#include "BaseFab.H"
#include "REAL.H"
#include "DataIterator.H"
#include "Tuple.H"
#include "InterpF_F.H"
#include "AverageF_F.H"
#include "DebugOut.H"
#include "FineInterp.H"
#include "NamespaceHeader.H"

/// static variable initialization
int FineInterp::s_default_boundary_limit_type = PCInterp;

FineInterp::FineInterp()
  :
  is_defined(false)
{
}

FineInterp::~FineInterp()
{
}

FineInterp::FineInterp(const DisjointBoxLayout& a_fine_domain,
                       const int&  a_numcomps,
                       const int& a_ref_ratio,
                       const Box& a_fine_problem_domain)
  :
  is_defined(false)
{
  ProblemDomain fineProbDomain(a_fine_problem_domain);
  define(a_fine_domain, a_numcomps, a_ref_ratio, fineProbDomain);
}

FineInterp::FineInterp(const DisjointBoxLayout& a_fine_domain,
                       const int&  a_numcomps,
                       const int& a_ref_ratio,
                       const ProblemDomain& a_fine_problem_domain)
  :
  is_defined(false)
{
  define(a_fine_domain, a_numcomps, a_ref_ratio, a_fine_problem_domain);
}

void
FineInterp::define(const DisjointBoxLayout& a_fine_domain,
                   const int& a_numcomps,
                   const int& a_ref_ratio,
                   const Box& a_fine_problem_domain)
{
  ProblemDomain fineProbDomain(a_fine_problem_domain);
  define(a_fine_domain, a_numcomps, a_ref_ratio, fineProbDomain);
}

void
FineInterp::define(const DisjointBoxLayout& a_fine_domain,
                   const int& a_numcomps,
                   const int& a_ref_ratio,
                   const ProblemDomain& a_fine_problem_domain)
{
  CH_TIME("FineInterp::define");
  
  // set boundary limit type to default value
  m_boundary_limit_type = s_default_boundary_limit_type;

  // check for consistency
  CH_assert (a_fine_domain.checkPeriodic(a_fine_problem_domain));
  m_ref_ratio = a_ref_ratio;
  m_coarse_problem_domain = coarsen(a_fine_problem_domain, m_ref_ratio);
  //
  // create the work array
  DisjointBoxLayout coarsened_fine_domain;
  coarsen ( coarsened_fine_domain,
            a_fine_domain,
            m_ref_ratio );
  m_coarsened_fine_data.define ( coarsened_fine_domain,
                                 a_numcomps,
                                 IntVect::Unit );
  is_defined = true;
}

bool
FineInterp::isDefined() const
{
  return ( is_defined );
}

// interpolate from coarse level to fine level
void
FineInterp::interpToFine(LevelData<FArrayBox>& a_fine_data,
                         const LevelData<FArrayBox>& a_coarse_data,
                         bool a_averageFromDest)
{
  CH_TIME("FineInterp::interpToFine");
  CH_assert(is_defined);

#ifndef NDEBUG
  // debugging check
  {
    DataIterator crseDit = m_coarsened_fine_data.dataIterator();
    for (crseDit.reset(); crseDit.ok(); ++crseDit)
      {
        m_coarsened_fine_data[crseDit()].setVal(1.0e9);
      }
  }
#endif

  if (a_averageFromDest)
    {
      // average down fine data -- this is a local operation
      DataIterator dit = a_fine_data.dataIterator();
      for (dit.begin(); dit.ok(); ++dit)
        {
          FArrayBox& fineFab = a_fine_data[dit];
          FArrayBox& crseFab = m_coarsened_fine_data[dit];
          const Box& crseBox = m_coarsened_fine_data.getBoxes()[dit];

          Box refbox(IntVect::Zero,
                     (m_ref_ratio-1)*IntVect::Unit);
          FORT_AVERAGE(CHF_FRA(crseFab),
                       CHF_CONST_FRA(fineFab),
                       CHF_BOX(crseBox),
                       CHF_CONST_INT(m_ref_ratio),
                       CHF_BOX(refbox));
        }
    }

  // this should handle all the periodic BCs as well,
  // by filling in the ghost cells in an appropriate way
  a_coarse_data.copyTo(a_coarse_data.interval(),
                       m_coarsened_fine_data,
                       m_coarsened_fine_data.interval() );

  const BoxLayout fine_domain = a_fine_data.boxLayout();
  DataIterator dit = fine_domain.dataIterator();
  m_coarsened_fine_data.exchange();
  for (dit.begin(); dit.ok(); ++dit)
    {
      const BaseFab<Real>& coarsened_fine = m_coarsened_fine_data[dit()];
      const Box& coarsened_fine_box = m_coarsened_fine_data.getBoxes()[dit()];
      BaseFab<Real>& fine = a_fine_data[dit()];
      // interpGridData interpolates from an entire coarse grid onto an
      // entire fine grid.
      interpGridData(fine,
                     coarsened_fine,
                     coarsened_fine_box,
                     m_ref_ratio);
    }
}
void
FineInterp::pwcinterpToFine(LevelData<FArrayBox>& a_fine_data,
                            const LevelData<FArrayBox>& a_coarse_data,
                            bool a_averageFromDest)
{
  CH_TIME("FineInterp::pwcinterpToFine");
  CH_assert(is_defined);

  if (a_averageFromDest)
    {
      // average down fine data -- this is a local operation
      DataIterator dit = a_fine_data.dataIterator();
      for (dit.begin(); dit.ok(); ++dit)
        {
          FArrayBox& fineFab = a_fine_data[dit];
          FArrayBox& crseFab = m_coarsened_fine_data[dit];
          const Box& crseBox = m_coarsened_fine_data.getBoxes()[dit];

          Box refbox(IntVect::Zero,
                     (m_ref_ratio-1)*IntVect::Unit);
          FORT_AVERAGE(CHF_FRA(crseFab),
                       CHF_CONST_FRA(fineFab),
                       CHF_BOX(crseBox),
                       CHF_CONST_INT(m_ref_ratio),
                       CHF_BOX(refbox));
        }
    }


  // this should handle all the periodic BCs as well,
  // by filling in the ghost cells in an appropriate way
  a_coarse_data.copyTo(a_coarse_data.interval(),
                       m_coarsened_fine_data,
                       m_coarsened_fine_data.interval() );

  const BoxLayout fine_domain = a_fine_data.boxLayout();
  DataIterator dit = fine_domain.dataIterator();

  for (dit.begin(); dit.ok(); ++dit)
    {
      const BaseFab<Real>& coarsened_fine = m_coarsened_fine_data[dit()];
      const Box& coarsened_fine_box = m_coarsened_fine_data.getBoxes()[dit()];
      BaseFab<Real>& fine = a_fine_data[dit()];
      // interpGridData interpolates from an entire coarse grid onto an
      // entire fine grid.
      pwcinterpGridData(fine,
                        coarsened_fine,
                        coarsened_fine_box,
                        m_ref_ratio);
    }
}

void
FineInterp::pwcinterpGridData(BaseFab<Real>& a_fine,
                           const BaseFab<Real>& a_coarse,
                           const Box& a_coarsened_fine_box,
                           int a_ref_ratio) const
{
  CH_TIME("FineInterp::pwcinterpGridData");
  // fill fine data with piecewise constant coarse data
  const Box& b = a_coarsened_fine_box;
  Box refbox(IntVect::Zero,
             (a_ref_ratio-1)*IntVect::Unit);

  FORT_INTERPCONSTANT ( CHF_FRA(a_fine),
                        CHF_CONST_FRA(a_coarse),
                        CHF_BOX(b),
                        CHF_CONST_INT(a_ref_ratio),
                        CHF_BOX(refbox)
                        );
}
// interpolate from fine grid to coarse grid.  prerequisite:
// coarsened.box contains coarsen(fine.box).
//
// uses piecewise bilinear interpolation with multidimensional-limited
// slopes.  see design document for details.
void
FineInterp::interpGridData(BaseFab<Real>& a_fine,
                           const BaseFab<Real>& a_coarse,
                           const Box& a_coarsened_fine_box,
                           int a_ref_ratio)
  const
{
  CH_TIME("FineInterp::interpGridData");

  // This function assumes 3D or less
  CH_assert(SpaceDim <= 3);

  // fill fine data with piecewise constant coarse data
  const Box& b = a_coarsened_fine_box;
  const int num_comp = a_fine.nComp ();
  Box refbox(IntVect::Zero,
             (a_ref_ratio-1)*IntVect::Unit);

  FORT_INTERPCONSTANT ( CHF_FRA(a_fine),
                        CHF_CONST_FRA(a_coarse),
                        CHF_BOX(b),
                        CHF_CONST_INT(a_ref_ratio),
                        CHF_BOX(refbox)
                        );
  //  Tuple<BaseFab<Real>, SpaceDim> slopes;
  //  for (int dir = 0; dir < SpaceDim; ++dir)
  // hardwired to 3 due to lack of variable number of arguments in chfpp
  FArrayBox slopes[3] {{b,num_comp}, {b,num_comp}, {b,num_comp}};
  for (int dir = 0; dir < 3; ++dir)
    {
      FArrayBox& dir_slope = slopes[dir];
      // initialize to zero for PC-interp case
      dir_slope.setVal(0.0);
    }
  for (int dir = 0; dir < SpaceDim; ++dir)
    {
      FArrayBox& dir_slope = slopes[dir];

      const Box bcenter = grow(m_coarse_problem_domain,-BASISV(dir)) & b;
      if (!bcenter.isEmpty())
        {
          FORT_INTERPCENTRALSLOPE ( CHF_FRA ( dir_slope ),
                                    CHF_CONST_FRA ( a_coarse ),
                                    CHF_BOX ( bcenter ),
                                    CHF_CONST_INT ( dir )
                                    );
        }
      const Box blo = b & adjCellLo(grow(m_coarse_problem_domain,-BASISV(dir)),dir);
      if (!blo.isEmpty())
        {
          FORT_INTERPHISIDESLOPE ( CHF_FRA ( dir_slope ),
                                   CHF_CONST_FRA ( a_coarse ),
                                   CHF_BOX ( blo ),
                                   CHF_CONST_INT ( dir )

                                   );
        }
      const Box bhi = b & adjCellHi(grow(m_coarse_problem_domain,-BASISV(dir)),dir);
      if (!bhi.isEmpty())
        {
          FORT_INTERPLOSIDESLOPE ( CHF_FRA ( dir_slope ),
                                   CHF_CONST_FRA ( a_coarse ),
                                   CHF_BOX ( bhi ),
                                   CHF_CONST_INT ( dir )
                                   );
        }
     // pout() << "before limit slopes for dir " << dir << " and box " << dir_slope.box() << endl;
     // dumpFAB(&dir_slope);
    }

  // to do limits, we need to have a box which includes
  // the neighbors of a given point (to check for the
  // local maximum...
  Box neighborBox(-1*IntVect::Unit,
                  IntVect::Unit);

  // GHM 7/12/01
  // interplimit iterates over box b_mod (was b), but cells within
  // 1 of the physical boundary never enter result (and this
  // wasted calculation may call upon uninitialized memory).
  // DFM 10/8/01
  // note that this turns off slope limiting for cells adjacent to the
  // boundary -- may want to revisit this in the future
  // DFM (9/23/14) -- finally revisiting this; only compute modified box if 
  // slope limiting is turned off or if PC interpolation. 
  // (otherwise, do limiiting adjacent to domain boundaries
  // if we're only limiting tangential to the boundary,  )
  Box b_mod(b);
  bool doLimiting = ((m_boundary_limit_type == limitSlopes) || (m_boundary_limit_type == limitTangentialOnly));
  if (!doLimiting)
    {
      b_mod.grow(1);
      b_mod = m_coarse_problem_domain & b_mod;
      b_mod.grow(-1);
    }

  // if limitTangentialOnly, set normal slopes in cells
  // adjacent to boundary to zero
  if (m_boundary_limit_type == limitTangentialOnly)
    {
      Box testBox(b_mod);
      testBox.grow(1);
      // strip off ghost cells that hang outside the domain
      testBox = m_coarse_problem_domain & testBox;
      for (int dir=0; dir<SpaceDim; dir++)
        {
          // do lo side, then hi
          // first test to see if we're adjacent to a domain boundary
          // grab cells adjacent to b
          Box loBndryBox = adjCellLo(b, dir, 1);
          // only time loBndryBox doesn't intersect testBox is if we're
          // at a non-periodic domain boundary
          if (!loBndryBox.intersects(testBox))
            {
              // move box to interior cells
              loBndryBox.shift(dir, 1);
              // set normal slopes to zero
              slopes[dir].setVal(0, loBndryBox, 0);
            } // end if this box abuts a low-side non-periodic domain boundary


          Box hiBndryBox = adjCellHi(b, dir, 1);
          // only time hiBndryBox doesn't intersect testBox is if we're
          // at a non-periodic domain boundary
          if (!hiBndryBox.intersects(testBox))
            {
              // move box to interior cells
              hiBndryBox.shift(dir, -1);
              // set normal slopes to zero
              slopes[dir].setVal(0, hiBndryBox, 0);
            } // end if this box abuts a hi-side non-periodic domain boundary
        } // end loop over directions      
    }
  
  // create a box grown big enough to remove periodic BCs from domain
  Box domBox = grow(b, 2);
  domBox = m_coarse_problem_domain & domBox;

  FORT_INTERPLIMIT ( CHF_FRA ( slopes[0] ),
                     CHF_FRA ( slopes[1] ),
                     CHF_FRA ( slopes[2] ),
                     CHF_CONST_FRA ( a_coarse ),
                     CHF_BOX ( b_mod ),
                     CHF_BOX ( neighborBox ),
                     CHF_BOX (domBox)
                     );

  //for(int idir = 0; idir < SpaceDim; idir++)
  //  {
  //    FArrayBox& dir_slope = slopes[idir];
  //    pout() << "afterlimit slopes for dir " << idir << " and box " << dir_slope.box() << endl;
  //    dumpFAB(&dir_slope);
  //  }
  for (int dir = 0; dir < SpaceDim; ++dir)
    {
      BaseFab<Real>& dir_slope = slopes[dir];

      Box linearInterpBox = b;
      if (m_boundary_limit_type == PCInterp)
        {
          linearInterpBox = b_mod;
        }
      FORT_INTERPLINEAR ( CHF_FRA ( a_fine ),
                          CHF_CONST_FRA ( dir_slope ),
                          CHF_BOX ( linearInterpBox ),
                          CHF_CONST_INT ( dir ),
                          CHF_CONST_INT ( a_ref_ratio ),
                          CHF_BOX ( refbox )
                          );
    }
}
#include "NamespaceFooter.H"
