//
//  BendingOperator_Parametric.hpp
//  Elasticity
//
//  Created by Wim van Rees on 12/23/17.
//  Copyright © 2017 Wim van Rees. All rights reserved.
//

#ifndef BendingOperator_Parametric_hpp
#define BendingOperator_Parametric_hpp

#include "common.hpp"
#include "EnergyOperator.hpp"
#include "MaterialProperties.hpp"

/**
 * @brief Computes the Bending energy and its gradient for a monolayer or bilayer
 * @tparam tMesh Type of the mesh
 * @tparam MeshLayer The layer of the mesh we are computing for (single, bottom or top)
 *
 * @details
 * This class is responsible for computing the isotropic hyperelastic Bending energy of the current configuration
 * with respect to the reference first fundamental form of the rest configuration. It is also responsible
 * for computing the algebraic gradient of this energy with respect to the vertex positions and the edge director values.
 * It can handle a monolayer mesh, as well as each layer of a bilayer mesh with different reference first fundamental forms
 * It is based on the full elastic energy (stretching + bending) method, and so it is inefficient - it just computes all components of the energy but discards the bending part by not using it. Should only be used for post-processing / analysis, not for serious computations.
 */

template<typename tMesh, typename tMaterialType = Material_Isotropic, MeshLayer layer = single>
class BendingOperator_Parametric : public EnergyOperator_DCS<tMesh>
{
public:
    using EnergyOperator_DCS<tMesh>::compute;
    typedef typename tMesh::tCurrentConfigData tCurrentConfigData;
    
protected:
    /**
     * A reference to the material properties of the faces
     */
    const MaterialProperties<tMaterialType> & material_properties;
    
    /**
     * The last computed bending energy will be stored here
     */
    mutable Real lastEnergy;
    
    /**
     * Computes the bending energy and, if desired, the gradient
     *
     * Assumes that the reference first form have been set
     * Also assumes that the current configuration is up-to-date, in other words that update() has been called
     * on the current configuration before calling this method
     *
     * @param [in] mesh the mesh for which we compute the energy
     * @param [out] gradient_vertices the gradient of the energy with respect each vertex coordinate (unused if computeGradient is false)
     * @param [out]  gradient_edges the gradient of the energy with respect each edge director value (unused if computeGradient is false)
     * @param [in] computeGradient whether or not we should compute the gradient
     * @return the total computed energy (lastEnergy)
     */    
    Real computeAll(const tMesh & mesh, Eigen::Ref<Eigen::MatrixXd> gradient_vertices, Eigen::Ref<Eigen::VectorXd> gradient_edges, const bool computeGradient) const override;
    
public:
    
    BendingOperator_Parametric(const MaterialProperties<tMaterialType> & material_properties):
    EnergyOperator_DCS<tMesh>(),
    material_properties(material_properties),
    lastEnergy(0)
    {}
    
    virtual void addEnergy(std::vector<std::pair<std::string, Real>> & out) const override
    {
        std::string prefix;
        switch(layer)
        {
            case single   : prefix=""; break;
            case bottom   : prefix="bottom"; break;
            case top   : prefix="top"; break;
            default    : std::cout << "no valid layer given\n";
        }
        
        out.push_back(std::make_pair(prefix+"bending_bb", lastEnergy));
    }
    
    /**
     * Computes the energy for only a subset of all the triangle faces, and integrates them
     *
     * Computes only the energy (no gradient) for the face indices inside the face_indices vector
     * and integrates those
     *
     * @param [in] mesh the mesh for which we compute the energy
     * @param [in] face_indices the list of face indices for which we should compute the energy
     * @return the integrated energy over the subset of faces
     */
    Real computeSubsetEnergies(const tMesh & mesh, const std::vector<int> & face_indices) const override;
    bool isSubsetImplemented() const override {return true;}
    
    void printProfilerSummary() const override
    {
        if(layer == bottom) std::cout << "=== BOTTOM LAYER === \n";
        if(layer == top)  std::cout << "=== TOP LAYER === \n";
        this->profiler.printSummary();
    }
    
    Real getLastEnergy() const
    {
        return lastEnergy;
    }
};

#endif /* BendingOperator_Parametric_hpp */
