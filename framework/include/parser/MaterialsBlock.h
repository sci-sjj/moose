/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIALSBLOCK_H
#define MATERIALSBLOCK_H

#include "ParserBlock.h"

class MaterialsBlock;

template<>
InputParameters validParams<MaterialsBlock>();

class MaterialsBlock: public ParserBlock
{
public:
  MaterialsBlock(const std::string & name, InputParameters params);

  virtual void execute();
};

  

#endif //MATERIALSBLOCK_H
