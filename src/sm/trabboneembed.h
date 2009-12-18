/* $Header: /home/cvs/bp/oofem/oofemlib/src/structuralmaterial.h,v 1.11.4.1 2004/04/05 15:19:44 bp Exp $ */
/*

                   *****    *****   ******  ******  ***   ***                            
                 **   **  **   **  **      **      ** *** **                             
                **   **  **   **  ****    ****    **  *  **                              
               **   **  **   **  **      **      **     **                               
              **   **  **   **  **      **      **     **                                
              *****    *****   **      ******  **     **         
            
                                                                   
               OOFEM : Object Oriented Finite Element Code                 
                    
                 Copyright (C) 1993 - 2000   Borek Patzak                                       



         Czech Technical University, Faculty of Civil Engineering,
     Department of Structural Mechanics, 166 29 Prague, Czech Republic
                                                                               
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                                                                              
*/

//   ************************************************
//   *** CLASS TRABECULAR BONE EMBEDDING MATERIAL ***
//   ************************************************

#ifndef trabboneembed_h 

#include "structuralmaterial.h"
#include "flotarry.h"
#include "flotmtrx.h"
#include "cltypes.h"
#include "matconst.h"
#include "matstatus.h"
#include "strainvector.h"


#include "linearelasticmaterial.h"
#include "dictionr.h"

#include "structuralms.h"
#include "cltypes.h"

namespace oofem {

/*
 This class implements associated Material Status to TrabBoneEmbed.
 It is atribute of matStatusDictionary at every GaussPoint, for which this material 
 is active.
*/


/////////////////////////////////////////////////////////////////
/////////////TRABECULAR BONE EMBEDDING STATUS////////////////////
/////////////////////////////////////////////////////////////////

class TrabBoneEmbedStatus : public StructuralMaterialStatus
{
  protected:

  /////////////////////////////////////////////////////////////////
  // STATE VARIABLE DECLARATION
  //

  double tempAlpha, alpha;
  double tempDam, dam;
  double tempPSED, psed;
  double tempTSED, tsed;
  FloatMatrix smtrx, matConstD;
  FloatArray densStress, tempPlasDef, plasDef, tempIncPlasDef;

  public: 

  /////////////////////////////////////////////////////////////////
  // CONSTRUCTOR
  //

  TrabBoneEmbedStatus (int n, Domain*d, GaussPoint* g) ;


  /////////////////////////////////////////////////////////////////
  // DESTRUCTOR
  // 

  ~TrabBoneEmbedStatus ();
  

  /////////////////////////////////////////////////////////////////
  // OUTPUT PRINT 
  // Prints the receiver state to stream

  void   printOutputAt (FILE *file, TimeStep* tStep) ;


  /////////////////////////////////////////////////////////////////
  // STATE VARIABLE 
  // declare state variable access and modification methods

  double giveTempTSED();

  void setTempDam(double da){tempDam=da;}
  void setTempTSED(double tse){tempTSED=tse;}
  void setTempAlpha(double al){tempAlpha=al;}
  void setTempPlasDef(FloatArray epsip){tempPlasDef = epsip;}
  void setSmtrx(FloatMatrix smt){smtrx = smt;}


  ///////////////////////////////////////////////////////////////// 
  // DEFINITION
  // 

  const char* giveClassName () const { return "TrabBoneEmbedStatus" ;}
  classType   giveClassID () const { return TrabBoneEmbedStatusClass; }


  /////////////////////////////////////////////////////////////////
  // INITIALISATION OF TEMPORARY VARIABLES
  // Initializes the temporary internal variables, describing the current state according to
  // previously reached equilibrium internal variables.

  virtual void initTempStatus ();


  /////////////////////////////////////////////////////////////////
  // UPDATE VARIABLES 
  // Update equilibrium history variables according to temp-variables.
  // Invoked, after new equilibrium state has been reached.

  virtual void updateYourself(TimeStep*);


  /////////////////////////////////////////////////////////////////
  // SAVE CONTEXT - INTERUPT/RESTART
  // saves current context(state) into stream
  // Only non-temp internal history variables are stored.
  // @param stream stream where to write data
  // @param obj pointer to integration point, which invokes this method
  // @return contextIOResultType.

  contextIOResultType    saveContext (DataStream* stream, ContextMode mode, void *obj = NULL);


  /////////////////////////////////////////////////////////////////
  // RESTORE CONTEXT - INTERUPT/RESTART
  // Restores context of receiver from given stream. 
  // @param stream stream where to read data
  // @param obj pointer to integration point, which invokes this method
  // @return contextIOResultType.

   contextIOResultType    restoreContext(DataStream* stream, ContextMode mode, void *obj = NULL);

}; 


/////////////////////////////////////////////////////////////////
////////////////TRABECULAR BONE EMBEDDING////////////////////////
/////////////////////////////////////////////////////////////////


class TrabBoneEmbed : public StructuralMaterial
{
  protected:

  /////////////////////////////////////////////////////////////////
  // STATE VARIABLE DECLARATION
  // declare material properties here

  double eps0, nu0;

  public:

  /////////////////////////////////////////////////////////////////
  // CONSTRUCTOR
  //

  TrabBoneEmbed (int n, Domain* d);


  /////////////////////////////////////////////////////////////////
  // DESTRUCTOR
  // 

  // NO DESTRUCTOR

  /////////////////////////////////////////////////////////////////
  // INITIALIZATION OF FUNCTION/SUBROUTINE
  // 

  void performPlasticityReturn (GaussPoint* gp, const FloatArray& totalStrain);

  double computeDamageParam (double alpha, GaussPoint* gp);

  double computeDamage (GaussPoint* gp, TimeStep* atTime);

  virtual void computeCumPlastStrain (double& alpha, GaussPoint* gp, TimeStep* atTime);

  // Construct anisotropic compliance tensor.
  void constructIsoComplTensor (FloatMatrix& answer, const  double eps0, const  double nu0);

  //Method for computing 1d  stifness matrix of receiver.
  //Default implementation computes 3d stifness matrix using give3dMaterialStiffnessMatrix and
  //reduces it to 1d stiffness using reduce method described above.
  //Howewer, this reduction is quite time consuming and if it is possible, 
  //it is recomended to overload this method and provide direct method for computing
  //particular stifness matrix.
  //@param answer stifness matrix
  //@param form material response form
  //@param mode material response mode
  //@param gp integration point, which load history is used
  //@param atTime time step (most models are able to respond only when atTime is current time step)
 
  virtual void give3dMaterialStiffnessMatrix (FloatMatrix& answer,
                   MatResponseForm,MatResponseMode,GaussPoint* gp,
                   TimeStep* atTime);
  
  //Computes the real stress vector for given total strain and integration point. 
  //The total strain is defined as strain computed directly from displacement field at given time.
  //The stress independent parts (temperature, eigen strains) are substracted in constitutive
  //driver.
  //The service should use previously reached equilibrium history variables. Also
  //it should update temporary history variables in status according to newly reached state.
  //The temporary history variables are moved into equilibrium ones after global structure
  //equlibrium has been reached by iteration process.
  //@param answer contains result
  //@param form material response form
  //@param gp integration point 
  //@param reducedStrain strain vector in reduced form
  //@param tStep current time step (most models are able to respond only when atTime is current time step)
  
  virtual void giveRealStressVector (FloatArray& answer, MatResponseForm, GaussPoint*, 
                                     const FloatArray& , TimeStep*);
  
  //Requests material mode capability.
  //@param mode material mode requested
  //@return nonzero if available
 
  virtual int hasMaterialModeCapability (MaterialMode ) ;

  // Returns class name of the receiver.

  const char* giveClassName () const { return "TrabBoneEmbed" ;}

  // Returns classType id of receiver.
 
  classType giveClassID () const {return TrabBoneEmbedClass; }

  //Initializes receiver acording to object description stored in input record.
  //The density of material is read into property dictionary (keyword 'd')
  
  IRResultType initializeFrom (InputRecord* ir);

  MaterialStatus* CreateStatus (GaussPoint* gp) const;

 /**
  Returns the integration point corresponding value in Reduced form.
  @param answer contain corresponding ip value, zero sized if not available
  @param aGaussPoint integration point
  @param type determines the type of internal variable
  @param type determines the type of internal variable
  @returns nonzero if ok, zero if var not supported
  */
 virtual int giveIPValue (FloatArray& answer, GaussPoint* aGaussPoint, InternalStateType type, TimeStep* atTime) ;

 /**
  Returns the mask of reduced indexes of Internal Variable component .
  @param answer mask of Full VectorSize, with components beeing the indexes to reduced form vectors.
  @param type determines the internal variable requested (physical meaning)
  @returns nonzero if ok or error is generated for unknown mat mode.
  */
 virtual int giveIntVarCompFullIndx (IntArray& answer, InternalStateType type, MaterialMode mmode);

 /**
  Returns the type of internal variable (scalar, vector, tensor,...).
  @param type determines the type of internal variable
  @returns type of internal variable
  */
 virtual  InternalStateValueType giveIPValueType (InternalStateType type) ;

 /**
  Returns the corresponding integration point  value size in Reduced form.
  @param type determines the type of internal variable
  @returns var size, zero if var not supported
  */
 virtual int giveIPValueSize (InternalStateType type, GaussPoint* aGaussPoint) ;
};

} // end namespace oofem
#define trabboneembed_h
#endif
