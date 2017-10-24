//
//  SolidEntity.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 12/29/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#include "SolidEntity.h"
#include "MathsUtil.hpp"
#include "SystemUtil.hpp"
#include "Console.h"
#include "Ocean.h"

SolidEntity::SolidEntity(std::string uniqueName, Material m, int _lookId) : Entity(uniqueName)
{
	mat = m;
    mass = btScalar(0);
	Ipri = btVector3(0,0,0);
    volume = btScalar(0);
	thickness = btScalar(0);
	//aMass = btVector3(0,0,0);
    //dragCoeff = btVector3(0,0,0);
    CoB = btVector3(0,0,0);
    localTransform = btTransform::getIdentity(); //CoG = (0,0,0)
    ellipsoidR = btVector3(0,0,0);
    ellipsoidTransform = btTransform::getIdentity();
	computeHydro = true;
    Fb = btVector3(0,0,0);
    Tb = btVector3(0,0,0); 
    Fd = btVector3(0,0,0);
    Td = btVector3(0,0,0); 
    Fa = btVector3(0,0,0); 
    Ta = btVector3(0,0,0);
    	
    linearAcc = btVector3(0,0,0);
    angularAcc = btVector3(0,0,0);
    
    rigidBody = NULL;
    multibodyCollider = NULL;
    
	mesh = NULL;
	lookId = _lookId;
	objectId = -1;
	
    dispCoordSys = false;
}

SolidEntity::~SolidEntity()
{
    if(mesh != NULL)
	{
		delete mesh;
		mesh = NULL;
	}
	
    rigidBody = NULL;
    multibodyCollider = NULL;
}

EntityType SolidEntity::getType()
{
    return ENTITY_SOLID;
}

void SolidEntity::ScalePhysicalPropertiesToArbitraryMass(btScalar mass)
{
    if(rigidBody != NULL)
    {
        btScalar oldMass = this->mass;
        this->mass = UnitSystem::SetMass(mass);
        Ipri *= this->mass/oldMass;
        rigidBody->setMassProps(this->mass, Ipri);
    }
    else if(multibodyCollider == NULL) 
    {
        btScalar oldMass = this->mass;
        this->mass = UnitSystem::SetMass(mass);
        Ipri *= this->mass/oldMass;        
    }
}

void SolidEntity::SetArbitraryPhysicalProperties(btScalar mass, const btVector3& inertia, const btTransform& cogTransform)
{
    if(rigidBody != NULL)
    {
        this->mass = UnitSystem::SetMass(mass);
        Ipri = UnitSystem::SetInertia(inertia);
        rigidBody->setMassProps(this->mass, Ipri);
        
        btTransform oldLocalTransform = localTransform;
        localTransform = UnitSystem::SetTransform(cogTransform);
        btCompoundShape* colShape = (btCompoundShape*)rigidBody->getCollisionShape();
        rigidBody->setCenterOfMassTransform(oldLocalTransform.inverse() * localTransform * rigidBody->getCenterOfMassTransform());
        colShape->updateChildTransform(0, localTransform.inverse());
    }
    else if(multibodyCollider == NULL) // && rigidBody == NULL
    {
        this->mass = UnitSystem::SetMass(mass);
        Ipri = UnitSystem::SetInertia(inertia);
        localTransform = UnitSystem::SetTransform(cogTransform);
    }
}

void SolidEntity::SetHydrodynamicProperties(const eigMatrix6x6& addedMass, const eigMatrix6x6& damping, const btTransform& cobTransform)
{
}

void SolidEntity::setComputeHydrodynamics(bool flag)
{
	computeHydro = flag;
}

void SolidEntity::SetLook(int newLookId)
{
    lookId = newLookId;
}

void SolidEntity::setDisplayCoordSys(bool enabled)
{
    dispCoordSys = enabled;
}

bool SolidEntity::isCoordSysVisible()
{
    return dispCoordSys;
}

int SolidEntity::getLook()
{
    return lookId;
}

int SolidEntity::getObject()
{
	return objectId;
}

btRigidBody* SolidEntity::getRigidBody()
{
    return rigidBody;
}

btMultiBodyLinkCollider* SolidEntity::getMultibodyLinkCollider()
{
    return multibodyCollider;
}

void SolidEntity::GetAABB(btVector3& min, btVector3& max)
{
    rigidBody->getAabb(min, max);
}

std::vector<Renderable> SolidEntity::Render()
{
	std::vector<Renderable> items(0);
	
	if((rigidBody != NULL || multibodyCollider != NULL) && objectId >= 0 && isRenderable())
	{
		btTransform oTrans =  getTransform() * localTransform.inverse();
		
		Renderable item;
		item.objectId = objectId;
		item.lookId = lookId;
		item.dispCoordSys = dispCoordSys;
		item.model = glMatrixFromBtTransform(oTrans);
		item.csModel = glMatrixFromBtTransform(getTransform());
        item.eModel = glMatrixFromBtTransform(oTrans * ellipsoidTransform);
        item.eRadii = glm::vec3((GLfloat)ellipsoidR[0], (GLfloat)ellipsoidR[1], (GLfloat)ellipsoidR[2]);
        items.push_back(item);
	}
	
	return items;
}

btTransform SolidEntity::getTransform() const
{
    if(rigidBody != NULL)
    {
        btTransform trans;
        rigidBody->getMotionState()->getWorldTransform(trans);
        return trans;
    }
    else if(multibodyCollider != NULL)
    {
        return multibodyCollider->getWorldTransform();
    }
    else
        return btTransform::getIdentity();
}

void SolidEntity::setTransform(const btTransform& trans)
{
    if(rigidBody != NULL)
    {
        rigidBody->getMotionState()->setWorldTransform(trans);
    }
    else if(multibodyCollider != NULL)
    {
        multibodyCollider->setWorldTransform(trans);
    }
}

btVector3 SolidEntity::getLinearVelocity()
{
    if(rigidBody != NULL)
    {
        return rigidBody->getLinearVelocity();
    }
    else if(multibodyCollider != NULL)
    {
        //Get multibody and link id
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        //Start with base velocity
        btVector3 linVelocity = multiBody->getBaseVel(); //Global
        
        if(index >= 0) //If collider is not base
        {
            for(unsigned int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
            {
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::ePrismatic) //Just add linear velocity
                {
                    btVector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local axis
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
                else if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Add linear velocity due to rotation
                {
                    btVector3 axis = multiBody->getLink(i).getAxisBottom(0); //Local linear motion
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    linVelocity += gvel;
                }
            }
        }
        
        return linVelocity;
    }
    else
        return btVector3(0.,0.,0.);
}

btVector3 SolidEntity::getAngularVelocity()
{
    if(rigidBody != NULL)
    {
        return rigidBody->getAngularVelocity();
    }
    else if(multibodyCollider != NULL)
    {
        //Get multibody and link id
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        //Start with base velocity
        btVector3 angVelocity = multiBody->getBaseOmega(); //Global
        
        if(index >= 0)
        {
            for(unsigned int i = 0; i <= index; i++) //Accumulate velocity resulting from joints
                if(multiBody->getLink(i).m_jointType == btMultibodyLink::eRevolute) //Only revolute joints can change angular velocity
                {
                    btVector3 axis = multiBody->getLink(i).getAxisTop(0); //Local axis
                    btVector3 vel = multiBody->getJointVel(i) * axis; //Local velocity
                    btVector3 gvel = multiBody->localDirToWorld(i, vel); //Global velocity
                    angVelocity += gvel;
                }
        }
        
        return angVelocity;
    }
    else
        return btVector3(0.,0.,0.);
}


btVector3 SolidEntity::getLinearVelocityInLocalPoint(const btVector3& relPos)
{
    if(rigidBody != NULL)
    {
        return rigidBody->getVelocityInLocalPoint(relPos);
    }
    else if(multibodyCollider != NULL)
    {
        return getLinearVelocity() + getAngularVelocity().cross(relPos);
    }
    else
        return btVector3(0.,0.,0.);
}

btVector3 SolidEntity::getLinearAcceleration()
{
    return linearAcc;
}

btVector3 SolidEntity::getAngularAcceleration()
{
    return angularAcc;
}

btScalar SolidEntity::getVolume()
{
    return volume;
}

btTransform SolidEntity::getGeomToCOGTransform()
{
    return localTransform;
}

btVector3 SolidEntity::getCOBInGeomFrame()
{
    return CoB;
}

btVector3 SolidEntity::getMomentsOfInertia()
{
    return Ipri;
}

btScalar SolidEntity::getMass()
{
    return mass;
}

Material SolidEntity::getMaterial()
{
    return mat;
}

std::vector<Vertex>* SolidEntity::getMeshVertices()
{
    std::vector<Vertex>* pVert = new std::vector<Vertex>(0);
    
    if(mesh != NULL)
        pVert->insert(pVert->end(), mesh->vertices.begin(), mesh->vertices.end());
        
    return pVert;
}

void SolidEntity::ComputeEquivEllipsoid()
{
    //Get vertices of solid
	std::vector<Vertex>* vertices = getMeshVertices();
    if(vertices->size() < 9)
    {
        delete vertices;
        return;
    }
    
	//Fill points matrix
	eigMatrix P(vertices->size(), 3);
	for(unsigned int i=0; i<vertices->size(); ++i)
		P.row(i) << (*vertices)[i].pos.x, (*vertices)[i].pos.y, (*vertices)[i].pos.z;
    delete vertices;
    
	//Compute contraints
	eigMatrix A(P.rows(), 9);
	A.col(0) = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() - 2 * P.col(2).array() * P.col(2).array();
	A.col(1) = P.col(0).array() * P.col(0).array() + P.col(2).array() * P.col(2).array() - 2 * P.col(1).array() * P.col(1).array();
	A.col(2) = 2 * P.col(0).array() * P.col(1).array();
	A.col(3) = 2 * P.col(0).array() * P.col(2).array();
	A.col(4) = 2 * P.col(1).array() * P.col(2).array();
	A.col(5) = 2 * P.col(0);
	A.col(6) = 2 * P.col(1);
	A.col(7) = 2 * P.col(2);
	A.col(8) = eigMatrix::Ones(P.rows(), 1);
	
	//Solve Least-Squares problem Ax=b
	eigMatrix b(P.rows(), 1);
	eigMatrix x(9, 1);	
	//squared norm
	b = P.col(0).array() * P.col(0).array() + P.col(1).array() * P.col(1).array() + P.col(2).array() * P.col(2).array();
	//solution
	//x = (A.transpose() * A).ldlt().solve(A.transpose() * b); //normal equations
	x = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b); 
    
	//Find ellipsoid parameters
	eigMatrix p(10, 1);
	p(0) = x(0) + x(1) - 1;
	p(1) = x(0) - 2 * x(1) - 1;
	p(2) = x(1) - 2 * x(0) - 1;
	p(3) = x(2);
	p(4) = x(3);
	p(5) = x(4);
	p(6) = x(5);
	p(7) = x(6);
	p(8) = x(7);
	p(9) = x(8);
	
	eigMatrix E(4, 4);
	E << p(0), p(3), p(4), p(6),
		 p(3), p(1), p(5), p(7),
		 p(4), p(5), p(2), p(8),
		 p(6), p(7), p(8), p(9);
		 
	//Compute center
	eigMatrix c(3, 1);
	c = -E.block(0, 0, 3, 3).jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(p.block(6, 0, 3, 1));
	
	//Compute transform matrix
	eigMatrix4x4 T;
	T.setIdentity();
	T.block(3, 0, 1, 3) = c.transpose();
	T = T * E * T.transpose();
	
	//Compute axes
	Eigen::SelfAdjointEigenSolver<eigMatrix> eigenSolver(T.block(0, 0, 3, 3)/(-T(3,3)));
	if(eigenSolver.info() != Eigen::Success) 
	{
		cError("Error computing ellipsoid for %s!", getName().c_str());
		return;
	}
	
    //Ellipsoid radii
	eigMatrix r(3, 1);
	r = Eigen::sqrt(1/Eigen::abs(eigenSolver.eigenvalues().array()));
    ellipsoidR = btVector3(r(0), r(1), r(2));
    
    //Ellipsoid axes
	eigMatrix axes(3, 3);
	axes = eigenSolver.eigenvectors().array();
    ellipsoidTransform.setIdentity();
    ellipsoidTransform.setOrigin(btVector3(c(0), c(1), c(2)));
    ellipsoidTransform.setBasis(btMatrix3x3(axes(0,0), axes(0,1), axes(0,2), axes(1,0), axes(1,1), axes(1,2), axes(2,0), axes(2,1), axes(2,2)));
}

void SolidEntity::BuildGraphicalObject()
{
	if(mesh == NULL)
		return;
		
	objectId = OpenGLContent::getInstance()->BuildObject(mesh);	
}

void SolidEntity::BuildRigidBody()
{
    if(rigidBody == NULL)
    {
        btDefaultMotionState* motionState = new btDefaultMotionState();
        
        btCompoundShape* colShape = new btCompoundShape();
        colShape->addChildShape(localTransform.inverse(), BuildCollisionShape());
        colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.001));
        
        btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, colShape, Ipri);
        rigidBodyCI.m_friction = rigidBodyCI.m_rollingFriction = rigidBodyCI.m_restitution = btScalar(0.); //not used
        rigidBodyCI.m_linearDamping = rigidBodyCI.m_angularDamping = btScalar(0.); //not used
		rigidBodyCI.m_linearSleepingThreshold = rigidBodyCI.m_angularSleepingThreshold = btScalar(0.); //not used
        rigidBodyCI.m_additionalDamping = false;
        
        rigidBody = new btRigidBody(rigidBodyCI);
        rigidBody->setUserPointer(this);
        rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        rigidBody->setFlags(rigidBody->getFlags() | BT_ENABLE_GYROSCOPIC_FORCE_IMPLICIT_BODY);
        //rigidBody->setActivationState(DISABLE_DEACTIVATION);
        //rigidBody->setCcdMotionThreshold(0.01);
        //rigidBody->setCcdSweptSphereRadius(0.9);
    }
}

void SolidEntity::BuildMultibodyLinkCollider(btMultiBody *mb, unsigned int child, btMultiBodyDynamicsWorld *world)
{
    if(multibodyCollider == NULL)
    {
        //Shape with offset
        btCompoundShape* colShape = new btCompoundShape();
        colShape->addChildShape(localTransform.inverse(), BuildCollisionShape());
        colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), 0.001));
        
        //Link
        multibodyCollider = new btMultiBodyLinkCollider(mb, child - 1);
        multibodyCollider->setUserPointer(this);
        multibodyCollider->setCollisionShape(colShape);
        multibodyCollider->setFriction(btScalar(0.));
        multibodyCollider->setRestitution(btScalar(0.));
        multibodyCollider->setRollingFriction(btScalar(0.));
        multibodyCollider->setCollisionFlags(multibodyCollider->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
		multibodyCollider->setActivationState(DISABLE_DEACTIVATION);
        
        if(child > 0)
            mb->getLink(child - 1).m_collider = multibodyCollider;
        else
            mb->setBaseCollider(multibodyCollider);
        
        world->addCollisionObject(multibodyCollider, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
        
        //Graphics
        BuildGraphicalObject();
    }
}

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    AddToDynamicsWorld(world, btTransform::getIdentity());
}

void SolidEntity::AddToDynamicsWorld(btMultiBodyDynamicsWorld* world, const btTransform& worldTransform)
{
    if(rigidBody == NULL)
    {
        BuildRigidBody();
		BuildGraphicalObject();
        
        //rigidBody->setMotionState(new btDefaultMotionState(UnitSystem::SetTransform(worldTransform)));
        rigidBody->setCenterOfMassTransform(UnitSystem::SetTransform(worldTransform) * localTransform);
        world->addRigidBody(rigidBody, MASK_DEFAULT, MASK_STATIC | MASK_DEFAULT);
    }
}

void SolidEntity::RemoveFromDynamicsWorld(btMultiBodyDynamicsWorld* world)
{
    if(rigidBody != NULL)
    {
        world->removeRigidBody(rigidBody);
        delete rigidBody;
        rigidBody = NULL;
    }
}

void SolidEntity::UpdateAcceleration()
{
    if(rigidBody != NULL)
    {
        linearAcc = rigidBody->getTotalForce()/mass;
        btVector3 torque = rigidBody->getTotalTorque();
        angularAcc = btVector3(torque.x()/Ipri.x(), torque.y()/Ipri.y(), torque.z()/Ipri.z());
    }
}

void SolidEntity::ApplyGravity()
{
    if(rigidBody != NULL)
    {
        rigidBody->applyGravity();
    }
}

void SolidEntity::ApplyCentralForce(const btVector3& force)
{
    if(rigidBody != NULL)
        rigidBody->applyCentralForce(force);
    else if(multibodyCollider != NULL)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        if(index == -1) //base
            multiBody->addBaseForce(force);
        else
            multiBody->addLinkForce(index, force);
    }
}

void SolidEntity::ApplyTorque(const btVector3& torque)
{
    if(rigidBody != NULL)
        rigidBody->applyTorque(torque);
    else if(multibodyCollider != NULL)
    {
        btMultiBody* multiBody = multibodyCollider->m_multiBody;
        int index = multibodyCollider->m_link;
        
        if(index == -1) //base
            multiBody->addBaseTorque(torque);
        else
            multiBody->addLinkTorque(index, torque);
    }
}

void SolidEntity::ComputeFluidForces(const HydrodynamicsSettings& settings, const Liquid* liquid, const btTransform& cogTransform, const btTransform& geometryTransform, const btVector3& v, const btVector3& omega, const btVector3& a, const btVector3& epsilon, btVector3& _Fb, btVector3& _Tb, btVector3& _Fd, btVector3& _Td, btVector3& _Fa, btVector3& _Ta)
{
    //Set zeros
	_Fb.setZero();
	_Tb.setZero();
	_Fd.setZero();
	_Td.setZero();
	_Fa.setZero();
	_Ta.setZero();
	
	if(!computeHydro || mesh == NULL)
		return;
    
    uint64_t start = GetTimeInMicroseconds();
    
	//Calculate fluid dynamics forces and torques
    btVector3 p = cogTransform.getOrigin();
    
	//Loop through all faces...
    for(int i=0; i<mesh->faces.size(); ++i)
    {
        //Global coordinates
		glm::vec3 p1gl = mesh->vertices[mesh->faces[i].vertexID[0]].pos;
		glm::vec3 p2gl = mesh->vertices[mesh->faces[i].vertexID[1]].pos;
		glm::vec3 p3gl = mesh->vertices[mesh->faces[i].vertexID[2]].pos;
        btVector3 p1 = geometryTransform * btVector3(p1gl.x,p1gl.y,p1gl.z);
        btVector3 p2 = geometryTransform * btVector3(p2gl.x,p2gl.y,p2gl.z);
        btVector3 p3 = geometryTransform * btVector3(p3gl.x,p3gl.y,p3gl.z);
        
        //Check if underwater
        btScalar depth[3];
        depth[0] = liquid->GetDepth(p1);
        depth[1] = liquid->GetDepth(p2);
        depth[2] = liquid->GetDepth(p3);
        
        if(depth[0] < btScalar(0.) && depth[1] < btScalar(0.) && depth[2] < btScalar(0.))
            continue;
        
        //Calculate face properties
        btVector3 fc;
        btVector3 fn;
        btVector3 fn1;
        btScalar A;
        
        if(depth[0] < btScalar(0.))
        {
            if(depth[1] < btScalar(0.))
            {
                p1 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p2 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                //p3 without change
                
                //Calculate
                btVector3 fv1 = p2-p1; //One side of the face (triangle)
                btVector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                btScalar len = fn.length();
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/btScalar(2); //Area of the face (triangle)
            }
            else if(depth[2] < btScalar(0.))
            {
                p1 = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 withour change
                p3 = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
                
                //Calculate
                btVector3 fv1 = p2-p1; //One side of the face (triangle)
                btVector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                btScalar len = fn.length();
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/btScalar(2); //Area of the face (triangle)
            }
            else //depth[1] >= 0 && depth[2] >= 0
            {
                //Quad!!!!
                btVector3 p1temp = p2 + (p1-p2) * (depth[1]/(btFabs(depth[0]) + depth[1]));
                //p2 without change
                //p3 without change
                btVector3 p4 = p3 + (p1-p3) * (depth[2]/(btFabs(depth[0]) + depth[2]));
                p1 = p1temp;
                
                //Calculate
                btVector3 fv1 = p2-p1;
                btVector3 fv2 = p3-p1;
                btVector3 fv3 = p4-p1;
                
                fc = (p1 + p2 + p3 + p4)/btScalar(4);
                fn = fv1.cross(fv2);
                btScalar len = fn.length();
                fn1 = fn/len;
                A = len + fv2.cross(fv3).length();
                fn = fn1 * A;
            }
        }
        else if(depth[1] < btScalar(0.))
        {
            if(depth[2] < btScalar(0.))
            {
                //p1 without change
                p2 = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                p3 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
                
                //Calculate
                btVector3 fv1 = p2-p1; //One side of the face (triangle)
                btVector3 fv2 = p3-p1; //Another side of the face (triangle)
                fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
                fn = fv1.cross(fv2); //Normal of the face (length != 1)
                btScalar len = fn.length();
                fn1 = fn/len; //Normalised normal (length = 1)
                A = len/btScalar(2); //Area of the face (triangle)
            }
            else
            {
                //Quad!!!!
                btVector3 p2temp = p1 + (p2-p1) * (depth[0]/(btFabs(depth[1]) + depth[0]));
                //p2 without change
                //p3 without change
                btVector3 p4 = p3 + (p2-p3) * (depth[2]/(btFabs(depth[1]) + depth[2]));
                p2 = p2temp;
                
                //Calculate
                btVector3 fv1 = p2-p1;
                btVector3 fv2 = p3-p1;
                btVector3 fv3 = p4-p1;
                
                fc = (p1 + p2 + p3 + p4)/btScalar(4);
                fn = fv1.cross(fv2);
                btScalar len = fn.length();
                fn1 = fn/len;
                A = len + fv2.cross(fv3).length();
                fn = fn1 * A;
            }
        }
        else if(depth[2] < btScalar(0.))
        {
            
            //Quad!!!!
            btVector3 p3temp = p2 + (p3-p2) * (depth[1]/(btFabs(depth[2]) + depth[1]));
            //p2 without change
            //p3 without change
            btVector3 p4 = p1 + (p3-p1) * (depth[0]/(btFabs(depth[2]) + depth[0]));
            p3 = p3temp;
                
            //Calculate
            btVector3 fv1 = p2-p1;
            btVector3 fv2 = p3-p1;
            btVector3 fv3 = p4-p1;
                
            fc = (p1 + p2 + p3 + p4)/btScalar(4);
            fn = fv1.cross(fv2);
            btScalar len = fn.length();
            fn1 = fn/len;
            A = len + fv2.cross(fv3).length();
            fn = fn1 * A;
        }
        else //All underwater
        {
            btVector3 fv1 = p2-p1; //One side of the face (triangle)
            btVector3 fv2 = p3-p1; //Another side of the face (triangle)
            fc = (p1+p2+p3)/btScalar(3); //Face centroid
        
            fn = fv1.cross(fv2); //Normal of the face (length != 1)
            btScalar len = fn.length();
            fn1 = fn/len; //Normalised normal (length = 1)
            A = len/btScalar(2); //Area of the face (triangle)
        }
        
        btScalar pressure = liquid->GetPressure(fc);   //(liquid->GetPressure(p1) + liquid->GetPressure(p2) + liquid->GetPressure(p3))/btScalar(3);
        
        //Buoyancy force
        if(settings.reallisticBuoyancy)
        {
            btVector3 Fbi = -fn1 * A * pressure;  //-fn/btScalar(2)*pressure; //Buoyancy force per face (based on pressure)
            
            //Accumulate
            _Fb += Fbi;
            _Tb += (fc - p).cross(Fbi);
        }
        
        //Damping force
        if(settings.dampingForces)
		{
            //Skin drag force
            btVector3 vc = liquid->GetFluidVelocity(fc) - (v + omega.cross(fc - p)); //Water velocity at face center
            btVector3 vt = vc - (vc.dot(fn)*fn)/fn.length2(); //Water velocity projected on face (tangent to face)
            btVector3 Fds = liquid->getFluid()->viscosity * vt * A / btScalar(0.0001);
            //btVector3 Fds = vt.safeNormalize()*btScalar(0.5)*fluid->getFluid()->density*btScalar(1.328)/1000.0*vt.length2()*fn.length()/btScalar(2);
        
            //Pressure drag force
            btVector3 vn = vc - vt; //Water velocity normal to face
            btVector3 Fdp(0,0,0);
            if(fn.dot(vn) < btScalar(0))
                Fdp = btScalar(0.5)*liquid->getFluid()->density * vn * vn.length() * A;
            
            //Accumulate
            _Fd += Fds + Fdp;
            _Td += (fc - p).cross(Fds + Fdp);
        }
        
        //Added mass effect
		if(settings.addedMassForces)
        {
            btVector3 ac = -(a + epsilon.cross(fc - p)); //Water acceleration at face center (velocity of fluid is constant)
            btVector3 Fai(0,0,0);
            btScalar an; //acceleration normal to face
            
            if((an = fn1.dot(ac)) < btScalar(0))
            {
                btScalar d = btScalar(1.)/(-an + btScalar(1.)); //Positive thickness of affected layer of fluid
                Fai = liquid->getFluid()->density * A * d * an * fn1; //Fa = rho V a = rho A d a
            }
            
            //Accumulate
            _Fa += Fai;
            _Ta += (fc - p).cross(Fai);
        }
    }
    
    uint64_t elapsed = GetTimeInMicroseconds() - start;
    
    //std::cout << getName() << ": " <<  elapsed << std::endl; 
}

void SolidEntity::ComputeFluidForces(const HydrodynamicsSettings& settings, const Liquid* liquid)
{
    btTransform T = getTransform() * localTransform.inverse();
    btVector3 v = getLinearVelocity();
    btVector3 omega = getAngularVelocity();
	btVector3 a = getLinearAcceleration();
    btVector3 epsilon = getAngularAcceleration();
	ComputeFluidForces(settings, liquid, getTransform(), T, v, omega, a, epsilon, Fb, Tb, Fd, Td, Fa, Ta);
}

void SolidEntity::ApplyFluidForces()
{
    //std::cout << getName() << " " << getMass() << " " << Fb.x() << " " << Fb.y() << " " << Fb.z() << std::endl;
    ApplyCentralForce(Fb + Fd + Fa);
    ApplyTorque(Tb + Td + Ta);
}
