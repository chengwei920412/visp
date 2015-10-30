/****************************************************************************
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2015 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * See http://visp.inria.fr for more information.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at visp@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Description:
 * Compute the visibility of 3D polygons already transformed in the camera frame
 *
 * Authors:
 * Aurelien Yol
 *
 *****************************************************************************/

#ifndef vpMbScanLine_HH
#define vpMbScanLine_HH

#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <limits>   // numeric_limits

#include <visp3/core/vpColVector.h>
#include <visp3/core/vpCameraParameters.h>
#include <visp3/core/vpPoint.h>
#include <visp3/core/vpImagePoint.h>
#include <visp3/core/vpImage.h>
#include <visp3/core/vpImageConvert.h>

#include <visp3/core/vpDisplayX.h>

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// #define DEBUG_DISP // Uncomment to get visibility debug display

/*!
  \class vpMbScanLine

  \ingroup group_mbt_faces

 */
class vpMbScanLine
{
public:
  //! Structure to define if a scanline intersection is a starting or an ending point of a polygon, or just a single line intersection.
  typedef enum
  {
    START = 1,
    END = 0,
    POINT = 2
  } vpMbScanLineType ;

  //! Structure to define a scanline edge (basically a pair of (X,Y,Z) vectors).
  typedef std::pair<vpColVector, vpColVector> vpMbScanLineEdge;

  //! Structure to define a scanline intersection.
  struct vpMbScanLineSegment
  {
    vpMbScanLineType type;
    vpMbScanLineEdge edge;
    double p; // This value can be either x or y-coordinate value depending if the structure is used in X or Y-axis scanlines computation.
    double P1, P2; // Same comment as previous value.
    double Z1, Z2;
    int ID;
    bool b_sample_Y;
  };

  //! vpMbScanLineEdge Comparator.
  struct vpMbScanLineEdgeComparator
  {
    inline bool operator()(const vpMbScanLineEdge &l0, const vpMbScanLineEdge &l1) const
    {
      for(unsigned int i = 0 ; i < 3 ; ++i)
        if (l0.first[i] < l1.first[i])
          return true;
        else if(l0.first[i] > l1.first[i])
          return false;
      for(unsigned int i = 0 ; i < 3 ; ++i)
        if (l0.second[i] < l1.second[i])
          return true;
        else if(l0.second[i] > l1.second[i])
          return false;
      return false;
    }
  };

  //! vpMbScanLineSegment Comparators.
  struct vpMbScanLineSegmentComparator
  {
    inline bool operator()(const vpMbScanLineSegment &a, const vpMbScanLineSegment &b) const
    {
      //return a.p == b.p ? a.type < b.type : a.p < b.p;
      return (std::fabs(a.p - b.p) <= std::numeric_limits<double>::epsilon()) ? a.type < b.type : a.p < b.p;
    }

    inline bool operator()(const std::pair<double,vpMbScanLineSegment> &a, const std::pair<double, vpMbScanLineSegment> &b) const
    {
      return a.first < b.first;
    }
  };

private:
  unsigned int            w, h;
  vpCameraParameters      K;
  unsigned int            maskBorder;
  vpImage<unsigned char>  mask;
  vpImage<int>            primitive_ids;
  std::map<vpMbScanLineEdge, std::set<int>, vpMbScanLineEdgeComparator> visibility_samples;
  double                  depthTreshold;

public:
#if defined(VISP_HAVE_X11) && defined(DEBUG_DISP)
  vpDisplayX dispMaskDebug;
  vpDisplayX dispLineDebug;
  vpImage<unsigned char>  linedebugImg;
#endif

  vpMbScanLine();

  void drawScene(const std::vector<std::vector<std::pair<vpPoint, unsigned int> > * > &polygons,
                 std::vector<int> listPolyIndices,
                 const vpCameraParameters &K, unsigned int w, unsigned int h);

  /*!
    If there is one polygon behind another,
    this threshold defines the minimum distance between both polygons to still consider the one behind as visible.

    \return Current Threshold.
  */
  double                        getDepthTreshold() { return depthTreshold; }
  unsigned int                  getMaskBorder() { return maskBorder; }
  const vpImage<unsigned char>& getMask() const  { return mask; }
  const vpImage<int>&           getPrimitiveIDs() const  { return primitive_ids; }

  void                          queryLineVisibility(vpPoint a,
                                                    vpPoint b,
                                                    std::vector<std::pair<vpPoint, vpPoint> > &lines, const bool &displayResults = false);

  /*!
    If there is one polygon behind another,
    this threshold defines the minimum distance between both polygons to still consider the one behind as visible.

    \param treshold : New Threshold.
  */
  void                          setDepthTreshold(const double &treshold) { depthTreshold = treshold; }
  void                          setMaskBorder(const unsigned int &mb){ maskBorder = mb; }


private:
  void createScanLinesFromLocals(std::vector<std::vector<vpMbScanLineSegment> > &scanlines,
                                 std::vector<std::vector<vpMbScanLineSegment> > &localScanlines,
                                 const unsigned int &size);

  void drawLineY(const vpColVector &a,
                 const vpColVector &b,
                 const vpMbScanLineEdge &line_ID,
                 const int ID,
                 std::vector<std::vector<vpMbScanLineSegment> > &scanlines);

  void drawLineX(const vpColVector &a,
                 const vpColVector &b,
                 const vpMbScanLineEdge &line_ID,
                 const int ID,
                 std::vector<std::vector<vpMbScanLineSegment> > &scanlines);

  void drawPolygonY(const std::vector<std::pair<vpPoint, unsigned int> > &polygon,
                    const int ID,
                    std::vector<std::vector<vpMbScanLineSegment> > &scanlines);

  void drawPolygonX(const std::vector<std::pair<vpPoint, unsigned int> > &polygon,
                    const int ID,
                    std::vector<std::vector<vpMbScanLineSegment> > &scanlines);

  // Static functions
  static vpMbScanLineEdge makeMbScanLineEdge(const vpPoint &a, const vpPoint &b);
  static void             createVectorFromPoint(const vpPoint &p, vpColVector &v, const vpCameraParameters &K);
  static double           getAlpha(double x, double X0, double Z0, double X1, double Z1);
  static double           mix(double a, double b, double alpha);
  static vpPoint          mix(const vpPoint &a, const vpPoint &b, double alpha);
  static double           norm(const vpPoint &a, const vpPoint &b);
};

#endif // doxygen should skip this

#endif
