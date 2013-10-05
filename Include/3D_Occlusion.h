/*
 * 3D_Occlusion.h
 *
 *  Created on: Oct 1, 2013
 *      Author: lshields
 */

#ifndef _3D_OCCLUSION_H_
#define _3D_OCCLUSION_H_

typedef GLdouble T_occludeValue;

void OcclusionInit(void);
void OcclusionAdd(T_occludeValue aLeft, T_occludeValue aRight);
int OcclusionIsVisible(T_occludeValue aLeft, T_occludeValue aRight);


#endif /* _3D_OCCLUSION_H_ */
