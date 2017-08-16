//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

#include "Problem.h"
#include "DiracKernelInfo.h"
#include "GeometricSearchData.h"
#include "MooseTypes.h"

#include "libmesh/coupling_matrix.h"

class MooseMesh;
class SubProblem;
class Factory;
class Assembly;
class MooseVariableFEBase;
class MooseVariableScalar;
template <typename>
class MooseVariableFEImpl;
typedef MooseVariableFEImpl<Real> MooseVariable;
typedef MooseVariableFEImpl<RealVectorValue> VectorMooseVariable;
class RestartableDataValue;

// libMesh forward declarations
namespace libMesh
{
class EquationSystems;
class DofMap;
class CouplingMatrix;
template <typename T>
class SparseMatrix;
template <typename T>
class NumericVector;
class System;
}

template <>
InputParameters validParams<SubProblem>();

/**
 * Generic class for solving transient nonlinear problems
 *
 */
class SubProblem : public Problem
{
public:
  SubProblem(const InputParameters & parameters);
  virtual ~SubProblem();

  virtual EquationSystems & es() = 0;
  virtual MooseMesh & mesh() = 0;

  virtual bool checkNonlocalCouplingRequirement() { return _requires_nonlocal_coupling; }

  virtual void solve() = 0;
  virtual bool converged() = 0;

  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual bool isTransient() const = 0;

  /**
   * Create a Tag.  Tags can be associated with Vectors and Matrices and allow objects
   * (such as Kernels) to arbitrarily contribute values to any set of vectors/matrics
   *
   * Note: If the tag is already present then this will simply return the TagID of that Tag
   *
   * @param tag_name The name of the tag to create, the TagID will get automatically generated
   */
  TagID addTag(TagName tag_name);

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) const = 0;

  /// Returns the variable reference for requested variable which may be in any system
  virtual MooseVariableFEBase & getVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the variable reference for requested MooseVariable which may be in any system
  virtual MooseVariable & getStandardVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the variable reference for requested VectorMooseVariable which may be in any system
  virtual VectorMooseVariable & getVectorVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns a Boolean indicating whether any system contains a variable with the name provided
  virtual bool hasScalarVariable(const std::string & var_name) const = 0;

  /// Returns the scalar variable reference from whichever system contains it
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name) = 0;

  /// Returns the equation system containing the variable provided
  virtual System & getSystem(const std::string & var_name) = 0;

  /**
   * Set the MOOSE variables to be reinited on each element.
   * @param moose_vars A set of variables that need to be reinited each time reinit() is called.
   *
   * @param tid The thread id
   */
  virtual void setActiveElementalMooseVariables(const std::set<MooseVariableFEBase *> & moose_vars,
                                                THREAD_ID tid);

  /**
   * Get the MOOSE variables to be reinited on each element.
   *
   * @param tid The thread id
   */
  virtual const std::set<MooseVariableFEBase *> &
  getActiveElementalMooseVariables(THREAD_ID tid) const;

  /**
   * Whether or not a list of active elemental moose variables has been set.
   *
   * @return True if there has been a list of active elemental moose variables set, False otherwise
   */
  virtual bool hasActiveElementalMooseVariables(THREAD_ID tid) const;

  /**
   * Clear the active elemental MooseVariableFEBase.  If there are no active variables then they
   * will all be reinited. Call this after finishing the computation that was using a restricted set
   * of MooseVariableFEBases
   *
   * @param tid The thread id
   */
  virtual void clearActiveElementalMooseVariables(THREAD_ID tid);

  /**
   * Record and set the material properties required by the current computing thread.
   * @param mat_prop_ids The set of material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual void setActiveMaterialProperties(const std::set<unsigned int> & mat_prop_ids,
                                           THREAD_ID tid);

  /**
   * Get the material properties required by the current computing thread.
   *
   * @param tid The thread id
   */
  virtual const std::set<unsigned int> & getActiveMaterialProperties(THREAD_ID tid) const;

  /**
   * Method to check whether or not a list of active material roperties has been set. This method
   * is called by reinitMaterials to determine whether Material computeProperties methods need to be
   * called. If the return is False, this check prevents unnecessary material property computation
   * @param tid The thread id
   *
   * @return True if there has been a list of active material properties set, False otherwise
   */
  virtual bool hasActiveMaterialProperties(THREAD_ID tid) const;

  /**
   * Clear the active material properties. Should be called at the end of every computing thread
   *
   * @param tid The thread id
   */
  virtual void clearActiveMaterialProperties(THREAD_ID tid);

  virtual Assembly & assembly(THREAD_ID tid) = 0;
  virtual void prepareShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) = 0;

  /**
   * Returns the desired radial direction for RZ coordinate transformation
   * @return The coordinate direction for the radial direction
   */
  unsigned int getAxisymmetricRadialCoord() const;

  virtual DiracKernelInfo & diracKernelInfo();
  virtual Real finalNonlinearResidual() const;
  virtual unsigned int nNonlinearIterations() const;
  virtual unsigned int nLinearIterations() const;

  virtual void addResidual(THREAD_ID tid) = 0;
  virtual void addResidualNeighbor(THREAD_ID tid) = 0;

  virtual void cacheResidual(THREAD_ID tid) = 0;
  virtual void cacheResidualNeighbor(THREAD_ID tid) = 0;
  virtual void addCachedResidual(THREAD_ID tid) = 0;

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid) = 0;
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) = 0;

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian,
                                unsigned int ivar,
                                unsigned int jvar,
                                const DofMap & dof_map,
                                std::vector<dof_id_type> & dof_indices,
                                THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   std::vector<dof_id_type> & dof_indices,
                                   std::vector<dof_id_type> & neighbor_dof_indices,
                                   THREAD_ID tid) = 0;

  virtual void cacheJacobian(THREAD_ID tid) = 0;
  virtual void cacheJacobianNeighbor(THREAD_ID tid) = 0;
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;

  virtual void prepare(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepareFace(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepare(const Elem * elem,
                       unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices,
                       THREAD_ID tid) = 0;
  virtual void setCurrentSubdomainID(const Elem * elem, THREAD_ID tid) = 0;
  virtual void setNeighborSubdomainID(const Elem * elem, unsigned int side, THREAD_ID tid) = 0;
  virtual void prepareAssembly(THREAD_ID tid) = 0;

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemPhys(const Elem * elem,
                              const std::vector<Point> & phys_points_in_elem,
                              THREAD_ID tid) = 0;
  virtual void
  reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNodes(const std::vector<dof_id_type> & nodes, THREAD_ID tid) = 0;
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes, THREAD_ID tid) = 0;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  unsigned int neighbor_side,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor,
                                  const std::vector<Point> & physical_points,
                                  THREAD_ID tid) = 0;
  virtual void reinitScalars(THREAD_ID tid) = 0;
  virtual void reinitOffDiagScalars(THREAD_ID tid) = 0;

  /**
   * Returns true if the Problem has Dirac kernels it needs to compute on elem.
   */
  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid) = 0;
  /**
   * Fills "elems" with the elements that should be looped over for Dirac Kernels
   */
  virtual void getDiracElements(std::set<const Elem *> & elems) = 0;
  /**
   * Gets called before Dirac Kernels are asked to add the points they are supposed to be evaluated
   * in
   */
  virtual void clearDiracInfo() = 0;

  // Geom Search
  virtual void
  updateGeomSearch(GeometricSearchData::GeometricSearchType type = GeometricSearchData::ALL) = 0;

  virtual GeometricSearchData & geomSearchData() = 0;

  virtual void meshChanged();

  /**
   * Adds the given material property to a storage map based on block ids
   *
   * This is method is called from within the Material class when the property
   * is first registered.
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeMatPropName(SubdomainID block_id, const std::string & name);

  /**
   * Adds the given material property to a storage map based on boundary ids
   *
   * This is method is called from within the Material class when the property
   * is first registered.
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeMatPropName(BoundaryID boundary_id, const std::string & name);

  /**
   * Adds to a map based on block ids of material properties for which a zero
   * value can be returned. Thes properties are optional and will not trigger a
   * missing material property error.
   *
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeZeroMatProp(SubdomainID block_id, const MaterialPropertyName & name);

  /**
   * Adds to a map based on boundary ids of material properties for which a zero
   * value can be returned. Thes properties are optional and will not trigger a
   * missing material property error.
   *
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeZeroMatProp(BoundaryID boundary_id, const MaterialPropertyName & name);

  /**
   * Adds to a map based on block ids of material properties to validate
   *
   * @param block_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeDelayedCheckMatProp(const std::string & requestor,
                                        SubdomainID block_id,
                                        const std::string & name);

  /**
   * Adds to a map based on boundary ids of material properties to validate
   *
   * @param requestor The MOOSE object name requesting the material property
   * @param boundary_id The block id for the MaterialProperty
   * @param name The name of the property
   */
  virtual void storeDelayedCheckMatProp(const std::string & requestor,
                                        BoundaryID boundary_id,
                                        const std::string & name);

  /**
   * Checks block material properties integrity
   *
   * \see FEProblemBase::checkProblemIntegrity
   */
  virtual void checkBlockMatProps();

  /**
   * Checks boundary material properties integrity
   *
   * \see FEProblemBase::checkProblemIntegrity
   */
  virtual void checkBoundaryMatProps();

  /**
   * Helper method for adding a material property name to the _material_property_requested set
   */
  virtual void markMatPropRequested(const std::string &);

  /**
   * Find out if a material property has been requested by any object
   */
  virtual bool isMatPropRequested(const std::string & prop_name) const;

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(dof_id_type elem_id) = 0;

  /**
   * Will make sure that all necessary elements from boundary_id are ghosted to this processor
   */
  virtual void addGhostedBoundary(BoundaryID boundary_id) = 0;

  /**
   * Causes the boundaries added using addGhostedBoundary to actually be ghosted.
   */
  virtual void ghostGhostedBoundaries() = 0;

  /**
   * Get a vector containing the block ids the material property is defined on.
   */
  virtual std::set<SubdomainID> getMaterialPropertyBlocks(const std::string & prop_name);

  /**
   * Get a vector of block id equivalences that the material property is defined on.
   */
  virtual std::vector<SubdomainName> getMaterialPropertyBlockNames(const std::string & prop_name);

  /**
   * Check if a material property is defined on a block.
   */
  virtual bool hasBlockMaterialProperty(SubdomainID block_id, const std::string & prop_name);

  /**
   * Get a vector containing the block ids the material property is defined on.
   */
  virtual std::set<BoundaryID> getMaterialPropertyBoundaryIDs(const std::string & prop_name);

  /**
   * Get a vector of block id equivalences that the material property is defined on.
   */
  virtual std::vector<BoundaryName> getMaterialPropertyBoundaryNames(const std::string & prop_name);

  /**
   * Check if a material property is defined on a block.
   */
  virtual bool hasBoundaryMaterialProperty(BoundaryID boundary_id, const std::string & prop_name);

  /**
   * Returns true if the problem is in the process of computing it's initial residual.
   * @return Whether or not the problem is currently computing the initial residual.
   */
  virtual bool computingInitialResidual() const = 0;

  /**
   * Return the list of elements that should have their DoFs ghosted to this processor.
   * @return The list
   */
  virtual std::set<dof_id_type> & ghostedElems() { return _ghosted_elems; }

  std::map<std::string, std::vector<dof_id_type>> _var_dof_map;
  const CouplingMatrix & nonlocalCouplingMatrix() const { return _nonlocal_cm; }

  /**
   * Returns true if the problem is in the process of computing Jacobian
   */
  virtual bool currentlyComputingJacobian() const { return _currently_computing_jacobian; };

  /// Check whether residual being evaulated is non-linear
  bool & computingNonlinearResid() { return _computing_nonlinear_residual; }

protected:
  /// The currently declared tags
  std::map<TagName, TagID> _tag_name_to_tag_id;

  /// The Factory for building objects
  Factory & _factory;

  CouplingMatrix _nonlocal_cm; /// nonlocal coupling matrix;

  /// Type of coordinate system per subdomain
  std::map<SubdomainID, Moose::CoordinateSystemType> _coord_sys;

  DiracKernelInfo _dirac_kernel_info;

  /// Map of material properties (block_id -> list of properties)
  std::map<SubdomainID, std::set<std::string>> _map_block_material_props;

  /// Map for boundary material properties (boundary_id -> list of properties)
  std::map<BoundaryID, std::set<std::string>> _map_boundary_material_props;

  /// Set of properties returned as zero properties
  std::map<SubdomainID, std::set<MaterialPropertyName>> _zero_block_material_props;
  std::map<BoundaryID, std::set<MaterialPropertyName>> _zero_boundary_material_props;

  /// set containing all material property names that have been requested by getMaterialProperty*
  std::set<std::string> _material_property_requested;

  ///@{
  /**
   * Data structures of the requested material properties.  We store them in a map
   * from boudnary/block id to multimap.  Each of the multimaps is a list of
   * requestor object names to material property names.
   */
  std::map<SubdomainID, std::multimap<std::string, std::string>> _map_block_material_props_check;
  std::map<BoundaryID, std::multimap<std::string, std::string>> _map_boundary_material_props_check;
  ///@}

  /// This is the set of MooseVariableFEBases that will actually get reinited by a call to reinit(elem)
  std::vector<std::set<MooseVariableFEBase *>> _active_elemental_moose_variables;

  /// Whether or not there is currently a list of active elemental moose variables
  /* This needs to remain <unsigned int> for threading purposes */
  std::vector<unsigned int> _has_active_elemental_moose_variables;

  /// Set of material property ids that determine whether materials get reinited
  std::vector<std::set<unsigned int>> _active_material_property_ids;

  /// nonlocal coupling requirement flag
  bool _requires_nonlocal_coupling;

  /// Elements that should have Dofs ghosted to the local processor
  std::set<dof_id_type> _ghosted_elems;

  /// Storage for RZ axis selection
  unsigned int _rz_coord_axis;

  /// Flag to determine whether the problem is currently computing Jacobian
  bool _currently_computing_jacobian;

  /// Whether residual being evaulated is non-linear
  bool _computing_nonlinear_residual;

private:
  /**
   * Helper method for performing material property checks
   * @param props Reference to the map of properties known
   * @param check_props Reference to the map of properties to check
   * @param type - string describing the type of the material property being checked
   * \see checkBlockMatProps
   * \see checkBoundaryMatProps
   */
  template <typename T>
  void checkMatProps(std::map<T, std::set<std::string>> & props,
                     std::map<T, std::multimap<std::string, std::string>> & check_props,
                     std::map<T, std::set<MaterialPropertyName>> & zero_props);

  ///@{ Helper functions for checkMatProps
  template <typename T>
  std::string restrictionTypeName();
  std::string restrictionCheckName(SubdomainID check_id);
  std::string restrictionCheckName(BoundaryID check_id);
  ///@}

  friend class Restartable;
};

namespace Moose
{

void initial_condition(EquationSystems & es, const std::string & system_name);

} // namespace Moose

#endif /* SUBPROBLEM_H */
