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
 * Test matrix exceptions.
 *
 * Authors:
 * Eric Marchand
 * Fabien Spindler
 *
 *****************************************************************************/

/*!
  \example testMatrixException.cpp
  \brief Tests matrix exception
*/

#include <visp3/core/vpMath.h>
#include <visp3/core/vpMatrix.h>
#include <visp3/core/vpMatrixException.h>
#include <visp3/core/vpDebug.h>

#include <stdlib.h>
#include <stdio.h>


int main()
{
  try {
    vpMatrix M ;
    vpMatrix M1(2,3) ;
    vpMatrix M2(3,3) ;
    vpMatrix M3(2,2) ;

    vpTRACE("test matrix size in multiply") ;

    try
    {
      M = M1*M3 ;
    }
    catch (vpMatrixException me)
    {
      std::cout << me << std::endl ;
    }


    vpTRACE("test matrix size in addition") ;

    try
    {
      M = M1+M3 ;
    }
    catch (vpMatrixException me)
    {
      std::cout << me << std::endl ;
    }
  }
  catch(vpException e) {
    std::cout << "Catch an exception: " << e << std::endl;
    return 1;
  }
}
