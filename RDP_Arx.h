
#pragma once

#include "gepnt2d.h"
#include "gept2dar.h"

// Polyline creation (your own function)
AcDbObjectId Poly(AcGePoint2dArray ptArr,
				  CString          layerName,
				  int              color     = 256,
				  BOOL             isClosed  = FALSE,
				  double           width     = 0.0,
				  CString          lineType  = _T(""));

// Main API
void DouglasPeucker2d(const AcGePoint2dArray& inPts,
					  double                  epsilon,
					  AcGePoint2dArray&       outPts);

// Test command
void Cmd_TestRdpPolyline();
