#ifndef __RWFILE__
#define __RWFILE__

#include "include.h"
#include "point.h"

typedef struct _PointFile TPointFile;
typedef const TPointFile CPointFile;

/**
 * \struct _PointFile
 * \brief List of all the points of the problem
 *
 * Use to store all the data of the problem
 * - points is a list of all the points
 * - kdTree is a tree to find nearest points easily
 * - nbpoints is the nlength of points
 * - solution is the list of all the points where we put a sensor
 * - cover is the number of point covered by at list one sensor. The objective
 * 	 is to cover all the points
 * - communication_radius the communication radius
 * - capture_radius the capture radius
 */
struct _PointFile {
  TPoint**       points;
  struct kdtree* kdTree;
  int            nbpoints;
  AVLTree*       solution;
  int			 cover;
  double 		 communication_radius;
  double	     capture_radius;
};

#define pf_name(i) pf->points[i]->name
#define pf_x(i) pf->points[i]->x
#define pf_y(i) pf->points[i]->y
#define pf_cover() pf->cover
#define pf_communication_radius() pf->communication_radius
#define pf_capture_radius() pf->capture_radius

TPointFile* read_point_file(char* filename, double communication_radius, double capture_radius);

TPointFile* create_point_file(int size, double communication_radius, double capture_radius);

#endif
