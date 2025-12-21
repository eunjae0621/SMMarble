//
//  smm_object.h
//  SMMarble object
//
//  Created by Juyeop Kim on 2023/11/05.
//

#ifndef smm_object_h
#define smm_object_h

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/
#define SMMNODE_TYPE_LECTURE			0
#define SMMNODE_TYPE_RESTAURANT			1
#define SMMNODE_TYPE_LABORATORY			2
#define SMMNODE_TYPE_HOME				3
#define SMMNODE_TYPE_GOTOLAB			4
#define SMMNODE_TYPE_FOODCHANCE			5
#define SMMNODE_TYPE_FESTIVAL			6

#define SMMNODE_OBJTYPE_BOARD	0
#define SMMNODE_OBJTYPE_GRADE	1
#define SMMNODE_OBJTYPE_FOOD	2
#define SMMNODE_OBJTYPE_FEST	3

/* grade :
    Ap,
    A0,
    Am,
    Bp,
    B0,
    Bm,
    Cp,
    C0,
    Cm
*/
#define SMMNODE_MAX_GRADE       13



typedef enum {
	GRADE_Ap = 0,
	GRADE_A0,		
	GRADE_Am,		
	GRADE_Bp,		
	GRADE_B0,		
	GRADE_Bm,		
	GRADE_Cp,		
	GRADE_C0,		
	GRADE_Cm,		
	GRADE_Dp,		
	GRADE_D0,		
	GRADE_Dm,		
	GRADE_F
} smmGrade_e;

//object generation
void* smmObj_genObject(char* name, int objType, int type, int credit, int energy, int grade);
char* smmObj_getObjectName(void *ptr);
int smmObj_getObjectType(void *ptr);
int smmObj_getObjectEnergy(void *ptr);
int smmObj_getObjectCredit(void *ptr);
int smmObj_getObjectGrade(void *ptr);
char* smmObj_getTypeName(int node_type);
//member retrieving


//element to string
char* smmObj_getGradeName(int grade);

#endif/* smm_object_h */
