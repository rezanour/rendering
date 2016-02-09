#include "Precomp.h"
#include "MathUtil.h"

// Given a desired direction to point (and an up vector), compute a quaternion representing that orientation.
XMVECTOR __vectorcall QuaternionFromViewDirection(FXMVECTOR forward, FXMVECTOR up)
{
    XMVECTOR right = XMVector3Cross(up, forward);

    XMMATRIX rot = XMMatrixIdentity();
    rot.r[0] = right;
    rot.r[1] = up;
    rot.r[2] = forward;

    return XMQuaternionRotationMatrix(rot);
}
