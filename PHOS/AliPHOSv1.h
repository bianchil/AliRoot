#ifndef PHOSv1_H
#define PHOSv1_H
////////////////////////////////////////////////////////
//  Manager and hits classes for set:PHOS version 1   //
////////////////////////////////////////////////////////

// --- galice header files ---
#include "AliPHOS.h"
 
class AliPHOSv1 : public AliPHOS {

 public:
                        AliPHOSv1();
                        AliPHOSv1(const char *name, const char *title);
  virtual              ~AliPHOSv1(){}
  virtual void          CreateGeometry();
  virtual Int_t         IsVersion() const {return 1;}
  virtual void          StepManager();

 ClassDef(AliPHOSv1,1)  //Hits manager for set:PHOS version 1
};
 
#endif

