//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseVariableFEBase.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseVariableField.h"
#include "MooseVariableData.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_vector.h"

class TimeIntegrator;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;

template <>
InputParameters validParams<MooseVariable>();
template <>
InputParameters validParams<VectorMooseVariable>();
template <>
InputParameters validParams<ArrayMooseVariable>();

/**
 * Class for stuff related to variables
 *
 * Each variable can compute nodal or elemental (at QPs) values.
 *
 * OutputType          OutputShape           OutputData
 * ----------------------------------------------------
 * Real                Real                  Real
 * RealVectorValue     RealVectorValue       Real
 * RealEigenVector      Real                  RealEigenVector
 *
 */
template <typename OutputType>
class MooseVariableFE : public MooseVariableField<OutputType>
{
public:
  using OutputGradient = typename MooseVariableField<OutputType>::OutputGradient;
  using OutputSecond = typename MooseVariableField<OutputType>::OutputSecond;
  using OutputDivergence = typename MooseVariableField<OutputType>::OutputDivergence;

  using FieldVariableValue = typename MooseVariableField<OutputType>::FieldVariableValue;
  using FieldVariableGradient = typename MooseVariableField<OutputType>::FieldVariableGradient;
  using FieldVariableSecond = typename MooseVariableField<OutputType>::FieldVariableSecond;
  using FieldVariableCurl = typename MooseVariableField<OutputType>::FieldVariableCurl;
  using FieldVariableDivergence = typename MooseVariableField<OutputType>::FieldVariableDivergence;

  using OutputShape = typename MooseVariableField<OutputType>::OutputShape;
  using OutputShapeGradient = typename MooseVariableField<OutputType>::OutputShapeGradient;
  using OutputShapeSecond = typename MooseVariableField<OutputType>::OutputShapeSecond;
  using OutputShapeDivergence = typename MooseVariableField<OutputType>::OutputShapeDivergence;

  using FieldVariablePhiValue = typename MooseVariableField<OutputType>::FieldVariablePhiValue;
  using FieldVariablePhiGradient =
      typename MooseVariableField<OutputType>::FieldVariablePhiGradient;
  using FieldVariablePhiSecond = typename MooseVariableField<OutputType>::FieldVariablePhiSecond;
  using FieldVariablePhiCurl = typename MooseVariableField<OutputType>::FieldVariablePhiCurl;
  using FieldVariablePhiDivergence =
      typename MooseVariableField<OutputType>::FieldVariablePhiDivergence;

  using FieldVariableTestValue = typename MooseVariableField<OutputType>::FieldVariableTestValue;
  using FieldVariableTestGradient =
      typename MooseVariableField<OutputType>::FieldVariableTestGradient;
  using FieldVariableTestSecond = typename MooseVariableField<OutputType>::FieldVariableTestSecond;
  using FieldVariableTestCurl = typename MooseVariableField<OutputType>::FieldVariableTestCurl;
  using FieldVariableTestDivergence =
      typename MooseVariableField<OutputType>::FieldVariableTestDivergence;

  using OutputData = typename MooseVariableField<OutputType>::OutputData;
  using DoFValue = typename MooseVariableField<OutputType>::DoFValue;

  MooseVariableFE(const InputParameters & parameters);

  void clearDofIndices() override;

  void prepare() override;
  void prepareNeighbor() override;
  void prepareLowerD() override;
  virtual void prepareIC() override;

  void prepareAux() override;

  void reinitNode() override;
  void reinitAux() override;
  void reinitAuxNeighbor() override;

  void reinitNodes(const std::vector<dof_id_type> & nodes) override;
  void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes) override;

  /**
   * Whether or not this variable is actually using the shape function value.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesPhi() const { return true; }
  /**
   * Whether or not this variable is actually using the shape function gradient.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesGradPhi() const { return true; }
  /**
   * Whether or not this variable is actually using the shape function value.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesPhiNeighbor() const { return true; }
  /**
   * Whether or not this variable is actually using the shape function gradient.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesGradPhiNeighbor() const { return true; }
  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool usesSecondPhi() const;

  /**
   * Whether or not this variable is actually using the shape function second derivative on a
   * neighbor.
   */
  bool usesSecondPhiNeighbor() const;

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool computingSecond() const { return usesSecondPhi(); }

  /**
   * Whether or not this variable is computing the curl
   */
  bool computingCurl() const;

  const std::set<SubdomainID> & activeSubdomains() const override;
  bool activeOnSubdomain(SubdomainID subdomain) const override;

  bool isNodal() const override { return _element_data->isNodal(); }
  Moose::VarFieldType fieldType() const override;
  bool isVector() const override;
  const Node * const & node() const { return _element_data->node(); }
  const dof_id_type & nodalDofIndex() const override { return _element_data->nodalDofIndex(); }
  virtual bool isNodalDefined() const override;

  const Node * const & nodeNeighbor() const { return _neighbor_data->node(); }
  const dof_id_type & nodalDofIndexNeighbor() const override
  {
    return _neighbor_data->nodalDofIndex();
  }
  bool isNodalNeighborDefined() const;

  const Elem * const & currentElem() const override { return _element_data->currentElem(); }

  /**
   * Current side this variable is being evaluated on
   */
  const unsigned int & currentSide() const { return _element_data->currentSide(); }

  /**
   * Current neighboring element
   */
  const Elem * const & neighbor() const { return _neighbor_data->currentElem(); }

  virtual void getDofIndices(const Elem * elem,
                             std::vector<dof_id_type> & dof_indices) const override;
  const std::vector<dof_id_type> & dofIndices() const final { return _element_data->dofIndices(); }
  unsigned int numberOfDofs() const final { return _element_data->numberOfDofs(); }
  const std::vector<dof_id_type> & dofIndicesNeighbor() const final
  {
    return _neighbor_data->dofIndices();
  }
  const std::vector<dof_id_type> & dofIndicesLower() const final
  {
    return _lower_data->dofIndices();
  }

  unsigned int numberOfDofsNeighbor() override { return _neighbor_data->dofIndices().size(); }

  const FieldVariablePhiValue & phi() const { return _element_data->phi(); }
  const FieldVariablePhiGradient & gradPhi() const { return _element_data->gradPhi(); }
  const MappedArrayVariablePhiGradient & arrayGradPhi() const
  {
    return _element_data->arrayGradPhi();
  }
  const FieldVariablePhiSecond & secondPhi() const;
  const FieldVariablePhiCurl & curlPhi() const;

  const FieldVariablePhiValue & phiFace() const { return _element_data->phiFace(); }
  const FieldVariablePhiGradient & gradPhiFace() const { return _element_data->gradPhiFace(); }
  const MappedArrayVariablePhiGradient & arrayGradPhiFace() const
  {
    return _element_data->arrayGradPhiFace();
  }
  const FieldVariablePhiSecond & secondPhiFace() const;
  const FieldVariablePhiCurl & curlPhiFace() const;

  const FieldVariablePhiValue & phiNeighbor() const { return _neighbor_data->phi(); }
  const FieldVariablePhiGradient & gradPhiNeighbor() const { return _neighbor_data->gradPhi(); }
  const MappedArrayVariablePhiGradient & arrayGradPhiNeighbor() const
  {
    return _neighbor_data->arrayGradPhi();
  }
  const FieldVariablePhiSecond & secondPhiNeighbor() const;
  const FieldVariablePhiCurl & curlPhiNeighbor() const;

  const FieldVariablePhiValue & phiFaceNeighbor() const { return _neighbor_data->phiFace(); }
  const FieldVariablePhiGradient & gradPhiFaceNeighbor() const
  {
    return _neighbor_data->gradPhiFace();
  }
  const MappedArrayVariablePhiGradient & arrayGradPhiFaceNeighbor() const
  {
    return _neighbor_data->arrayGradPhiFace();
  }
  const FieldVariablePhiSecond & secondPhiFaceNeighbor() const;
  const FieldVariablePhiCurl & curlPhiFaceNeighbor() const;

  const FieldVariablePhiValue & phiLower() const { return _lower_data->phi(); }
  const FieldVariablePhiGradient & gradPhiLower() const { return _lower_data->gradPhi(); }

  const ADTemplateVariableTestGradient<OutputShape> & adGradPhi()
  {
    return _element_data->adGradPhi();
  }

  const ADTemplateVariableTestGradient<OutputShape> & adGradPhiFace()
  {
    return _element_data->adGradPhiFace();
  }

  // damping
  const FieldVariableValue & increment() const { return _element_data->increment(); }

  const FieldVariableValue & vectorTagValue(TagID tag)
  {
    return _element_data->vectorTagValue(tag);
  }
  const FieldVariableValue & matrixTagValue(TagID tag)
  {
    return _element_data->matrixTagValue(tag);
  }

  /// element solutions
  const FieldVariableValue & sln() const { return _element_data->sln(Moose::Current); }
  const FieldVariableValue & slnOld() const { return _element_data->sln(Moose::Old); }
  const FieldVariableValue & slnOlder() const { return _element_data->sln(Moose::Older); }
  const FieldVariableValue & slnPreviousNL() const { return _element_data->sln(Moose::PreviousNL); }

  /// element gradients
  const FieldVariableGradient & gradSln() const { return _element_data->gradSln(Moose::Current); }
  const FieldVariableGradient & gradSlnOld() const { return _element_data->gradSln(Moose::Old); }
  const FieldVariableGradient & gradSlnOlder() const
  {
    return _element_data->gradSln(Moose::Older);
  }
  const FieldVariableGradient & gradSlnPreviousNL() const
  {
    return _element_data->gradSln(Moose::PreviousNL);
  }

  /// element gradient dots
  const FieldVariableGradient & gradSlnDot() const { return _element_data->gradSlnDot(); }
  const FieldVariableGradient & gradSlnDotDot() const { return _element_data->gradSlnDotDot(); }

  /// element seconds
  const FieldVariableSecond & secondSln() const { return _element_data->secondSln(Moose::Current); }
  const FieldVariableSecond & secondSlnOld() const { return _element_data->secondSln(Moose::Old); }
  const FieldVariableSecond & secondSlnOlder() const
  {
    return _element_data->secondSln(Moose::Older);
  }
  const FieldVariableSecond & secondSlnPreviousNL() const
  {
    return _element_data->secondSln(Moose::PreviousNL);
  }

  /// element curls
  const FieldVariableCurl & curlSln() const { return _element_data->curlSln(Moose::Current); }
  const FieldVariableCurl & curlSlnOld() const { return _element_data->curlSln(Moose::Old); }
  const FieldVariableCurl & curlSlnOlder() const { return _element_data->curlSln(Moose::Older); }

  /// AD
  const ADTemplateVariableValue<OutputType> & adSln() const override
  {
    return _element_data->adSln();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSln() const override
  {
    return _element_data->adGradSln();
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSln() const override
  {
    return _element_data->adSecondSln();
  }
  const ADTemplateVariableValue<OutputType> & adUDot() const override
  {
    return _element_data->adUDot();
  }

  /// neighbor AD
  const ADTemplateVariableValue<OutputType> & adSlnNeighbor() const override
  {
    return _neighbor_data->adSln();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnNeighbor() const override
  {
    return _neighbor_data->adGradSln();
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSlnNeighbor() const override
  {
    return _neighbor_data->adSecondSln();
  }
  const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const override
  {
    return _neighbor_data->adUDot();
  }

  /// element dots
  const FieldVariableValue & uDot() const { return _element_data->uDot(); }
  const FieldVariableValue & uDotDot() const { return _element_data->uDotDot(); }
  const FieldVariableValue & uDotOld() const { return _element_data->uDotOld(); }
  const FieldVariableValue & uDotDotOld() const { return _element_data->uDotDotOld(); }
  const VariableValue & duDotDu() const { return _element_data->duDotDu(); }
  const VariableValue & duDotDotDu() const { return _element_data->duDotDotDu(); }

  /// neighbor solutions
  const FieldVariableValue & slnNeighbor() const { return _neighbor_data->sln(Moose::Current); }
  const FieldVariableValue & slnOldNeighbor() const { return _neighbor_data->sln(Moose::Old); }
  const FieldVariableValue & slnOlderNeighbor() const { return _neighbor_data->sln(Moose::Older); }
  const FieldVariableValue & slnPreviousNLNeighbor() const
  {
    return _neighbor_data->sln(Moose::PreviousNL);
  }

  /// neighbor solution gradients
  const FieldVariableGradient & gradSlnNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Current);
  }
  const FieldVariableGradient & gradSlnOldNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Old);
  }
  const FieldVariableGradient & gradSlnOlderNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Older);
  }
  const FieldVariableGradient & gradSlnPreviousNLNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::PreviousNL);
  }

  /// neighbor grad dots
  const FieldVariableGradient & gradSlnNeighborDot() const { return _neighbor_data->gradSlnDot(); }
  const FieldVariableGradient & gradSlnNeighborDotDot() const
  {
    return _neighbor_data->gradSlnDotDot();
  }

  /// neighbor solution seconds
  const FieldVariableSecond & secondSlnNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::Current);
  }
  const FieldVariableSecond & secondSlnOldNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::Old);
  }
  const FieldVariableSecond & secondSlnOlderNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::Older);
  }
  const FieldVariableSecond & secondSlnPreviousNLNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::PreviousNL);
  }

  /// neighbor solution curls
  const FieldVariableCurl & curlSlnNeighbor() const
  {
    return _neighbor_data->curlSln(Moose::Current);
  }
  const FieldVariableCurl & curlSlnOldNeighbor() const
  {
    return _neighbor_data->curlSln(Moose::Old);
  }
  const FieldVariableCurl & curlSlnOlderNeighbor() const
  {
    return _neighbor_data->curlSln(Moose::Older);
  }

  /// neighbor dots
  const FieldVariableValue & uDotNeighbor() const { return _neighbor_data->uDot(); }
  const FieldVariableValue & uDotDotNeighbor() const { return _neighbor_data->uDotDot(); }
  const FieldVariableValue & uDotOldNeighbor() const { return _neighbor_data->uDotOld(); }
  const FieldVariableValue & uDotDotOldNeighbor() const { return _neighbor_data->uDotDotOld(); }
  const VariableValue & duDotDuNeighbor() const { return _neighbor_data->duDotDu(); }
  const VariableValue & duDotDotDuNeighbor() const { return _neighbor_data->duDotDotDu(); }

  /// lower-d element solution
  const ADTemplateVariableValue<OutputType> & adSlnLower() const { return _lower_data->adSln(); }
  const FieldVariableValue & slnLower() const { return _lower_data->sln(Moose::Current); }

  /// Actually compute variable values from the solution vectors
  virtual void computeElemValues() override;
  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;
  virtual void computeLowerDValues() override;

  virtual void setNodalValue(const OutputType & value, unsigned int idx = 0) override;

  virtual void setDofValue(const OutputData & value, unsigned int index) override;

  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  void setDofValues(const DenseVector<OutputData> & values);

  /**
   * Write a nodal value to the passed-in solution vector
   */
  void insertNodalValue(NumericVector<Number> & residual, const OutputData & v);

  /**
   * Get the value of this variable at given node
   */
  OutputData getNodalValue(const Node & node);
  /**
   * Get the old value of this variable at given node
   */
  OutputData getNodalValueOld(const Node & node);
  /**
   * Get the t-2 value of this variable at given node
   */
  OutputData getNodalValueOlder(const Node & node);
  /**
   * Get the current value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  OutputData getElementalValue(const Elem * elem, unsigned int idx = 0) const;
  /**
   * Get the old value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  OutputData getElementalValueOld(const Elem * elem, unsigned int idx = 0) const;
  /**
   * Get the older value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  OutputData getElementalValueOlder(const Elem * elem, unsigned int idx = 0) const;
  /**
   * Set the current local DOF values to the input vector
   */
  void insert(NumericVector<Number> & residual) override;
  /**
   * Add the current local DOF values to the input vector
   */
  void add(NumericVector<Number> & residual) override;
  /**
   * Add passed in local DOF values onto the current solution
   */
  void addSolution(const DenseVector<Number> & v);
  /**
   * Add passed in local neighbor DOF values onto the current solution
   */
  void addSolutionNeighbor(const DenseVector<Number> & v);

  const DoFValue & dofValue();
  const DoFValue & dofValues();
  const DoFValue & dofValuesOld();
  const DoFValue & dofValuesOlder();
  const DoFValue & dofValuesPreviousNL();
  const DoFValue & dofValuesNeighbor();
  const DoFValue & dofValuesOldNeighbor();
  const DoFValue & dofValuesOlderNeighbor();
  const DoFValue & dofValuesPreviousNLNeighbor();
  const DoFValue & dofValuesDot();
  const DoFValue & dofValuesDotNeighbor();
  const DoFValue & dofValuesDotOld();
  const DoFValue & dofValuesDotOldNeighbor();
  const DoFValue & dofValuesDotDot();
  const DoFValue & dofValuesDotDotNeighbor();
  const DoFValue & dofValuesDotDotOld();
  const DoFValue & dofValuesDotDotOldNeighbor();
  const MooseArray<Number> & dofValuesDuDotDu();
  const MooseArray<Number> & dofValuesDuDotDuNeighbor();
  const MooseArray<Number> & dofValuesDuDotDotDu();
  const MooseArray<Number> & dofValuesDuDotDotDuNeighbor();

  /**
   * Return the AD dof values
   */
  const MooseArray<ADReal> & adDofValues();

  /**
   * Compute and store incremental change in solution at QPs based on increment_vec
   */
  void computeIncrementAtQps(const NumericVector<Number> & increment_vec);

  /**
   * Compute and store incremental change at the current node based on increment_vec
   */
  void computeIncrementAtNode(const NumericVector<Number> & increment_vec);

  /**
   * Compute the variable value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable value
   */
  OutputType getValue(const Elem * elem, const std::vector<std::vector<OutputShape>> & phi) const;

  /**
   * Compute the variable gradient value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable gradient value
   */
  typename OutputTools<OutputType>::OutputGradient getGradient(
      const Elem * elem,
      const std::vector<std::vector<typename OutputTools<OutputType>::OutputShapeGradient>> &
          grad_phi) const;

  /**
   * Return phi size
   */
  virtual size_t phiSize() const final { return _element_data->phiSize(); }
  /**
   * Return phiFace size
   */
  virtual size_t phiFaceSize() const final { return _element_data->phiFaceSize(); }
  /**
   * Return phiNeighbor size
   */
  virtual size_t phiNeighborSize() const final { return _neighbor_data->phiSize(); }
  /**
   * Return phiFaceNeighbor size
   */
  virtual size_t phiFaceNeighborSize() const final { return _neighbor_data->phiFaceSize(); }

  size_t phiLowerSize() const final { return _lower_data->phiSize(); }

  /**
   * Methods for retrieving values of variables at the nodes
   */
  const OutputType & nodalValue();
  const OutputType & nodalValueOld();
  const OutputType & nodalValueOlder();
  const OutputType & nodalValuePreviousNL();
  const OutputType & nodalValueDot();
  const OutputType & nodalValueDotDot();
  const OutputType & nodalValueDotOld();
  const OutputType & nodalValueDotDotOld();
  const OutputType & nodalValueDuDotDu();
  const OutputType & nodalValueDuDotDotDu();
  const OutputType & nodalValueNeighbor();
  const OutputType & nodalValueOldNeighbor();
  const OutputType & nodalValueOlderNeighbor();
  const OutputType & nodalValuePreviousNLNeighbor();
  const OutputType & nodalValueDotNeighbor();
  const OutputType & nodalValueDotDotNeighbor();
  const OutputType & nodalValueDotOldNeighbor();
  const OutputType & nodalValueDotDotOldNeighbor();
  const OutputType & nodalValueDuDotDuNeighbor();
  const OutputType & nodalValueDuDotDotDuNeighbor();

  /**
   * Methods for retrieving values of variables at the nodes in a MooseArray for AuxKernelBase
   */
  const MooseArray<OutputType> & nodalValueArray()
  {
    return _element_data->nodalValueArray(Moose::Current);
  }
  const MooseArray<OutputType> & nodalValueOldArray()
  {
    return _element_data->nodalValueArray(Moose::Old);
  }
  const MooseArray<OutputType> & nodalValueOlderArray()
  {
    return _element_data->nodalValueArray(Moose::Older);
  }

  const DoFValue & nodalVectorTagValue(TagID tag);
  const DoFValue & nodalMatrixTagValue(TagID tag);

  const typename Moose::ADType<OutputType>::type & adNodalValue();

  virtual void computeNodalValues() override;
  virtual void computeNodalNeighborValues() override;

protected:
  usingMooseVariableBaseMembers;

  /// Holder for all the data associated with the "main" element
  std::unique_ptr<MooseVariableData<OutputType>> _element_data;

  /// Holder for all the data associated with the neighbor element
  std::unique_ptr<MooseVariableData<OutputType>> _neighbor_data;

  /// Holder for all the data associated with the lower dimeensional element
  std::unique_ptr<MooseVariableData<OutputType>> _lower_data;
};

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFE<OutputType>::adDofValues()
{
  return _element_data->adDofValues();
}

template <typename OutputType>
inline const typename Moose::ADType<OutputType>::type &
MooseVariableFE<OutputType>::adNodalValue()
{
  return _element_data->adNodalValue();
}
