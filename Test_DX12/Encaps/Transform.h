#ifndef TRANSFORM_H_DEFINED
#define TRANSFORM_H_DEFINED

#include <DirectXMath.h>
#include <type_traits>
using namespace DirectX;

enum DIRTY_FLAG : uint8_t
{
    WORLD =     0b01,
    INVERSE =   0b10,

    ALL =       0b11
};

class Transform
{
private:
    XMFLOAT3 pos;
    XMFLOAT3 scale;
    
    XMFLOAT3 forward;
    XMFLOAT3 up;
    XMFLOAT3 right;
    XMFLOAT4 quat;
    XMFLOAT4X4 rot;

    XMFLOAT4X4 worldMatrix;
    XMFLOAT4X4 invMatrix;

    int dirty = 0;

public:
    Transform();
    ~Transform() = default;

    Transform(const Transform& other);
    Transform(Transform&& other) noexcept;
    Transform& operator=(const Transform& other);
    Transform& operator=(Transform&& other) noexcept;
    
    XMFLOAT4X4& GetWorldMatrix();
    XMFLOAT4X4& GetInvMatrix();

    void SetIdentity();
    void UpdateWorldMatrix();
    void UpdateInvMatrix();
    
    // Pos
    
    const XMFLOAT3& GetPosition() { return pos; };
    
    void SetPosition(XMFLOAT3 const& position);
    void Move(XMFLOAT3 const& delta);
    void Move(XMFLOAT3 const& dir, float distance);

    // Scale
    
    const XMFLOAT3& GetScale() { return scale; };
    
    void SetScale(XMFLOAT3 const& scale);
    void SetScale(float scale);
    void Scale(XMFLOAT3 const& scale);
    void Scale(float scale);

    // Rotate

    const XMFLOAT3& GetForward()    { return forward; }
    const XMFLOAT3& GetRight()      { return right; }
    const XMFLOAT3& GetUp()         { return up; }
    const XMFLOAT4& GetRotation()   { return quat; }

    void LookAt(XMFLOAT3 const& target);
    void LookTo(XMFLOAT3 const& dir);
    void SetRotationMatrix(XMFLOAT4X4 const& rotation);
    void SetRotationQuaternion(XMFLOAT4 const& quat);
    
    void ResetRotation();
    
    void SetYPR(XMFLOAT3 const& ypr);
    void AddYPR(XMFLOAT3 const& ypr);

    void UpdateRotationFromAxes();
    void UpdateRotationFromQuaternion();
    void UpdateRotationFromMatrix();
};

#endif
