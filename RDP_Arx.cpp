#include "stdafx.h"
#include "RDP_Arx.h"

#include "aced.h"
#include "dbents.h"
#include "dbsymtb.h"

// Distance from a point to an infinite line AB
static double pointToLineDist2d(const AcGePoint2d& p,
								const AcGePoint2d& a,
								const AcGePoint2d& b)
{
	if (a.isEqualTo(b, AcGeContext::gTol))
		return p.distanceTo(a);

	AcGeLine2d ln(a, b);
	return ln.distanceTo(p);
}

// Recursive part
static void dpRecursive(const AcGePoint2dArray& inPts,
						int                     first,
						int                     last,
						double                  epsilon,
						AcGePoint2dArray&       outPts)
{
	if (last <= first + 1)
		return;

	const AcGePoint2d& a = inPts[first];
	const AcGePoint2d& b = inPts[last];

	double maxDist = 0.0;
	int    idx     = -1;

	for (int i = first + 1; i < last; ++i)
	{
		double d = pointToLineDist2d(inPts[i], a, b);
		if (d > maxDist)
		{
			maxDist = d;
			idx     = i;
		}
	}

	if (idx >= 0 && maxDist > epsilon)
	{
		dpRecursive(inPts, first, idx, epsilon, outPts);
		outPts.append(inPts[idx]);
		dpRecursive(inPts, idx, last,  epsilon, outPts);
	}
}

// Main function
void DouglasPeucker2d(const AcGePoint2dArray& inPts,
					  double                  epsilon,
					  AcGePoint2dArray&       outPts)
{
	outPts.setLogicalLength(0);

	int n = (int)inPts.length();
	if (n <= 2)
	{
		outPts = inPts;
		return;
	}

	outPts.append(inPts[0]);
	dpRecursive(inPts, 0, n - 1, epsilon, outPts);
	outPts.append(inPts[n - 1]);
}


// Simple sort by X, then by Y (ascending)
static void sortPointsByX(AcGePoint2dArray& pts)
{
	int n = (int)pts.length();
	for (int i = 0; i < n - 1; ++i)
	{
		for (int j = i + 1; j < n; ++j)
		{
			const AcGePoint2d& pi = pts.at(i);
			const AcGePoint2d& pj = pts.at(j);

			bool swapNeeded = false;
			if (pj.x < pi.x)
				swapNeeded = true;
			else if (pj.x == pi.x && pj.y < pi.y)
				swapNeeded = true;

			if (swapNeeded)
			{
				AcGePoint2d tmp = pts.at(i);
				pts.setAt(i, pj);
				pts.setAt(j, tmp);
			}
		}
	}
}


// Sort points around centroid by angle (atan2)
static void sortPointsByAngle(AcGePoint2dArray& pts)
{
	int n = (int)pts.length();
	if (n < 2)
		return;

	AcGePoint2d c(0.0, 0.0);
	int i;
	for (i = 0; i < n; ++i)
	{
		const AcGePoint2d& p = pts.at(i);
		c.x += p.x;
		c.y += p.y;
	}
	c.x /= (double)n;
	c.y /= (double)n;

	double* ang = new double[n];
	for (i = 0; i < n; ++i)
	{
		const AcGePoint2d& p = pts.at(i);
		ang[i] = atan2(p.y - c.y, p.x - c.x);
	}

	// simple bubble sort
	for (i = 0; i < n - 1; ++i)
	{
		for (int j = i + 1; j < n; ++j)
		{
			if (ang[j] < ang[i])
			{
				double tmpAng = ang[i];
				ang[i] = ang[j];
				ang[j] = tmpAng;

				AcGePoint2d tmpPt = pts.at(i);
				pts.setAt(i, pts.at(j));
				pts.setAt(j, tmpPt);
			}
		}
	}

	delete[] ang;
}


//------------------------------------------------------------
// Test command: pick a set of POINT entities and create a
// simplified polyline using Douglas-Peucker.
//------------------------------------------------------------
void Cmd_TestRdpPolyline()
{
	ads_name ss;
	if (acedSSGet(NULL, NULL, NULL, NULL, ss) != RTNORM)
	{
		acutPrintf(_T("\nNothing selected."));
		return;
	}

	long count = 0;
	acedSSLength(ss, &count);
	if (count < 2)
	{
		acutPrintf(_T("\nAt least two point entities required."));
		acedSSFree(ss);
		return;
	}

	AcGePoint2dArray inPts;
	CString layerName(_T("0"));

	for (long i = 0; i < count; ++i)
	{
		ads_name en;
		acedSSName(ss, i, en);

		AcDbObjectId id;
		acdbGetObjectId(id, en);

		AcDbEntity* e = NULL;
		if (acdbOpenObject(e, id, AcDb::kForRead) != Acad::eOk)
			continue;

		AcDbPoint* pt = AcDbPoint::cast(e);
		if (pt)
		{
			AcGePoint3d p3 = pt->position();
			inPts.append(AcGePoint2d(p3.x, p3.y));

			if (i == 0)
				layerName = pt->layer();
		}
		e->close();
	}

	acedSSFree(ss);

	if (inPts.length() < 2)
	{
		acutPrintf(_T("\nNot enough valid points."));
		return;
	}
 

	// Sort points  
	//sortPointsByX(inPts);
	sortPointsByAngle(inPts);

	double epsilon = 0.0;
	if (acedGetReal(_T("\nEpsilon: "), &epsilon) != RTNORM || epsilon <= 0.0)
	{
		acutPrintf(_T("\nInvalid epsilon."));
		return;
	}

	AcGePoint2dArray outPts;
	DouglasPeucker2d(inPts, epsilon, outPts);

	acutPrintf(_T("\nInput: %d   Output: %d"),
		inPts.length(), outPts.length());

	if (outPts.length() >= 2)
		Poly(outPts, layerName,1);
}
