#ifndef TRANSFORM_CPP_DEFINED
#define TRANSFORM_CPP_DEFINED

#include "Transform.h"

Transform::Transform()
{
    dirty = 0;
    SetIdentity();
    ResetRotation();
}

Transform::Transform(const Transform& other)
        : pos(other.pos),
          scale(other.scale),
          forward(other.forward),
          up(other.up),
          right(other.right),
          quat(other.quat),
          rot(other.rot),
          worldMatrix(other.worldMatrix),
          invMatrix(other.invMatrix),
          dirty(other.dirty)
{
}

Transform::Transform(Transform&& other) noexcept
    : pos(std::move(other.pos)),
      scale(std::move(other.scale)),
      forward(std::move(other.forward)),
      up(std::move(other.up)),
      right(std::move(other.right)),
      quat(std::move(other.quat)),
      rot(std::move(other.rot)),
      worldMatrix(std::move(other.worldMatrix)),
      invMatrix(std::move(other.invMatrix)),
      dirty(other.dirty)
{
}

Transform& Transform::operator=(const Transform& other)
{
    if (this == &other)
        return *this;
    pos = other.pos;
    scale = other.scale;
    forward = other.forward;
    up = other.up;
    right = other.right;
    quat = other.quat;
    rot = other.rot;
    worldMatrix = other.worldMatrix;
    invMatrix = other.invMatrix;
    dirty = other.dirty;
    return *this;
}

Transform& Transform::operator=(Transform&& other) noexcept
{
    if (this == &other)
        return *this;
    pos = std::move(other.pos);
    scale = std::move(other.scale);
    forward = std::move(other.forward);
    up = std::move(other.up);
    right = std::move(other.right);
    quat = std::move(other.quat);
    rot = std::move(other.rot);
    worldMatrix = std::move(other.worldMatrix);
    invMatrix = std::move(other.invMatrix);
    dirty = other.dirty;
    return *this;
}

XMFLOAT4X4& Transform::GetWorldMatrix()
{
    if (dirty &= WORLD)
        UpdateWorldMatrix();
    
    return worldMatrix;
}

XMFLOAT4X4& Transform::GetInvMatrix()
{
    if (dirty &= INVERSE)
        UpdateInvMatrix();
    
    return invMatrix;
}

void Transform::SetIdentity()
{
    pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
    scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    ResetRotation();
}

void Transform::UpdateWorldMatrix()
{
    XMVECTOR p = XMLoadFloat3(&pos);
    XMVECTOR s = XMLoadFloat3(&scale);

    XMVECTOR sx = XMVectorSplatX(s);
    XMVECTOR sy = XMVectorSplatY(s);
    XMVECTOR sz = XMVectorSplatZ(s);

    XMMATRIX m = XMLoadFloat4x4(&worldMatrix);
    m.r[0] = XMVectorMultiply(m.r[0], sx);
    m.r[1] = XMVectorMultiply(m.r[1], sy);
    m.r[2] = XMVectorMultiply(m.r[2], sz);
    m.r[3] = p;

    XMStoreFloat4x4(&worldMatrix, m);

    dirty |= ~WORLD;
}

void Transform::UpdateInvMatrix()
{
    if (dirty &= WORLD)
        UpdateWorldMatrix();
    
    XMMATRIX m = XMLoadFloat4x4(&worldMatrix);
    XMStoreFloat4x4(&invMatrix, XMMatrixInverse(nullptr, m));

    dirty |= ~INVERSE;
}


///////////////////////////////////////////////////////////////////////////////////////
/// POSITION ///

void Transform::SetPosition(XMFLOAT3 const& position)
{
    pos = position;

    dirty |= ALL;
}

void Transform::Move(XMFLOAT3 const& delta)
{
    XMVECTOR p = XMLoadFloat3(&pos);
    XMVECTOR s = XMLoadFloat3(&delta);

    p = XMVectorAdd(p, s);

    XMStoreFloat3(&pos, p);

    dirty |= ALL;
}

void Transform::Move(XMFLOAT3 const& dir, float distance)
{
    XMVECTOR p = XMLoadFloat3(&pos);
    XMVECTOR s = XMLoadFloat3(&dir);
    XMVECTOR scalar = XMVectorSet(distance, distance, distance, 0.0f);

    s = XMVectorMultiply(s, scalar);
    p = XMVectorAdd(p, s);

    XMStoreFloat3(&pos, p);

    dirty |= ALL;
}


///////////////////////////////////////////////////////////////////////////////////////
/// SCALE ///

void Transform::SetScale(XMFLOAT3 const& _scale)
{
    scale = _scale;

    dirty |= ALL;
}

void Transform::SetScale(float _scale)
{
    XMStoreFloat3(&scale, XMVectorSet(_scale, _scale, _scale, 0.0f));

    dirty |= ALL;
}

void Transform::Scale(XMFLOAT3 const& _scale)
{
    XMVECTOR s      = XMLoadFloat3(&scale);
    XMVECTOR mult   = XMLoadFloat3(&_scale);

    s = XMVectorMultiply(s, mult);

    XMStoreFloat3(&scale, s);

    dirty |= ALL;
}

void Transform::Scale(float _scale)
{
    XMVECTOR s      = XMLoadFloat3(&scale);
    XMVECTOR scalar = XMVectorSet(_scale, _scale, _scale, 0.0f);

    s = XMVectorMultiply(s, scalar);

    XMStoreFloat3(&scale, s);

    dirty |= ALL;
}


///////////////////////////////////////////////////////////////////////////////////////
/// ROTATION ///

void Transform::LookAt(XMFLOAT3 const& target)
{
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX m = XMMatrixLookAtLH(
        XMLoadFloat3(&pos), XMLoadFloat3(&target), up);
    XMStoreFloat4x4(&worldMatrix, m);
    UpdateRotationFromMatrix();
}

void Transform::LookTo(XMFLOAT3 const& dir)
{
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMMATRIX m = XMMatrixLookToLH(
        XMLoadFloat3(&pos), XMLoadFloat3(&dir), up);
    XMStoreFloat4x4(&worldMatrix, m);
    UpdateRotationFromMatrix();
}

void Transform::SetRotationMatrix(XMFLOAT4X4 const& rotation)
{
    rot = rotation;
    UpdateRotationFromMatrix();
}

void Transform::SetRotationQuaternion(XMFLOAT4 const& _quat)
{
    quat = _quat;
    UpdateRotationFromQuaternion();
}

void Transform::ResetRotation()
{
    forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
    up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    right = XMFLOAT3(1.0f, 0.0f, 0.0f);

    XMStoreFloat4x4(&rot, XMMatrixIdentity());
    XMStoreFloat4(&quat, XMQuaternionIdentity());

    dirty |= ALL;
}

void Transform::SetYPR(XMFLOAT3 const& ypr)
{
    ResetRotation();
    AddYPR(ypr);
}

void Transform::AddYPR(XMFLOAT3 const& ypr)
{
    XMVECTOR f = XMLoadFloat3(&forward);
    XMVECTOR u = XMLoadFloat3(&up);
    XMVECTOR r = XMLoadFloat3(&right);

    XMVECTOR qRot = XMLoadFloat4(&quat);
    if (ypr.z != 0.0f)
        qRot = XMQuaternionMultiply(qRot, XMQuaternionRotationAxis(f, ypr.z));
    if (ypr.y != 0.0f)
        qRot = XMQuaternionMultiply(qRot, XMQuaternionRotationAxis(r, ypr.y));
    if (ypr.x != 0.0f)
        qRot = XMQuaternionMultiply(qRot, XMQuaternionRotationAxis(u, ypr.x));

    qRot = XMQuaternionNormalize(qRot);
    XMStoreFloat4(&quat, qRot);

    SetRotationQuaternion(quat);
}

void Transform::UpdateRotationFromAxes()
{
    rot._11 = forward.x;
    rot._12 = forward.y;
    rot._13 = forward.z;
    rot._21 = right.x;
    rot._22 = right.y;
    rot._23 = right.z;
    rot._31 = up.x;
    rot._32 = up.y;
    rot._33 = up.z;

    XMStoreFloat4(&quat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&rot)));

    dirty |= ALL;
}

void Transform::UpdateRotationFromQuaternion()
{
    XMStoreFloat4x4(&rot,  XMMatrixRotationQuaternion(XMLoadFloat4(&quat)));

    forward.x = rot._11;
    forward.y = rot._12;
    forward.z = rot._13;
    right.x = rot._21;
    right.y = rot._22;
    right.z = rot._23;
    up.x = rot._31;
    up.y = rot._32;
    up.z = rot._33;

    dirty |= ALL;
}

void Transform::UpdateRotationFromMatrix()
{
    XMStoreFloat4(&quat, XMQuaternionRotationMatrix(XMLoadFloat4x4(&rot)));

    forward.x = rot._11;
    forward.y = rot._12;
    forward.z = rot._13;
    right.x = rot._21;
    right.y = rot._22;
    right.z = rot._23;
    up.x = rot._31;
    up.y = rot._32;
    up.z = rot._33;

    dirty |= ALL;
}


///////////////////////////////////////////////////////////////////////////////////////

#endif